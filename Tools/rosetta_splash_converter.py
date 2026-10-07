from PIL import Image, ImageOps
from pathlib import Path
import sys

if len(sys.argv) != 3:
    print("Uso: python rosetta_splash_converter.py imagen.png rosetta_logo.h")
    raise SystemExit(1)

src = Path(sys.argv[1])
dst = Path(sys.argv[2])

img = Image.open(src).convert("L")
img = ImageOps.contain(img, (128,64))
canvas = Image.new("L", (128,64), 0)
canvas.paste(img, ((128-img.width)//2, (64-img.height)//2))
canvas = canvas.point(lambda p: 255 if p >= 128 else 0, mode="1")

bitmap=[]
for page in range(8):
    for x in range(128):
        b=0
        for bit in range(8):
            y=page*8+bit
            if canvas.getpixel((x,y)):
                b |= 1<<bit
        bitmap.append(b)

out = """#ifndef ROSETTA_LOGO_H
#define ROSETTA_LOGO_H
#include <stdint.h>
#define ROSETTA_LOGO_WIDTH 128
#define ROSETTA_LOGO_HEIGHT 64
static const uint8_t rosetta_logo_128x64[1024] = {
"""
for i in range(0, len(bitmap), 16):
    out += "    " + ", ".join(f"0x{x:02X}" for x in bitmap[i:i+16]) + ",\n"
out += """};
#endif
"""
dst.write_text(out, encoding="utf-8")
print(f"Creado: {dst}")
