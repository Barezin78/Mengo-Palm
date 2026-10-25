# Gera os ícones BMP (escudo genérico rubro-negro com CRF) para o pilrc
from PIL import Image, ImageDraw
import os, sys
OUT = sys.argv[1] if len(sys.argv) > 1 else "."
MAG = (255, 0, 255)
def shield(w, h, color=True):
    im = Image.new("RGB", (w, h), MAG if color else (255,255,255)); d = ImageDraw.Draw(im)
    sw = int(w*0.82); x0 = (w-sw)//2; y0 = 0; sh = h-1
    pts = [(x0, y0), (x0+sw-1, y0), (x0+sw-1, y0+int(sh*0.58)), (x0+sw//2, y0+sh), (x0, y0+int(sh*0.58))]
    if color:
        m = Image.new("L", (w, h), 0); ImageDraw.Draw(m).polygon(pts, fill=255)
        st = Image.new("RGB", (w, h), (0,0,0)); sd = ImageDraw.Draw(st); band = sh/6
        for i in range(6):
            if i % 2 == 1: sd.rectangle([0, y0+i*band, w, y0+(i+1)*band], fill=(255,0,0))
        im.paste(st, (0,0), m)
    else:
        d.polygon(pts, fill=(0,0,0))
    return im
def save(im, name):
    im.save(os.path.join(OUT, name))
save(shield(22, 22), "icon-8.bmp"); save(shield(44, 44), "icon-8-hd.bmp")
save(shield(22, 22, False).convert("1"), "icon-1.bmp")
save(shield(15, 9), "sicon-8.bmp"); save(shield(30, 18), "sicon-8-hd.bmp")
save(shield(15, 9, False).convert("1"), "sicon-1.bmp")
