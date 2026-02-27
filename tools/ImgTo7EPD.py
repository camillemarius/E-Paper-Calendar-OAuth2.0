import sys
from PIL import Image, ImageTk
import tkinter as tk
from tkinter import filedialog
import matplotlib.pyplot as plt

# Hide Tkinter root window
root = tk.Tk()
root.withdraw()

# Ask user to choose an image
input_path = filedialog.askopenfilename(
    title="Select an image to convert to 7-color EPD format",
    filetypes=[("Image files", "*.png;*.jpg;*.jpeg;*.bmp")]
)
if not input_path:
    print("No image selected. Exiting.")
    sys.exit(0)

# Ask user where to save the .h file
output_path = filedialog.asksaveasfilename(
    title="Save as",
    defaultextension=".h",
    filetypes=[("C Header File", "*.h")]
)
if not output_path:
    print("No save location chosen. Exiting.")
    sys.exit(0)

# 7-color palette with Arduino index mapping (0-6)
PALETTE = [
    (0, 0, 0),       # 0 Black
    (255, 255, 255), # 1 White
    (0, 255, 0),     # 2 Green
    (0, 0, 255),     # 3 Blue
    (255, 0, 0),     # 4 Red
    (255, 255, 0),   # 5 Yellow
    (255, 128, 0)    # 6 Orange
]

def nearest_color_index(rgb):
    r, g, b = rgb
    best_idx = 0
    best_dist = float('inf')
    for idx, (pr, pg, pb) in enumerate(PALETTE):
        dist = (r - pr)**2 + (g - pg)**2 + (b - pb)**2
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
    return best_idx

# Load and convert image
im = Image.open(input_path).convert('RGB')

# Keep original orientation, resize to fit EPD while preserving aspect ratio
epd_w, epd_h = 800, 480
im.thumbnail((epd_w, epd_h), Image.Resampling.LANCZOS)

w, h = im.size
pixels = list(im.getdata())
indexed_pixels = [nearest_color_index(p) for p in pixels]

# Pack 3 bits per pixel
data = []
for y in range(h):
    row = indexed_pixels[y*w:(y+1)*w]
    for i in range(0, len(row), 8):
        group = row[i:i+8]
        while len(group) < 8:
            group.append(1)  # pad with white
        byte1 = (group[0] << 5) | (group[1] << 2) | (group[2] >> 1)
        byte2 = ((group[2] & 1) << 7) | (group[3] << 4) | (group[4] << 1) | (group[5] >> 2)
        byte3 = ((group[5] & 3) << 6) | (group[6] << 3) | group[7]
        data.extend([byte1, byte2, byte3])

# Save to .h file
with open(output_path, "w") as f:
    f.write(f"const unsigned char my_7color_image[] PROGMEM = {{\n")
    for i, b in enumerate(data):
        f.write(f"0x{b:02X},")
        if (i + 1) % 16 == 0:
            f.write("\n")
    f.write("};\n")
    f.write(f"// Width: {w}, Height: {h}\n")

# Optional preview: original vs 7-color converted
preview_im = Image.new("RGB", (w*2, h))
preview_im.paste(im, (0,0))
converted = Image.new("RGB", (w,h))
for y in range(h):
    for x in range(w):
        idx = indexed_pixels[y*w + x]
        converted.putpixel((x,y), PALETTE[idx])
preview_im.paste(converted, (w,0))
plt.figure(figsize=(12,6))
plt.imshow(preview_im)
plt.axis('off')
plt.title("Left: Original | Right: 7-color EPD preview")
plt.show()

print(f"Done! Saved to {output_path}")
