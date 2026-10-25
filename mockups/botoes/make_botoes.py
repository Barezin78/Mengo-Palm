#!/usr/bin/env python3
"""Mockups (não implementados) para a faixa de baixo da tela inicial.
Usa a prévia real do app (tools/hostsim) como base e redesenha só a área dos 4 botões.
Uso: python3 mockups/botoes/make_botoes.py  -> mockups/botoes/*.png"""
import os, sys, struct
from PIL import Image, ImageDraw
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.join(HERE, "../..")
sys.path.insert(0, os.path.join(ROOT, "tools/hostsim"))
import render as R
PDB = os.path.join(ROOT, "dist/MengaoDados.pdb")
RED, DRED, BLK, WHT, GRY, LGRY, PALE, PINK, GRN, OCH = (230,0,16), (150,0,8), (0,0,0), (255,255,255), (96,96,96), (220,220,220), (255,228,228), (255,130,130), (0,140,40), (150,120,0)
F, LH = R.FONTS, R.LH

def load():
    b = open(PDB, "rb").read(); n = struct.unpack(">H", b[76:78])[0]
    offs = [struct.unpack(">I", b[78+8*i:82+8*i])[0] for i in range(n)] + [len(b)]
    rec = [b[offs[i]:offs[i+1]] for i in range(n)]
    ng, nt = struct.unpack(">HH", rec[0][6:10]); cs = lambda x: x.split(b"\0")[0].decode("latin-1")
    games = [dict(st=r[4], home=r[5], gf=r[6], ga=r[7], pf=r[8], pa=r[9], opp=cs(r[16:20])) for r in (rec[1][i*64:(i+1)*64] for i in range(ng))]
    teams = [dict(pos=r[0], pts=r[1], j=r[2], name=cs(r[8:24])) for r in (rec[2][i*24:(i+1)*24] for i in range(nt))]
    return games, teams

def tw(s, f): return int(F[f].getlength(s))
def txt(d, s, x, y, f, fg, align="l"):
    w = tw(s, f); x = x - w//2 if align == "c" else x - w if align == "r" else x
    d.text((x, y+1), s, font=F[f], fill=fg); return w
