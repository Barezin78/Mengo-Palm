#!/usr/bin/env python3
"""Renderiza as operações de desenho do app (via ./sim) em PNG — prévia aproximada (fontes Liberation no lugar das fontes do Palm)."""
import subprocess, sys, os
from PIL import Image, ImageDraw, ImageFont
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "../../mockups"))
L = "/usr/share/fonts/truetype/liberation/"
FONTS = {0: ImageFont.truetype(L+"LiberationSans-Regular.ttf", 18), 1: ImageFont.truetype(L+"LiberationSans-Bold.ttf", 18),
         2: ImageFont.truetype(L+"LiberationSans-Regular.ttf", 23), 7: ImageFont.truetype(L+"LiberationSans-Bold.ttf", 24)}
LH = {0: 22, 1: 22, 2: 28, 7: 28}
def col(h): return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

def render(pdb, height, screen, out, now=None, scroll=None):
    args = [os.path.join(HERE, "sim"), pdb, str(height), str(screen)]
    if now or scroll is not None: args.append(str(now or 0) if now else "0")
    if scroll is not None: args.append(str(scroll))
    if now is None and len(args) > 4 and args[4] == "0":
        import time; args[4] = str(int(time.time()) - 3*3600 + 2082844800)
    ops = subprocess.run(args, capture_output=True, text=True, check=True).stdout.splitlines()
    im = Image.new("RGB", (320, height), (255, 255, 255)); d = ImageDraw.Draw(im)
    for op in ops:
        p = op.split()
        if p[0] == "R":
            x, y, w, h, r = map(int, p[1:6])
            if w <= 0 or h <= 0: continue
            box = [x, y, x+w-1, y+h-1]
            if r: d.rounded_rectangle(box, min(r, w//2, h//2), fill=col(p[6]))
            else: d.rectangle(box, fill=col(p[6]))
        elif p[0] == "L":
            d.line(list(map(int, p[1:5])), fill=col(p[5]))
        elif p[0] == "T":
            x, y, f = map(int, p[1:4]); fg, bg, w = col(p[4]), col(p[5]), int(p[6])
            s = bytes.fromhex(p[7]).decode("cp1252") if len(p) > 7 else ""
            d.rectangle([x, y, x+w-1, y+LH[f]-1], fill=bg)
            m = Image.new("L", im.size, 0); ImageDraw.Draw(m).text((x, y+1), s, font=FONTS[f], fill=255)
            m = m.point(lambda v: 255 if v > 100 else 0)
            im.paste(Image.new("RGB", im.size, fg), (0, 0), m)
    # moldura: barra de status do T3 (e área de Graffiti no modo 320x320)
    full = Image.new("RGB", (320, 480), (40, 40, 48)); full.paste(im, (0, 0))
    fd = ImageDraw.Draw(full)
    if height < 448:
        fd.rectangle([0, height, 320, 447], fill=(70, 70, 80))
        fd.text((160, (height+448)//2), "área de Graffiti (aberta)", font=FONTS[0], fill=(200, 200, 200), anchor="mm")
    import make_mockups as mm
    s = mm.S(); s.im = full; s.d = ImageDraw.Draw(full); mm.statusbar(s, "")
    full.save(out); full.resize((640, 960), Image.NEAREST).save(out.replace(".png", "@2x.png"))

if __name__ == "__main__":
    pdb = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "../../dist/MengaoDados.pdb")
    outd = os.path.join(HERE, "../../preview"); os.makedirs(outd, exist_ok=True)
    names = ["inicio", "jogos", "tabela", "elenco", "titulos"]
    for i, n in enumerate(names):
        render(pdb, 448, i, f"{outd}/{i+1}-{n}-320x480.png")
    render(pdb, 320, 0, f"{outd}/6-inicio-320x320.png")
    render(pdb, 320, 1, f"{outd}/7-jogos-320x320.png")
    render("-", 448, 0, f"{outd}/8-sem-dados.png")
    print("ok")
