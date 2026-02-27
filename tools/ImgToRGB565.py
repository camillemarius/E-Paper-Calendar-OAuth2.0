import sys
from PIL import Image
import tkinter as tk
from tkinter import filedialog
import matplotlib.pyplot as plt

# Tkinter Root verstecken
root = tk.Tk()
root.withdraw()

# ==== Datei-Auswahl ====
input_path = filedialog.askopenfilename(
    title="Select an image to convert to 7-color EPD format",
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

# ==== 7-Farb-Palette (RGB) und RGB565 ====
PALETTE_RGB = [
    (0, 0, 0),         # 0 Schwarz
    (255, 255, 255),   # 1 Weiß
    (0, 255, 0),       # 2 Grün
    (0, 0, 255),       # 3 Blau
    (255, 0, 0),       # 4 Rot
    (255, 255, 0),     # 5 Gelb
    (255, 165, 0)      # 6 Orange
]

PALETTE_RGB565 = [
    0x0000,  # Schwarz
    0xFFFF,  # Weiß
    0x07E0,  # Grün
    0x001F,  # Blau
    0xF800,  # Rot
    0xFFE0,  # Gelb
    0xFC00   # Orange
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
pixels = list(im.getdata())

# ==== RGB565 konvertieren ====
indexed_pixels = [nearest_color(p) for p in pixels]
rgb565_pixels = [PALETTE_RGB565[i] for i in indexed_pixels]

# ==== Header-Datei speichern ====
with open(output_path, "w") as f:
    f.write(f"const uint16_t my_7color_image[] PROGMEM = {{\n")
    for i, val in enumerate(rgb565_pixels):
        f.write(f"0x{val:04X},")
        if (i + 1) % 16 == 0:
            f.write("\n")
    f.write("};\n")
    f.write(f"// Width: {w}, Height: {h}\n")

print(f"Done! Saved to {output_path}")

# ==== Vorschau erzeugen ====
converted_img = Image.new("RGB", (w, h))
converted_img.putdata([PALETTE_RGB[i] for i in indexed_pixels])

# ==== Original & konvertiert nebeneinander anzeigen ====
fig, axs = plt.subplots(1, 2, figsize=(12, 6))
axs[0].imshow(im)
axs[0].set_title("Original (angepasst)")
axs[0].axis("off")

axs[1].imshow(converted_img)
axs[1].set_title("7-Color EPD Vorschau")
axs[1].axis("off")

plt.tight_layout()
plt.show()