def button(d, x, y, w, h, label, f=1, inv=False):
    d.rounded_rectangle([x, y, x+w-1, y+h-1], 7, fill=BLK)
    if not inv: d.rounded_rectangle([x+2, y+2, x+w-3, y+h-3], 5, fill=WHT)
    txt(d, label, x+w//2, y+(h-LH[f])//2, f, WHT if inv else BLK, "c")

def result(g):
    if g["gf"] > g["ga"]: return "V", GRN
    if g["gf"] < g["ga"]: return "D", RED
    return "E", OCH

def last_line(d, wy, g):
    s = (f"FLA {g['gf']} x {g['ga']} {g['opp']}" if g["home"] else f"{g['opp']} {g['ga']} x {g['gf']} FLA")
    c = result(g)[1]
    if g["pf"] != 255: s += " (p)"; c = GRN if (g["gf"] == g["ga"] and g["pf"] > g["pa"]) else c if g["gf"] != g["ga"] else RED
    x = 8 + txt(d, "Último: ", 8, wy, 1, BLK); txt(d, s, x, wy, 1, c); txt(d, "22/10", 312, wy, 0, GRY, "r")

def base(h):
    tmp = f"/tmp/_base_{h}.png"; R.render(PDB, h, 0, tmp); return Image.open(tmp).convert("RGB")

def opt_A(d, wy, H, games, teams, tall):
    """A: sem botões — mini tabela do Brasileirão; toques no cartão/tabela abrem as abas"""
    g = [x for x in games if x["st"] == 1][-1]; last_line(d, wy+6, g)
    i = next(k for k, t in enumerate(teams) if t["name"] == "Flamengo")
    rows = teams[max(0, i-1):max(0, i-1)+(3 if tall else 2)]
    y = wy + 32; rh = 22 if tall else 21
    d.rectangle([4, y-2, 315, y-1], fill=BLK)
    for t in rows:
        fla = t["name"] == "Flamengo"; bg = PALE if fla else WHT; fg = DRED if fla else BLK; f = 1 if fla else 0
        d.rectangle([4, y, 315, y+rh-1], fill=bg)
        txt(d, f"{t['pos']}º", 34, y, f, fg, "r"); txt(d, t["name"], 44, y, f, fg)
        txt(d, f"{t['pts']} pts", 250, y, 1, fg, "r"); txt(d, f"{t['j']} J", 308, y, 0, GRY, "r"); y += rh

def opt_B(d, wy, H, games, teams, tall):
    """B: atalhos úteis no lugar das abas repetidas"""
    g = [x for x in games if x["st"] == 1][-1]; last_line(d, wy+6, g)
    labels = ["Jogo", "Últ. 5", "Gols", "Abas »"]; bw = (320-8-3*6)//4
    if tall:   # 2ª linha: dica do que cada botão faz
        txt(d, "detalhes · forma · artilheiros · abas", 160, wy+36, 0, GRY, "c")
    for k, l in enumerate(labels):
        button(d, 4+k*(bw+6), H-40, bw, 34, l, 1, inv=(k == 0))

def opt_C(d, wy, H, games, teams, tall):
    """C: faixa de status — posição, pontos e forma (últimos 5)"""
    t = next(x for x in teams if x["name"] == "Flamengo"); fin = [x for x in games if x["st"] == 1][-5:]
    last_line(d, wy+6, fin[-1]); y = wy + 32; sh = H - y - 6
    d.rounded_rectangle([4, y, 315, y+sh-1], 9, fill=BLK)
    pf = 7 if tall else 1
    d.rounded_rectangle([8, y+4, 62, y+sh-5], 7, fill=RED)
    txt(d, f"{t['pos']}º", 35, y+(sh-LH[pf])//2, pf, WHT, "c")
    if tall:
        txt(d, f"{t['pts']} pts", 72, y+8, 1, WHT); txt(d, f"{t['j']} jogos", 72, y+sh-30, 0, PINK)
        txt(d, "últimos 5", 245, y+6, 0, PINK, "c"); cy = y + sh - 32
    else:
        txt(d, f"{t['pts']} pts", 72, y+(sh-LH[1])//2, 1, WHT); cy = y + (sh-24)//2
    x = 310 - 5*26 + 4
    for g in fin:
        r, c = result(g); d.rounded_rectangle([x, cy, x+22, cy+23], 4, fill=c); txt(d, r, x+11, cy, 1, WHT, "c"); x += 26

OPTS = [("A", "sem-botoes-mini-tabela", opt_A, "A · Sem botões: mini tabela"),
        ("B", "atalhos-uteis", opt_B, "B · Atalhos úteis"),
        ("C", "faixa-de-status", opt_C, "C · Faixa de status")]

def make(fn, h):
    games, teams = load(); im = base(h); d = ImageDraw.Draw(im)
    tall = h >= 440; wy = (192 if tall else 132) + 10 + (128 if tall else 88) + 10
    d.rectangle([0, wy, 319, h-1], fill=WHT); d.rectangle([0, wy, 319, wy+1], fill=BLK)
    fn(d, wy, h, games, teams, tall); return im

if __name__ == "__main__":
    out = []
    for key, slug, fn, label in OPTS:
        a, b = make(fn, 448), make(fn, 320)
        pair = Image.new("RGB", (660, 548), (30, 30, 36)); pd = ImageDraw.Draw(pair)
        pair.paste(a, (6, 34)); pair.paste(b, (334, 34))
        pd.text((330, 6), label, font=F[1], fill=WHT, anchor="ma")
        pd.text((166, 542), "Tungsten T3 · 320×480", font=F[0], fill=(170,170,170), anchor="md")
        pd.text((494, 542), "Tungsten E2 · 320×320", font=F[0], fill=(170,170,170), anchor="md")
        big = pair.resize((1320, 1096), Image.NEAREST); p = os.path.join(HERE, f"{key}-{slug}@2x.png"); big.save(p); out.append(big)
    sheet = Image.new("RGB", (1320*3 + 40, 1096), (30, 30, 36))
    for k, im in enumerate(out): sheet.paste(im, (k*(1320+20), 0))
    sheet.save(os.path.join(HERE, "comparativo-ABC@2x.png")); print("ok")
