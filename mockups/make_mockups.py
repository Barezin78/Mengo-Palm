# Mockups "Mengão" para Palm Tungsten T3 (320x480, 16-bit, Palm OS 5.2) — só visual, dados EXEMPLO
from PIL import Image, ImageDraw, ImageFont
import os
OUT = os.path.dirname(os.path.abspath(__file__))
W, H = 320, 480
SB = 32                      # T3 status bar height (bottom)
APP_H = H - SB               # 448 usable with slider open + DIA collapsed
G = "/usr/share/fonts/truetype/sand-box/google/"
L = "/usr/share/fonts/truetype/liberation/"
def F(p, s): return ImageFont.truetype(p, s)
UI   = lambda s: F(L+"LiberationSans-Regular.ttf", s)   # Palm system font feel
UIB  = lambda s: F(L+"LiberationSans-Bold.ttf", s)      # Palm bold font feel
ANTON= lambda s: F(G+"Anton/Anton-Regular.ttf", s)
BEB  = lambda s: F(G+"Bebas Neue/BebasNeue-Regular.ttf", s)
BARK = lambda s: F(G+"Barlow Condensed/BarlowCondensed-Black.ttf", s)
BARB = lambda s: F(G+"Barlow Condensed/BarlowCondensed-Bold.ttf", s)

RED, DRED, BLK, WHT = (232,0,16), (150,0,8), (0,0,0), (255,255,255)
GRY, LGRY, MGRY, DGRY = (200,200,200), (232,232,232), (150,150,150), (64,64,64)
GRN, YEL, PALE = (0,150,40), (255,200,0), (255,236,236)
SBBG = (40,40,48)

class S:
    def __init__(s, bg=WHT):
        s.im = Image.new("RGB", (W, H), bg); s.d = ImageDraw.Draw(s.im)
    def px(s, on=True): s.d.fontmode = "1" if on else "L"   # "1" = crisp bitmap, like Palm fonts
    def t(s, xy, txt, f, fill=BLK, anchor="la", aa=False):
        if aa:
            s.px(False); s.d.text(xy, txt, font=f, fill=fill, anchor=anchor); s.px(True); return
        # crisp 1-bit text (Palm bitmap-font feel) without mono-hinting spacing glitches
        m = Image.new("L", s.im.size, 0); md = ImageDraw.Draw(m)
        md.text(xy, txt, font=f, fill=255, anchor=anchor)
        m = m.point(lambda v: 255 if v > 100 else 0)
        s.im.paste(Image.new("RGB", s.im.size, fill), (0, 0), m)
    def tw(s, txt, f): b = s.d.textbbox((0,0), txt, font=f); return b[2]-b[0]

def title(s, txt, right=None, rfill=BLK, tab=BLK, bar=None):
    """Palm OS 5 form title: filled black tab + rule across the screen."""
    f = UIB(17); w = s.tw(txt, f) + 16
    if bar: s.d.rectangle([0, 0, W, 25], fill=bar)
    s.d.rounded_rectangle([0, 0, w, 25], 5, fill=tab); s.d.rectangle([0, 0, w, 6], fill=tab)
    s.d.rectangle([0, 0, 6, 25], fill=tab)
    s.t((8, 3), txt, f, WHT)
    s.d.line([0, 25, W, 25], fill=tab if bar else BLK, width=2)
    if right: s.t((W-6, 4), right, UIB(15), rfill, "ra")

def button(s, box, txt, sel=False, f=None):
    f = f or UIB(15)
    s.d.rounded_rectangle(box, 7, fill=BLK if sel else WHT, outline=BLK, width=2)
    cx, cy = (box[0]+box[2])//2, (box[1]+box[3])//2
    s.t((cx, cy), txt, f, WHT if sel else BLK, "mm")

def popup_trigger(s, x, y, txt):
    f = UIB(15); s.d.polygon([(x, y+6), (x+10, y+6), (x+5, y+12)], fill=BLK)
    s.t((x+14, y), txt, f)

