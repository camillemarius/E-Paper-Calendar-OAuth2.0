import sys
import os
from PIL import Image, ImageEnhance, ImageFilter
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

# ==== Speicherort-Auswahl (nur Ordner) ====
output_dir = filedialog.askdirectory(title="Select folder to save the .h file")
if not output_dir:
    print("No folder chosen. Exiting.")
    sys.exit(0)

# ==== Dateiname vom Eingabebild übernehmen ====
base_name = os.path.splitext(os.path.basename(input_path))[0]  # z.B. "bild.png" -> "bild"
output_path = os.path.join(output_dir, f"{base_name}.h")

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
if im.height > im.width:
    im = im.rotate(90, expand=True)

# Bild skalieren (Cover: Bild füllt das Display komplett)
ratio_w = EPD_W / im.width
ratio_h = EPD_H / im.height
scale = max(ratio_w, ratio_h)

new_size = (int(im.width * scale), int(im.height * scale))
im = im.resize(new_size, Image.Resampling.LANCZOS)

# Überschüssiges zuschneiden (zentriert)
left = (im.width - EPD_W) // 2
top = (im.height - EPD_H) // 2
right = left + EPD_W
bottom = top + EPD_H
im = im.crop((left, top, right, bottom))

# ==== Bild optional leicht weichzeichnen ====
im = im.filter(ImageFilter.GaussianBlur(0.5))

# ==== Kontrast und Sättigung erhöhen ====
im = ImageEnhance.Contrast(im).enhance(1.2)
im = ImageEnhance.Color(im).enhance(1.2)

w, h = im.size
pixels = np.array(im, dtype=np.float32)

# ==== Floyd-Steinberg Serpentine Dithering ====
dithered_indices = np.zeros((h, w), dtype=int)

for y in range(h):
    if y % 2 == 0:
        rng = range(w)
        direction = 1
    else:
        rng = range(w-1, -1, -1)
        direction = -1

    for x in rng:
        old_pixel = pixels[y, x].copy()
        idx = nearest_color(tuple(old_pixel))
        new_pixel = np.array(PALETTE_RGB[idx], dtype=np.float32)
        dithered_indices[y, x] = idx
        quant_error = old_pixel - new_pixel

        # Fehler verteilen
        if direction == 1:
            if x + 1 < w:
                pixels[y, x+1] += quant_error * 7/16
            if x - 1 >= 0 and y + 1 < h:
                pixels[y+1, x-1] += quant_error * 3/16
            if y + 1 < h:
                pixels[y+1, x] += quant_error * 5/16
            if x + 1 < w and y + 1 < h:
                pixels[y+1, x+1] += quant_error * 1/16
        else:
            if x - 1 >= 0:
                pixels[y, x-1] += quant_error * 7/16
            if x + 1 < w and y + 1 < h:
                pixels[y+1, x+1] += quant_error * 3/16
            if y + 1 < h:
                pixels[y+1, x] += quant_error * 5/16
            if x - 1 >= 0 and y + 1 < h:
                pixels[y+1, x-1] += quant_error * 1/16

        pixels = np.clip(pixels, 0, 255)

# ==== RGB565 konvertieren ====
rgb565_pixels = [PALETTE_RGB565[i] for i in dithered_indices.flatten()]

# ==== Header-Datei speichern ====
with open(output_path, "w") as f:
    f.write(f"const uint16_t {base_name}[] PROGMEM = {{\n")
    for i, val in enumerate(rgb565_pixels):
        f.write(f"0x{val:04X},")
        if (i + 1) % 16 == 0:
            f.write("\n")
    f.write("};\n")
    f.write(f"// Width: {w}, Height: {h}\n")

print(f"Done! Saved to {output_path}")

# ==== Vorschau erzeugen ====
converted_img = Image.new("RGB", (w, h))
converted_img.putdata([PALETTE_RGB[i] for i in dithered_indices.flatten()])

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
