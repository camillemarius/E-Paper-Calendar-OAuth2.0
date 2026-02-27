import sys
from PIL import Image
import tkinter as tk
from tkinter import filedialog
import matplotlib.pyplot as plt
import numpy as np

# Tkinter Root verstecken
root = tk.Tk()
root.withdraw()

# ==== Datei-Auswahl ====
input_path = filedialog.askopenfilename(
    title="Select an image to convert to 6-color EPD format",
    filetypes=[("Image files", "*.png;*.jpg;*.jpeg;*.bmp")]
)
if not input_path:
    print("No image selected. Exiting.")
    sys.exit(0)

# ==== Speicherort-Auswahl ====
output_path = filedialog.asksaveasfilename(
    title="Save as",
    defaultextension=".h",
    filetypes=[("C Header File", "*.h")]
)
if not output_path:
    print("No save location chosen. Exiting.")
    sys.exit(0)

# ==== 6-Farb-Palette (RGB) und RGB565 ====
PALETTE_RGB = [
    (0, 0, 0),         # 0 Black
    (255, 255, 255),   # 1 White
    (255, 255, 0),     # 2 Yellow
    (255, 0, 0),       # 3 Red
    (0, 0, 255),       # 5 Blue
    (0, 255, 0)        # 6 Green
]

PALETTE_RGB565 = [
    0x0000,  # Black
    0xFFFF,  # White
    0xFFE0,  # Yellow
    0xF800,  # Red
    0x001F,  # Blue
    0x07E0   # Green
]

# ==== Funktion: nächstgelegene Palette ====
def nearest_color(rgb):
    r, g, b = rgb
    best_idx = 0
    best_dist = float('inf')
    for idx, (pr, pg, pb) in enumerate(PALETTE_RGB):
        dist = (r - pr)**2 + (g - pg)**2 + (b - pb)**2
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
    return best_idx

# ==== Bild laden und skalieren ====
im = Image.open(input_path).convert('RGB')
EPD_W, EPD_H = 800, 480

# Hoch-/Querformat beibehalten
if (im.width, im.height) == (EPD_H, EPD_W):
    im = im.rotate(90, expand=True)

# Bild skalieren, Rahmen weiß
im.thumbnail((EPD_W, EPD_H), Image.Resampling.LANCZOS)
bg = Image.new("RGB", (EPD_W, EPD_H), (255, 255, 255))
bg.paste(im, ((EPD_W - im.width) // 2, (EPD_H - im.height) // 2))
im = bg

w, h = im.size
pixels = np.array(im, dtype=np.float32)

# ==== Floyd-Steinberg Dithering ====
dithered_indices = np.zeros((h, w), dtype=int)
for y in range(h):
    for x in range(w):
        old_pixel = pixels[y, x].copy()
        idx = nearest_color(tuple(old_pixel))
        new_pixel = np.array(PALETTE_RGB[idx], dtype=np.float32)
        dithered_indices[y, x] = idx
        quant_error = old_pixel - new_pixel
        if x + 1 < w:
            pixels[y, x+1] += quant_error * 7/16
        if x - 1 >= 0 and y + 1 < h:
            pixels[y+1, x-1] += quant_error * 3/16
        if y + 1 < h:
            pixels[y+1, x] += quant_error * 5/16
        if x + 1 < w and y + 1 < h:
            pixels[y+1, x+1] += quant_error * 1/16

# ==== 3-Bit-Pixel Packing für Arduino ====
packed_bytes = bytearray()
flat_indices = dithered_indices.flatten()
for i in range(0, len(flat_indices), 2):
    first = flat_indices[i] & 0x07  # 3 bits
    second = flat_indices[i+1] & 0x07 if i+1 < len(flat_indices) else 0
    packed = (first << 3) | second  # pack two 3-bit pixels in 1 byte, last byte uses 3+0 bits
    packed_bytes.append(packed)

# ==== Header-Datei speichern ====
with open(output_path, "w") as f:
    f.write(f"const uint8_t my_6color_image[] PROGMEM = {{\n")
    for i, val in enumerate(packed_bytes):
        f.write(f"0x{val:02X},")
        if (i + 1) % 16 == 0:
            f.write("\n")
    f.write("};\n")
    f.write(f"// Width: {w}, Height: {h}\n")

print(f"Done! Saved to {output_path}")

# ==== Vorschau erzeugen ====
converted_img = Image.new("RGB", (w, h))
converted_img.putdata([PALETTE_RGB[i] for i in flat_indices])

# ==== Original & konvertiert nebeneinander anzeigen ====
fig, axs = plt.subplots(1, 2, figsize=(12, 6))
axs[0].imshow(im)
axs[0].set_title("Original (angepasst)")
axs[0].axis("off")

axs[1].imshow(converted_img)
axs[1].set_title("6-Color EPD Vorschau (dithered)")
axs[1].axis("off")

plt.tight_layout()
plt.show()
