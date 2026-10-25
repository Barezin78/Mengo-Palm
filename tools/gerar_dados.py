#!/usr/bin/env python3
"""Gera MengaoDados.pdb (Palm OS) com jogos, classificação e elenco do Flamengo a partir da API pública da ESPN.

Uso: python3 gerar_dados.py [saida.pdb]
Formato (big-endian, ver src/dados.h):
  rec0 meta  : 'MNG1', u16 ver, u16 nJogos, u16 nTabela, u16 nElenco, u32 atualizado(seg Palm, horário de Brasília), char[16] temporada
  rec1 jogos : nJogos x 64 bytes
  rec2 tabela: nTabela x 24 bytes
  rec3 elenco: nElenco x 24 bytes
"""
import json, struct, sys, time, urllib.request
from datetime import datetime, timezone
from zoneinfo import ZoneInfo

FLA = "819"
SITE = "https://site.api.espn.com/apis/site/v2/sports/soccer/"
BRT = ZoneInfo("America/Sao_Paulo")
EPOCH = datetime(1904, 1, 1)
CREATOR, TYPE, DBNAME = b"MNGO", b"DATA", "MengaoDados"
COMP = {"bra.1": "BR", "conmebol.libertadores": "LIB", "bra.copa_do_brazil": "CDB", "bra.camp.carioca": "CAR",
        "conmebol.sudamericana": "SUL", "conmebol.recopa": "REC", "fifa.cwc": "MUN",
        "fifa.intercontinental_cup": "INT", "bra.supercopa": "SUP"}

def get(url):
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0 MengaoPalm/1.0"})
    with urllib.request.urlopen(req, timeout=30) as r:
        return json.load(r)

def palm_secs(dt_utc):
    local = dt_utc.astimezone(BRT).replace(tzinfo=None)
    return int((local - EPOCH).total_seconds())

def s(txt, n):
    b = (txt or "").encode("cp1252", "replace")[: n - 1]
    return b + b"\0" * (n - len(b))

def parse_events(j):
    out = []
    for e in j.get("events", []):
        c = e["competitions"][0]
        st = c["status"]["type"]
        dt = datetime.fromisoformat(e["date"].replace("Z", "+00:00"))
        fla = opp = None
        for t in c["competitors"]:
            if t["team"]["id"] == FLA: fla = t
            else: opp = t
        if not fla or not opp: continue
        def sc(t, k="value"):
            v = t.get("score")
            if isinstance(v, dict): v = v.get(k)
            try: return int(float(v))
            except (TypeError, ValueError): return 255
        done = st.get("completed", False) or st.get("state") == "post"
        status = 1 if done else (2 if st.get("name") in ("STATUS_POSTPONED", "STATUS_CANCELED") else 0)
        pf = pa = 255
        if done and isinstance(fla.get("score"), dict) and "shootoutScore" in fla["score"]:
            pf, pa = sc(fla, "shootoutScore"), sc(opp, "shootoutScore")
        slug = (e.get("league") or {}).get("slug", "")
        out.append(dict(id=e["id"], when=palm_secs(dt), status=status, home=1 if fla.get("homeAway") == "home" else 0,
                        gf=sc(fla) if done else 255, ga=sc(opp) if done else 255, pf=pf, pa=pa,
                        comp=COMP.get(slug, slug[:5].upper()), opp=opp["team"].get("abbreviation", "???")[:3],
                        oppname=opp["team"].get("shortDisplayName") or opp["team"].get("displayName", ""),
                        venue=(c.get("venue") or {}).get("fullName", "")))
    return out

def main(path):
    games = {}
    for q in ("", "?fixture=true"):
        for g in parse_events(get(SITE + "all/teams/" + FLA + "/schedule" + q)):
            games[g["id"]] = g
    games = sorted(games.values(), key=lambda g: g["when"])
    st = get("https://site.api.espn.com/apis/v2/sports/soccer/bra.1/standings")
    ch = st["children"][0]
    season = str(st.get("seasons", [{}])[0].get("year", "")) if st.get("seasons") else ch.get("name", "")
    table = []
    for e in ch["standings"]["entries"]:
        v = {x["name"]: x.get("value", 0) for x in e["stats"]}
        table.append(dict(pos=int(v.get("rank", 0)), pts=int(v.get("points", 0)), j=int(v.get("gamesPlayed", 0)),
                          w=int(v.get("wins", 0)), d=int(v.get("ties", 0)), l=int(v.get("losses", 0)),
                          gd=int(v.get("pointDifferential", 0)), ab=e["team"].get("abbreviation", ""),
                          name=e["team"].get("shortDisplayName", "")))
    table.sort(key=lambda t: t["pos"])
    try:
        ro = get(SITE + "bra.1/teams/" + FLA + "/roster")
        order = {"G": 0, "D": 1, "M": 2, "F": 3}
        squad = [dict(num=int(a["jersey"]) if str(a.get("jersey", "")).isdigit() else 0,
                      pos=(a.get("position") or {}).get("abbreviation", "")[:2],
                      name=a.get("shortName") or a.get("displayName", "")) for a in ro.get("athletes", [])]
        squad.sort(key=lambda p: (order.get(p["pos"], 9), p["num"] or 99))
    except Exception as ex:
        print("aviso: elenco indisponível:", ex); squad = []

    now = palm_secs(datetime.now(timezone.utc))
    rec0 = b"MNG1" + struct.pack(">HHHHI", 1, len(games), len(table), len(squad), now) + s(ch.get("name", "Brasileirão"), 16)
    rec1 = b"".join(struct.pack(">IBBBBBB", g["when"], g["status"], g["home"], g["gf"], g["ga"], g["pf"], g["pa"])
                    + s(g["comp"], 6) + s(g["opp"], 4) + s(g["oppname"], 20) + s(g["venue"], 24) for g in games)
    rec2 = b"".join(struct.pack(">BBBBBBh", t["pos"], t["pts"], t["j"], t["w"], t["d"], t["l"], t["gd"])
                    + s(t["name"], 16) for t in table)
    rec3 = b"".join(struct.pack(">B", p["num"]) + s(p["pos"], 3) + s(p["name"], 20) for p in squad)
    assert len(rec1) == 64 * len(games) and len(rec2) == 24 * len(table) and len(rec3) == 24 * len(squad)
    write_pdb(path, [rec0, rec1, rec2, rec3 or b"\0"], now)
    print(f"{path}: {len(games)} jogos, {len(table)} times, {len(squad)} jogadores")

def write_pdb(path, recs, ts):
    n = len(recs); off = 78 + 8 * n + 2
    hdr = s(DBNAME, 32) + struct.pack(">HHIIIIII", 0x0008, 1, ts, ts, 0, 0, 0, 0) + TYPE + CREATOR + struct.pack(">IIH", 0, 0, n)
    # attr 0x0008 = dmHdrAttrBackup
    ent = b""
    for i, r in enumerate(recs):
        ent += struct.pack(">IB", off, 0) + (0x6F0000 + i).to_bytes(3, "big"); off += len(r)
    with open(path, "wb") as f:
        f.write(hdr + ent + b"\0\0" + b"".join(recs))

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "MengaoDados.pdb")