def exemplo(s, x, y, color=RED):
    f = UIB(11); w = s.tw("EXEMPLO", f) + 8
    s.d.rectangle([x-w, y, x, y+15], fill=WHT, outline=color, width=1)
    s.t((x-w//2-0, y+8), "EXEMPLO", f, color, "mm")

def shield(s, x, y, w, h, outline=WHT, text=True):
    """Escudo genérico estilizado (NÃO é o escudo oficial): listras rubro-negras + CRF."""
    pts = [(x, y), (x+w, y), (x+w, y+h*0.58), (x+w/2, y+h), (x, y+h*0.58)]
    m = Image.new("L", (W, H), 0); ImageDraw.Draw(m).polygon(pts, fill=255)
    st = Image.new("RGB", (W, H), BLK); sd = ImageDraw.Draw(st)
    band = h/7
    for i in range(7):
        if i % 2 == 1: sd.rectangle([x, y+i*band, x+w, y+(i+1)*band], fill=RED)
    s.im.paste(st, (0, 0), m)
    s.d.line(pts+[pts[0]], fill=outline, width=2, joint="curve")
    if text:
        f = BARK(int(h*0.36)); cy = y + h*0.40
        s.d.rectangle([x+3, cy-h*0.17, x+w-3, cy+h*0.17], fill=BLK)
        s.t((x+w/2, cy), "CRF", f, WHT, "mm", aa=True)

def statusbar(s, clock="14:37"):
    """Tungsten T3 status bar (slider open): Home, Menu, Find | clock | alert, BT, battery, input toggle."""
    y = APP_H; d = s.d
    d.rectangle([0, y, W, H], fill=SBBG); d.line([0, y, W, y], fill=MGRY)
    c = (225,225,225); cy = y + SB//2
    # home
    d.polygon([(10, cy-1), (19, cy-9), (28, cy-1)], fill=c); d.rectangle([13, cy-1, 25, cy+8], fill=c)
    d.rectangle([17, cy+2, 21, cy+8], fill=SBBG)
    # menu (drop-down)
    d.rectangle([38, cy-8, 56, cy+8], outline=c, width=2); d.rectangle([38, cy-8, 56, cy-4], fill=c)
    for k in (0, 5): d.line([42, cy+k, 52, cy+k], fill=c, width=2)
    # find
    d.ellipse([66, cy-9, 80, cy+5], outline=c, width=2); d.line([78, cy+3, 84, cy+9], fill=c, width=3)
    s.t((W//2, cy), clock, UIB(16), c, "mm")
    # alert star, bluetooth
    d.polygon([(214, cy+7), (220, cy-9), (226, cy+7)], outline=c, width=2)
    d.line([(236, cy-4), (246, cy+5), (241, cy+9), (241, cy-9), (246, cy-5), (236, cy+4)], fill=c, width=2)
    # battery
    d.rectangle([256, cy-6, 278, cy+6], outline=c, width=2); d.rectangle([279, cy-3, 281, cy+3], fill=c)
    d.rectangle([259, cy-3, 272, cy+3], fill=(90,210,90))
    # input area toggle (show Graffiti)
    d.rectangle([290, cy-9, 312, cy+9], outline=c, width=2)
    d.polygon([(295, cy+4), (301, cy-4), (307, cy+4)], fill=c)

def stripes_bg(s, y0, y1, n=8, c1=RED, c2=BLK):
    h = (y1-y0)/n
    for i in range(n): s.d.rectangle([0, round(y0+i*h), W, round(y0+(i+1)*h)], fill=c1 if i % 2 == 0 else c2)

def to565(im):
    return im.point(lambda v: v & 0xF8 if True else v)  # approx RGB565 (G gets 6 bits; close enough for preview)

def save(s, name):
    im = to565(s.im)
    im.save(f"{OUT}/{name}.png"); im.resize((W*2, H*2), Image.NEAREST).save(f"{OUT}/{name}@2x.png"); return im

# ---------------- 1. Clássico Listrado (home) ----------------
def concept1():
    s = S(BLK)
    stripes_bg(s, 26, APP_H, 10)
    title(s, "Mengão", "Sáb 24/10", WHT, tab=RED, bar=BLK)
    # badge + clock panel
    s.d.rounded_rectangle([10, 38, 310, 196], 10, fill=BLK, outline=WHT, width=2)
    shield(s, 22, 52, 66, 82)
    s.t((50, 150), "DESDE", UIB(11), MGRY, "mm"); s.t((55, 168), "1895", BEB(26), WHT, "mm", aa=True)
    s.t((206, 104), "14:37", ANTON(92), WHT, "mm", aa=True)
    s.t((206, 172), "sábado, 24 de outubro", UIB(15), (255,120,120), "mm")
    # próximo jogo card
    s.d.rounded_rectangle([10, 208, 310, 344], 10, fill=WHT, outline=BLK, width=2)
    s.d.rounded_rectangle([10, 208, 310, 234], 10, fill=RED); s.d.rectangle([10, 222, 310, 234], fill=RED)
    s.t((20, 213), "PRÓXIMO JOGO", UIB(14), WHT); exemplo(s, 302, 213, DRED)
    s.t((160, 266), "FLA  x  CAM", BARK(40), BLK, "mm", aa=True)
    s.t((160, 298), "Seg 26/10 · 20h00 · Maracanã", UIB(14), DGRY, "mm")
    s.t((160, 323), "Brasileirão · faltam 1d 23h", UIB(14), DRED, "mm")
    # bottom panel with Palm buttons
    s.d.rectangle([0, 360, W, APP_H], fill=WHT); s.d.line([0, 360, W, 360], fill=BLK, width=2)
    s.t((10, 368), "Último:", UIB(13)); s.t((66, 368), "FLA 2 x 1 PAL", UI(13)); exemplo(s, 310, 366)
    for i, (lbl, sel) in enumerate([("Jogos", False), ("Tabela", False), ("Placar", False)]):
        x = 10 + i*102; button(s, [x, 396, x+94, 428], lbl, sel)
    statusbar(s); return save(s, "1-classico-listrado")

# ---------------- 2. Placar / Agenda ----------------
def concept2():
    s = S(WHT)
    title(s, "Placar")
    popup_trigger(s, 186, 5, "Brasileirão")
    # scoreboard
    s.d.rounded_rectangle([8, 34, 312, 150], 8, fill=BLK)
    s.t((160, 46), "PRÓXIMO · SEG 26/10 · 20h00", UIB(13), YEL, "mm")
    for cx, ab, col in ((70, "FLA", RED), (250, "CAM", (110,110,110))):
        s.d.rounded_rectangle([cx-46, 60, cx+46, 112], 6, fill=col, outline=WHT, width=2)
        s.t((cx, 86), ab, BARK(36), WHT, "mm", aa=True)
    s.t((160, 84), "x", BARK(34), WHT, "mm", aa=True)
    s.t((160, 140), "Maracanã · 32ª rodada", UI(13), GRY, "mm")
    # last results
    s.t((8, 158), "Últimos resultados", UIB(15)); exemplo(s, 312, 158)
    s.d.line([8, 177, 312, 177], fill=BLK)
    rows = [("V", "FLA 2 x 1 PAL", "19/10", GRN), ("E", "BOT 1 x 1 FLA", "15/10", (150,150,0)),
            ("V", "FLA 3 x 0 SAN", "11/10", GRN), ("D", "COR 1 x 0 FLA", "05/10", RED)]
    for i, (r, txt, dt, c) in enumerate(rows):
        y = 182 + i*22
        s.d.rounded_rectangle([10, y+1, 28, y+18], 4, fill=c); s.t((19, y+10), r, UIB(13), WHT, "mm")
        s.t((38, y+2), txt, UI(15)); s.t((312, y+2), dt, UI(14), DGRY, "ra")
    # standings
    y0 = 274
    s.t((8, y0), "Classificação", UIB(15)); exemplo(s, 312, y0)
    s.d.rectangle([8, y0+20, 312, y0+40], fill=BLK)
    cols = [(14, "#", "la"), (40, "Time", "la"), (200, "P", "ra"), (240, "J", "ra"), (280, "V", "ra"), (308, "SG", "ra")]
    for x, h, a in cols: s.t((x, y0+23), h, UIB(13), WHT, a)
    tab = [("1", "Flamengo", 68, 31, 21, "+38"), ("2", "Palmeiras", 64, 31, 19, "+27"),
           ("3", "Cruzeiro", 58, 31, 17, "+18"), ("4", "Botafogo", 53, 31, 15, "+11"), ("5", "Mirassol", 50, 31, 14, "+9")]
    for i, row in enumerate(tab):
        y = y0 + 42 + i*19
        if i == 0: s.d.rectangle([8, y-1, 312, y+17], fill=PALE)
        f = UIB(14) if i == 0 else UI(14); col = DRED if i == 0 else BLK
        vals = [row[0], row[1], str(row[2]), str(row[3]), str(row[4]), row[5]]
        for (x, _, a), v in zip(cols, vals): s.t((x, y), v, f, col, a)
        s.d.line([8, y+17, 312, y+17], fill=LGRY)
    button(s, [8, 412, 108, 440], "Detalhes"); button(s, [116, 412, 216, 440], "Agenda")
    s.t((312, 412), "Sinc. HotSync", UI(12), DGRY, "ra"); s.t((312, 427), "24/10 18:00", UI(12), DGRY, "ra")
    statusbar(s); return save(s, "2-placar-agenda")

# ---------------- 3. Retrô "Mengão Info" (tabs) ----------------
def tabs(s, items, sel, y=32):
    x = 6; w = (W-12)//len(items)
    for i, it in enumerate(items):
        b = [x+i*w, y, x+(i+1)*w-1, y+24]
        s.d.rectangle(b, fill=BLK if i == sel else WHT, outline=BLK, width=1)
        s.t(((b[0]+b[2])//2, y+12), it, UIB(14), WHT if i == sel else BLK, "mm")

def scrollbar(s, x, y0, y1, pos=0.0, size=0.4):
    d = s.d
    d.polygon([(x, y0+8), (x+10, y0+8), (x+5, y0)], fill=BLK)
    d.polygon([(x, y1-8), (x+10, y1-8), (x+5, y1)], fill=BLK)
    d.line([x+5, y0+12, x+5, y1-12], fill=MGRY)
    ty = y0+12 + (y1-y0-24)*pos; d.rectangle([x+2, ty, x+8, ty+(y1-y0-24)*size], fill=BLK)

def concept3(state="jogos"):
    s = S(WHT)
    title(s, "Mengão Info")
    shield(s, W-30, 2, 20, 22, outline=BLK, text=False)
    items = ["Jogos", "Tabela", "Elenco", "Títulos"]
    if state == "jogos":
        tabs(s, items, 0)
        s.t((8, 64), "Outubro 2026", UIB(15)); exemplo(s, 312, 64)
        games = [("Seg 26", "20:00", "FLA x CAM", "Bras", None), ("Qua 21", "21:30", "FLA x RAC", "Liberta", "2x0"),
                 ("Dom 19", "16:00", "FLA x PAL", "Bras", "2x1"), ("Qua 15", "19:00", "BOT x FLA", "Bras", "1x1"),
                 ("Sáb 11", "18:30", "FLA x SAN", "Bras", "3x0"), ("Dom 05", "16:00", "COR x FLA", "Bras", "1x0"),
                 ("Qua 01", "21:30", "RAC x FLA", "Liberta", "1x1"), ("Dom 28/9", "18:30", "FLA x VAS", "Bras", "2x0")]
        for i, (dt, hr, m, comp, res) in enumerate(games):
            y = 86 + i*36
            if i == 0: s.d.rectangle([4, y, 298, y+34], fill=BLK)
            fg = WHT if i == 0 else BLK; sub = (255,140,140) if i == 0 else DGRY
            s.t((10, y+2), dt, UIB(14), fg); s.t((10, y+18), hr, UI(13), sub)
            s.t((92, y+2), m, UIB(16), fg); s.t((92, y+19), comp, UI(12), sub)
            s.t((292, y+9), res or "PRÓX", UIB(16) if res else UIB(13), (255,200,0) if res is None else fg, "ra")
            if i: s.d.line([4, y, 298, y], fill=LGRY)
        scrollbar(s, 304, 86, 372, 0.0, 0.55)
        button(s, [6, 412, 96, 440], "Detalhes"); button(s, [104, 412, 194, 440], "Alarme")
        popup_trigger(s, 210, 418, "Todos")
        statusbar(s); return save(s, "3-mengao-info-jogos")
    # títulos + menu aberto
    tabs(s, items, 3)
    s.t((8, 64), "Galeria de títulos", UIB(15)); exemplo(s, 312, 64)
    rows = [("Mundial de Clubes", "1981"), ("Libertadores", "1981 · 2019 · 2022 · 2025"),
            ("Brasileirão", "1980 · 82 · 83 · 87 · 92 · 2009"), ("", "2019 · 2020 · 2025"),
            ("Copa do Brasil", "1990 · 2006 · 2013 · 2022 · 2024"), ("Recopa Sul-Americana", "2020"),
            ("Supercopa do Brasil", "2020 · 2021 · 2025"), ("Carioca", "39 títulos")]
    y = 88
    for name, yrs in rows:
        if name:
            s.d.line([4, y-3, 298, y-3], fill=LGRY)
            s.d.polygon([(10, y+4), (18, y+4), (14, y+12)], fill=RED)  # little pennant
            s.t((24, y), name, UIB(15)); y += 19
        s.t((24, y), yrs, UI(14), DGRY); y += 22
    scrollbar(s, 304, 86, 400, 0.0, 0.7)
    s.t((8, 412), "dados ilustrativos — conferir antes", UI(12), DRED)
    # open Palm menu
    s.d.rectangle([0, 0, W, 26], fill=WHT); s.d.line([0, 26, W, 26], fill=BLK, width=2)
    s.t((10, 4), "Dados", UIB(16), WHT if False else BLK); s.d.rectangle([4, 1, 70, 24], fill=BLK); s.t((12, 4), "Dados", UIB(16), WHT)
    s.t((84, 4), "Opções", UIB(16))
    mx0, my0, mx1 = 4, 26, 220
    items_m = [("Sincronizar (HotSync)", "/S"), ("Importar tabela...", "/I"), None, ("Preferências...", "/R"), ("Sobre o Mengão", "")]
    my1 = my0 + 8 + sum(10 if it is None else 24 for it in items_m)
    s.d.rectangle([mx0+3, my0+3, mx1+3, my1+3], fill=DGRY)
    s.d.rectangle([mx0, my0, mx1, my1], fill=WHT, outline=BLK, width=2)
    yy = my0 + 4
    for i, it in enumerate(items_m):
        if it is None:
            for xx in range(mx0+6, mx1-6, 4): s.d.point((xx, yy+4), fill=BLK)
            yy += 10; continue
        if i == 0: s.d.rectangle([mx0+3, yy, mx1-3, yy+22], fill=BLK)
        fg = WHT if i == 0 else BLK
        s.t((mx0+10, yy+3), it[0], UI(15), fg); s.t((mx1-10, yy+3), it[1], UI(15), fg, "ra"); yy += 24
    statusbar(s); return save(s, "3b-mengao-info-titulos-menu")

# ---------------- contact sheet with T3-ish frame ----------------
def device(screen):
    sc = screen.resize((W*2, H*2), Image.NEAREST)
    bw, bh = W*2 + 80, H*2 + 250
    im = Image.new("RGBA", (bw, bh), (0,0,0,0)); d = ImageDraw.Draw(im)
    d.rounded_rectangle([0, 0, bw-1, bh-1], 60, fill=(118,122,130))
    d.rounded_rectangle([6, 6, bw-7, bh-7], 56, fill=(78,82,90))
    d.rounded_rectangle([24, 50, bw-25, 50+H*2+32], 14, fill=(10,10,12))
    d.text((bw//2, 28), "palmOne", font=UIB(22), fill=(200,200,205), anchor="mm")
    im.paste(sc, (40, 66))
    cy = 66 + H*2 + 100
    for cx in (110, 220, bw-220, bw-110):
        d.ellipse([cx-30, cy-30, cx+30, cy+30], fill=(55,58,64), outline=(150,154,160), width=3)
    d.ellipse([bw//2-48, cy-48, bw//2+48, cy+48], fill=(55,58,64), outline=(170,174,180), width=4)
    d.ellipse([bw//2-18, cy-18, bw//2+18, cy+18], fill=(95,98,105))
    return im

def contact(screens, labels):
    devs = [device(s) for s in screens]
    gap = 50; dw, dh = devs[0].size
    sheet = Image.new("RGB", (len(devs)*dw + (len(devs)+1)*gap, dh + 210), (24,24,28))
    d = ImageDraw.Draw(sheet)
    d.text((gap, 30), "Mengão para Palm Tungsten T3 — mockups (320×480, dados EXEMPLO)", font=UIB(40), fill=WHT)
    for i, (dv, lb) in enumerate(zip(devs, labels)):
        x = gap + i*(dw+gap); sheet.paste(dv, (x, 110), dv)
        d.text((x+dw//2, 110+dh+50), lb, font=UIB(34), fill=(255,90,90), anchor="mm")
    sheet.save(f"{OUT}/contact-sheet.png")

if __name__ == "__main__":
    a = concept1(); b = concept2(); c = concept3("jogos"); c2 = concept3("titulos")
    contact([a, b, c, c2], ["1 · Clássico Listrado", "2 · Placar & Agenda", "3 · Mengão Info (Jogos)", "3 · Mengão Info (Títulos + menu)"])
    print("ok")
