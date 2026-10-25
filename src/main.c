/* Mengão 1.1.0 — app de fã do Flamengo para Palm OS 5 (Tungsten T3, 320x480 / 320x320)
 * Fonte em UTF-8; o Makefile converte para CP1252 (codificação do Palm OS) antes de compilar. */
#include <PalmOS.h>
#include <PenInputMgr.h>
#include "resource.h"

#define CRID      'MNGO'
#define DATA_TYPE 'DATA'

/* ---------- dados ---------- */
typedef struct { UInt32 when; UInt8 status, home, gf, ga, pf, pa; Char comp[6], opp[4], oppname[20], venue[24]; } Game;
typedef struct { UInt8 pos, pts, j, w, d, l; Int16 gd; Char name[16]; } Team;
typedef struct { UInt8 num; Char pos[3]; Char name[20]; } Player;

static Game   *gGames;   static UInt16 gNG;
static Team   *gTeams;   static UInt16 gNT;
static Player *gPlayers; static UInt16 gNP;
static UInt32  gUpdated;
static Boolean gHasData;

/* ---------- estado de tela ---------- */
enum { scrHome = 0, scrJogos, scrTabela, scrElenco, scrTitulos };
static UInt16  gScreen = scrHome;
static Coord   gW = 320, gH = 320;
static Boolean gPINS;
static Int16   gTop[5];
static Int16   gSel = -1;

typedef struct { Coord x, y, w, h; UInt16 action; } Hot;
#define MAXHOT 8
static Hot     gHot[MAXHOT]; static UInt16 gNHot;
enum { actJogos = 1, actTabela, actElenco, actTitulos, actHome, actDetalhes };

/* buffer fora da tela (anti-piscada): tudo é desenhado aqui e depois copiado de uma vez */
static WinHandle gOff;  static Coord gOffW, gOffH;
static UInt32  gShownMin = 0xFFFFFFFFUL;   /* minuto mostrado no relógio da tela inicial */

/* list geometry of the current tab screen (native coords) */
static Coord gListY0, gListY1, gRowH; static UInt16 gRows;

/* ---------- utilidades ---------- */
static UInt16 rd16(const UInt8 *p) { return ((UInt16)p[0] << 8) | p[1]; }
static UInt32 rd32(const UInt8 *p) { return ((UInt32)p[0] << 24) | ((UInt32)p[1] << 16) | ((UInt32)p[2] << 8) | p[3]; }
static void cpy(Char *dst, const UInt8 *src, UInt16 n) { MemMove(dst, src, n); dst[n - 1] = 0; }

static void Cat(Char *d, const Char *s) { StrCat(d, s); }
static void CatN(Char *d, Int32 n) { Char b[12]; StrIToA(b, n); StrCat(d, b); }
static void Cat2(Char *d, Int16 n) { Char b[3]; b[0] = '0' + (n / 10) % 10; b[1] = '0' + n % 10; b[2] = 0; StrCat(d, b); }

static const Char *kWd[7] = { "Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sáb" };

static const Char *CompName(const Char *c) {
    if (!StrCompare(c, "BR"))  return "Brasileirão";
    if (!StrCompare(c, "LIB")) return "Libertadores";
    if (!StrCompare(c, "CDB")) return "Copa do Brasil";
    if (!StrCompare(c, "CAR")) return "Carioca";
    if (!StrCompare(c, "SUL")) return "Sul-Americana";
    if (!StrCompare(c, "REC")) return "Recopa";
    if (!StrCompare(c, "SUP")) return "Supercopa";
    if (!StrCompare(c, "INT")) return "Intercontinental";
    if (!StrCompare(c, "MUN")) return "Mundial";
    return c;
}

/* ---------- carga do PDB ---------- */
static void *LoadArr(DmOpenRef db, UInt16 rec, UInt16 *n, UInt16 recSize, UInt16 memSize, UInt8 **pp, MemHandle *hh) {
    MemHandle h = DmQueryRecord(db, rec);
    UInt32 sz; void *mem;
    *pp = NULL; *hh = NULL;
    if (!h) { *n = 0; return NULL; }
    sz = MemHandleSize(h);
    if ((UInt32)*n * recSize > sz) *n = (UInt16)(sz / recSize);
    if (*n == 0) return NULL;
    mem = MemPtrNew((UInt32)*n * memSize);
    if (!mem) { *n = 0; return NULL; }
    MemSet(mem, (UInt32)*n * memSize, 0);
    *hh = h; *pp = (UInt8 *)MemHandleLock(h);
    return mem;
}

static void LoadData(void) {
    DmOpenRef db = DmOpenDatabaseByTypeCreator(DATA_TYPE, CRID, dmModeReadOnly);
    MemHandle h; UInt8 *p; UInt16 i;
    if (!db) return;
    if (DmNumRecords(db) >= 4 && (h = DmQueryRecord(db, 0)) != NULL && MemHandleSize(h) >= 16) {
        p = (UInt8 *)MemHandleLock(h);
        if (p[0] == 'M' && p[1] == 'N' && p[2] == 'G' && p[3] == '1') {
            gNG = rd16(p + 6); gNT = rd16(p + 8); gNP = rd16(p + 10); gUpdated = rd32(p + 12);
            gHasData = true;
        }
        MemHandleUnlock(h);
    }
    if (gHasData) {
        gGames = (Game *)LoadArr(db, 1, &gNG, 64, sizeof(Game), &p, &h);
        for (i = 0; i < gNG; i++) {
            UInt8 *r = p + i * 64; Game *g = &gGames[i];
            g->when = rd32(r); g->status = r[4]; g->home = r[5]; g->gf = r[6]; g->ga = r[7]; g->pf = r[8]; g->pa = r[9];
            cpy(g->comp, r + 10, 6); cpy(g->opp, r + 16, 4); cpy(g->oppname, r + 20, 20); cpy(g->venue, r + 40, 24);
        }
        if (h) MemHandleUnlock(h);
        gTeams = (Team *)LoadArr(db, 2, &gNT, 24, sizeof(Team), &p, &h);
        for (i = 0; i < gNT; i++) {
            UInt8 *r = p + i * 24; Team *t = &gTeams[i];
            t->pos = r[0]; t->pts = r[1]; t->j = r[2]; t->w = r[3]; t->d = r[4]; t->l = r[5];
            t->gd = (Int16)rd16(r + 6); cpy(t->name, r + 8, 16);
        }
        if (h) MemHandleUnlock(h);
        gPlayers = (Player *)LoadArr(db, 3, &gNP, 24, sizeof(Player), &p, &h);
        for (i = 0; i < gNP; i++) {
            UInt8 *r = p + i * 24; Player *pl = &gPlayers[i];
            pl->num = r[0]; cpy(pl->pos, r + 1, 3); cpy(pl->name, r + 4, 20);
        }
        if (h) MemHandleUnlock(h);
    }
    DmCloseDatabase(db);
}

static void FreeData(void) {
    if (gGames) MemPtrFree(gGames);
    if (gTeams) MemPtrFree(gTeams);
    if (gPlayers) MemPtrFree(gPlayers);
}

/* next scheduled game (index) and last finished game */
static Int16 NextGame(UInt32 now) {
    UInt16 i;
    for (i = 0; i < gNG; i++)
        if (gGames[i].status == 0 && gGames[i].when + 2 * 3600UL >= now) return i;
    return -1;
}
static Int16 LastGame(UInt32 now) {
    Int16 i;
    for (i = (Int16)gNG - 1; i >= 0; i--)
        if (gGames[i].status == 1 && gGames[i].when <= now) return i;
    return -1;
}

/* ---------- desenho (coordenadas nativas, 320 de largura) ---------- */
#define RGB(r, g, b) (((UInt32)(r) << 16) | ((UInt32)(g) << 8) | (b))
#define cRED   RGB(230, 0, 16)
#define cDRED  RGB(150, 0, 8)
#define cBLK   RGB(0, 0, 0)
#define cWHT   RGB(255, 255, 255)
#define cGRY   RGB(96, 96, 96)
#define cLGRY  RGB(220, 220, 220)
#define cPALE  RGB(255, 228, 228)
#define cPINK  RGB(255, 130, 130)
#define cGRN   RGB(0, 140, 40)
#define cOCHRE RGB(150, 120, 0)

static void ToRGB(UInt32 c, RGBColorType *r) { r->index = 0; r->r = (UInt8)(c >> 16); r->g = (UInt8)(c >> 8); r->b = (UInt8)c; }
static void Fg(UInt32 c) { RGBColorType r; ToRGB(c, &r); WinSetForeColorRGB(&r, NULL); }

static void Box(Coord x, Coord y, Coord w, Coord h, UInt32 c, UInt16 corner) {
    RectangleType r; r.topLeft.x = x; r.topLeft.y = y; r.extent.x = w; r.extent.y = h;
    Fg(c); WinDrawRectangle(&r, corner);
}
static void Line(Coord x1, Coord y1, Coord x2, Coord y2, UInt32 c) { Fg(c); WinDrawLine(x1, y1, x2, y2); }

enum { aL = 0, aC, aR };
static Coord Txt(const Char *s, Coord x, Coord y, FontID f, UInt32 fg, UInt32 bg, UInt16 align) {
    RGBColorType r; Int16 len = StrLen(s); Coord w;
    FntSetFont(f); w = FntCharsWidth(s, len);
    if (align == aC) x -= w / 2; else if (align == aR) x -= w;
    ToRGB(fg, &r); WinSetTextColorRGB(&r, NULL);
    ToRGB(bg, &r); WinSetBackColorRGB(&r, NULL);
    WinDrawChars(s, len, x, y);
    return w;
}
static Coord FH(FontID f) { FntSetFont(f); return FntLineHeight(); }

static void AddHot(Coord x, Coord y, Coord w, Coord h, UInt16 a) {
    if (gNHot < MAXHOT) { Hot *t = &gHot[gNHot++]; t->x = x; t->y = y; t->w = w; t->h = h; t->action = a; }
}

/* Palm-style rounded button */
static void Button(Coord x, Coord y, Coord w, Coord h, const Char *label, UInt16 action) {
    Box(x, y, w, h, cBLK, 7);
    Box(x + 2, y + 2, w - 4, h - 4, cWHT, 5);
    Txt(label, x + w / 2, y + (h - FH(boldFont)) / 2, boldFont, cBLK, cWHT, aC);
    AddHot(x, y, w, h, action);
}

/* Palm OS form title: tab + rule */
static void Title(const Char *t, UInt32 tab, UInt32 bar) {
    Coord w = 0;
    FntSetFont(boldFont); w = FntCharsWidth(t, StrLen(t)) + 18;
    if (bar != cWHT) Box(0, 0, gW, 26, bar, 0);
    Box(0, 0, w, 26, tab, 6);
    Box(0, 0, 8, 26, tab, 0);
    Box(0, 0, w, 8, tab, 0);
    Txt(t, 9, (26 - FH(boldFont)) / 2, boldFont, cWHT, tab, aL);
    Box(0, 24, gW, 2, bar == cWHT ? cBLK : tab, 0);
}

/* escudo genérico rubro-negro (não é o escudo oficial) */
static void Shield(Coord x, Coord y, Coord w, Coord h, Boolean crf) {
    Coord i, k = (h * 58) / 100;
    for (i = 0; i < h; i++) {
        Coord span = (i < k) ? w : (Coord)(((Int32)w * (h - i)) / (h - k));
        Coord xl = x + (w - span) / 2;
        UInt32 c = (((i * 7) / h) % 2) ? cRED : cBLK;
        if (span > 0) Line(xl, y + i, xl + span - 1, y + i, c);
    }
    Fg(cWHT);
    WinDrawLine(x, y, x + w - 1, y); WinDrawLine(x, y + 1, x + w - 1, y + 1);
    WinDrawLine(x, y, x, y + k); WinDrawLine(x + 1, y, x + 1, y + k);
    WinDrawLine(x + w - 1, y, x + w - 1, y + k); WinDrawLine(x + w - 2, y, x + w - 2, y + k);
    WinDrawLine(x, y + k, x + w / 2, y + h - 1); WinDrawLine(x + 1, y + k, x + w / 2, y + h - 2);
    WinDrawLine(x + w - 1, y + k, x + w / 2, y + h - 1); WinDrawLine(x + w - 2, y + k, x + w / 2, y + h - 2);
    if (crf) {
        FontID f = (h >= 70) ? largeBoldFont : boldFont;
        Coord fh = FH(f), cy = y + (h * 40) / 100;
        Box(x + 4, cy - fh / 2 - 2, w - 8, fh + 4, cBLK, 0);
        Txt("CRF", x + w / 2, cy - fh / 2, f, cWHT, cBLK, aC);
    }
}

/* big 7-segment digits for the clock */
static const UInt8 kSeg[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
static void Digit(Coord x, Coord y, Coord w, Coord h, Coord t, UInt8 d, UInt32 c) {
    UInt8 m = kSeg[d % 10]; Coord h2 = h / 2;
    if (m & 0x01) Box(x, y, w, t, c, 0);
    if (m & 0x02) Box(x + w - t, y, t, h2 + t / 2, c, 0);
    if (m & 0x04) Box(x + w - t, y + h2 - t / 2, t, h - h2 + t / 2, c, 0);
    if (m & 0x08) Box(x, y + h - t, w, t, c, 0);
    if (m & 0x10) Box(x, y + h2 - t / 2, t, h - h2 + t / 2, c, 0);
    if (m & 0x20) Box(x, y, t, h2 + t / 2, c, 0);
    if (m & 0x40) Box(x, y + h2 - t / 2, w, t, c, 0);
}
static void BigClock(Coord cx, Coord y, Coord w, Coord h, Coord t, Int16 hh, Int16 mm) {
    Coord g = t / 2 + 2, total = 4 * w + 4 * g + t, x = cx - total / 2;
    Digit(x, y, w, h, t, hh / 10, cWHT); x += w + g;
    Digit(x, y, w, h, t, hh % 10, cWHT); x += w + g;
    Box(x, y + h / 4, t, t, cWHT, 0); Box(x, y + (3 * h) / 4 - t, t, t, cWHT, 0); x += t + g;
    Digit(x, y, w, h, t, mm / 10, cWHT); x += w + g;
    Digit(x, y, w, h, t, mm % 10, cWHT);
}

static void FmtDate(Char *b, UInt32 secs, Boolean wd, Boolean hour) {
    DateTimeType dt; TimSecondsToDateTime(secs, &dt);
    b[0] = 0;
    if (wd) { Cat(b, kWd[dt.weekDay % 7]); Cat(b, " "); }
    Cat2(b, dt.day); Cat(b, "/"); Cat2(b, dt.month);
    if (hour) { Cat(b, " · "); Cat2(b, dt.hour); Cat(b, "h"); Cat2(b, dt.minute); }
}

static void MatchStr(Char *b, const Game *g, Boolean score) {
    b[0] = 0;
    Cat(b, g->home ? "FLA" : g->opp);
    if (score && g->status == 1) { Cat(b, " "); CatN(b, g->home ? g->gf : g->ga); Cat(b, " x "); CatN(b, g->home ? g->ga : g->gf); Cat(b, " "); }
    else Cat(b, " x ");
    Cat(b, g->home ? g->opp : "FLA");
}

static UInt32 ResultColor(const Game *g) {
    if (g->gf > g->ga) return cGRN;
    if (g->gf < g->ga) return cRED;
    if (g->pf != 255) return (g->pf > g->pa) ? cGRN : cRED;
    return cOCHRE;
}

/* ---------- tela inicial ---------- */
/* mini tabela do Brasileirão (v1.1): Flamengo + vizinhos; toque abre a aba Tabela */
static void DrawMiniTable(Coord y, Coord yMax, UInt16 n) {
    Int16 i, fla = -1, start; Coord rh = (n >= 3) ? 22 : 21, y0 = y;
    Char b[16];
    for (i = 0; i < (Int16)gNT; i++) if (!StrCompare(gTeams[i].name, "Flamengo")) { fla = i; break; }
    while (n > 1 && y + (Coord)n * rh > yMax - 2) n--;            /* adapta ao espaço que sobrar */
    if (n > gNT) n = gNT;
    start = (fla <= 0) ? 0 : fla - 1;                              /* 1º: mostra 1..n; senão: acima, FLA, abaixo */
    if (start + (Int16)n > (Int16)gNT) start = (Int16)gNT - (Int16)n;
    if (start < 0) start = 0;
    Box(4, y - 2, gW - 8, 2, cBLK, 0);
    for (i = start; i < start + (Int16)n; i++, y += rh) {
        Team *t = &gTeams[i]; Boolean me = (i == fla);
        UInt32 bg = me ? cPALE : cWHT, fg = me ? cDRED : cBLK; FontID f = me ? boldFont : stdFont;
        Coord ty = y + (rh - FH(f)) / 2;
        Box(4, y, gW - 8, rh, bg, 0);
        b[0] = 0; CatN(b, t->pos); Cat(b, "º"); Txt(b, 34, ty, f, fg, bg, aR);
        Txt(t->name, 44, ty, f, fg, bg, aL);
        b[0] = 0; CatN(b, t->pts); Cat(b, " pts"); Txt(b, 250, ty, boldFont, fg, bg, aR);
        b[0] = 0; CatN(b, t->j); Cat(b, " J"); Txt(b, gW - 12, ty, stdFont, cGRY, bg, aR);
    }
    AddHot(0, y0 - 4, gW, y - y0 + 4, actTabela);
}

static void DrawHome(void) {
    Boolean tall = gH >= 440;
    UInt32 now = TimGetSeconds();
    DateTimeType dt; Char b[64];
    Coord i, n = tall ? 10 : 8, py1, cy0, cy1, wy;
    Int16 ng, lg;

    TimSecondsToDateTime(now, &dt);
    for (i = 0; i < n; i++) {
        Coord y0 = 26 + ((gH - 26) * i) / n, y1 = 26 + ((gH - 26) * (i + 1)) / n;
        Box(0, y0, gW, y1 - y0, (i % 2) ? cBLK : cRED, 0);
    }
    Title("Mengão", cRED, cBLK);
    b[0] = 0; Cat(b, kWd[dt.weekDay % 7]); Cat(b, " "); Cat2(b, dt.day); Cat(b, "/"); Cat2(b, dt.month);
    Txt(b, gW - 6, (26 - FH(boldFont)) / 2, boldFont, cWHT, cBLK, aR);

    /* painel do relógio */
    py1 = tall ? 192 : 132;
    Box(8, 34, gW - 16, py1 - 34, cWHT, 10);
    Box(10, 36, gW - 20, py1 - 38, cBLK, 9);
    if (tall) {
        Shield(22, 48, 62, 78, true);
        Txt("DESDE", 53, 132, stdFont, cPINK, cBLK, aC);
        Txt("1895", 53, 154, boldFont, cWHT, cBLK, aC);
        BigClock(204, 50, 38, 92, 12, dt.hour, dt.minute);
    } else {
        Shield(20, 42, 48, 60, true);
        Txt("1895", 44, 104, boldFont, cPINK, cBLK, aC);
        BigClock(200, 44, 28, 56, 9, dt.hour, dt.minute);
    }
    {
        static const Char *mes[12] = { "janeiro", "fevereiro", "março", "abril", "maio", "junho", "julho",
                                       "agosto", "setembro", "outubro", "novembro", "dezembro" };
        static const Char *wdl[7] = { "domingo", "segunda", "terça", "quarta", "quinta", "sexta", "sábado" };
        b[0] = 0; Cat(b, wdl[dt.weekDay % 7]); Cat(b, ", "); CatN(b, dt.day); Cat(b, " de "); Cat(b, mes[(dt.month + 11) % 12]);
        Txt(b, tall ? 204 : 200, tall ? 156 : 104, tall ? boldFont : stdFont, cPINK, cBLK, aC);
    }

    /* cartão "Próximo jogo" */
    cy0 = py1 + 10; cy1 = cy0 + (tall ? 128 : 88);
    Box(8, cy0, gW - 16, cy1 - cy0, cWHT, 10);
    Box(8, cy0, gW - 16, 24, cRED, 10); Box(8, cy0 + 12, gW - 16, 12, cRED, 0);
    Txt("PRÓXIMO JOGO", 18, cy0 + (24 - FH(boldFont)) / 2, boldFont, cWHT, cRED, aL);
    ng = gHasData ? NextGame(now) : -1;
    if (!gHasData) {
        Txt("Sem dados :(", gW / 2, cy0 + 32, boldFont, cBLK, cWHT, aC);
        Txt("Importe MengaoDados.pdb", gW / 2, cy0 + 56, stdFont, cGRY, cWHT, aC);
    } else if (ng < 0) {
        Txt("Nenhum jogo agendado", gW / 2, cy0 + 40, boldFont, cGRY, cWHT, aC);
    } else {
        Game *g = &gGames[ng];
        if (g->when > now) {
            UInt32 d = g->when - now, days = d / 86400UL, hrs = (d % 86400UL) / 3600UL, mins = (d % 3600UL) / 60;
            b[0] = 0; Cat(b, "em ");
            if (days) { CatN(b, days); Cat(b, "d "); Cat2(b, (Int16)hrs); Cat(b, "h"); }
            else { CatN(b, hrs); Cat(b, "h "); Cat2(b, (Int16)mins); Cat(b, "min"); }
        } else StrCopy(b, "AGORA!");
        Txt(b, gW - 18, cy0 + (24 - FH(boldFont)) / 2, boldFont, cWHT, cRED, aR);
        MatchStr(b, g, false);
        Txt(b, gW / 2, cy0 + (tall ? 34 : 28), largeBoldFont, cBLK, cWHT, aC);
        FmtDate(b, g->when, true, true); Cat(b, " · "); Cat(b, CompName(g->comp));
        Txt(b, gW / 2, cy0 + (tall ? 70 : 60), tall ? boldFont : stdFont, tall ? cDRED : cGRY, cWHT, aC);
        if (tall && g->venue[0]) Txt(g->venue, gW / 2, cy0 + 96, stdFont, cGRY, cWHT, aC);
    }

    /* faixa branca: último resultado + mini tabela (v1.1: sem os 4 botões) */
    wy = cy1 + 10;
    Box(0, wy, gW, gH - wy, cWHT, 0); Box(0, wy, gW, 2, cBLK, 0);
    lg = gHasData ? LastGame(now) : -1;
    if (lg >= 0) {
        /* mede tudo com FntCharsWidth para o texto nunca encostar na data (fontes reais do Palm) */
        Game *g = &gGames[lg]; Coord x, dx, room; Char d[8], pen[16]; UInt16 k;
        FmtDate(d, g->when, false, false);
        dx = gW - 8 - Txt(d, gW - 8, wy + 6, stdFont, cGRY, cWHT, aR);   /* data à direita */
        MatchStr(b, g, true);
        {   /* rótulo encurta se nem o placar couber */
            const Char *lab = "Último: ";
            FntSetFont(boldFont);
            if (8 + FntCharsWidth(lab, StrLen(lab)) + FntCharsWidth(b, StrLen(b)) > dx - 10) lab = "Últ.: ";
            x = 8 + Txt(lab, 8, wy + 6, boldFont, cBLK, cWHT, aL);
        }
        room = dx - 10 - x;                                          /* espaço livre até a data */
        pen[0] = 0;
        if (g->pf != 255) {                                          /* placar dos pênaltis, na ordem do jogo */
            StrCopy(pen, " pên. "); CatN(pen, g->home ? g->pf : g->pa); Cat(pen, "x"); CatN(pen, g->home ? g->pa : g->pf);
        }
        for (k = 0; k < 3; k++) {                                     /* "pên. 4x3" -> "(p)" -> sem pênaltis */
            Char t[48]; StrCopy(t, b); Cat(t, pen);
            FntSetFont(boldFont);
            if (FntCharsWidth(t, StrLen(t)) <= room || !pen[0]) { StrCopy(b, t); break; }
            if (k == 0) StrCopy(pen, " (p)"); else pen[0] = 0;
        }
        Txt(b, x, wy + 6, boldFont, ResultColor(g), cWHT, aL);
    }
    gNHot = 0;
    AddHot(8, cy0, gW - 16, cy1 - cy0, actJogos);              /* toque no cartão -> Jogos */
    if (gHasData && gNT) DrawMiniTable(wy + 32, gH, tall ? 3 : 2);
}

/* ---------- telas com abas ---------- */
static const Char *kTitulos[] = {
    "#Mundial Interclubes (1)", "1981",
    "#Libertadores (4)", "1981 · 2019 · 2022 · 2025",
    "#Campeonato Brasileiro (9)", "1980 · 1982 · 1983 · 1987*", "1992 · 2009 · 2019", "2020 · 2025",
    "#Copa do Brasil (5)", "1990 · 2006 · 2013", "2022 · 2024",
    "#Supercopa do Brasil (3)", "2020 · 2021 · 2025",
    "#Recopa Sul-Americana (1)", "2020",
    "#Copa Mercosul (1)", "1999",
    "#Campeonato Carioca (40)", "o mais recente: 2026",
    "", "*1987: reconhecido pelo clube;", "  CBF/FIFA reconhecem o Sport.",
    "Fonte: flamengo.com.br, out/2026"
};
#define NTIT (sizeof(kTitulos) / sizeof(kTitulos[0]))

static UInt16 RowCount(void) {
    switch (gScreen) {
        case scrJogos: return gNG;
        case scrTabela: return gNT;
        case scrElenco: return gNP;
        case scrTitulos: return NTIT;
    }
    return 0;
}

static void DrawRow(UInt16 i, Coord y) {
    Char b[48];
    Coord h = gRowH, ty = y + (h - FH(stdFont)) / 2, right = gW - 18;
    if (gScreen == scrJogos) {
        Game *g = &gGames[i]; Boolean sel = (Int16)i == gSel;
        UInt32 bg = sel ? cBLK : cWHT, fg = sel ? cWHT : cBLK;
        Box(0, y, right + 4, h, bg, 0);
        FmtDate(b, g->when, true, false);
        Txt(b, 6, ty, boldFont, fg, bg, aL);
        Txt(g->comp, 110, ty, stdFont, sel ? cPINK : cGRY, bg, aL);
        {   /* resultado/horário à direita primeiro; o confronto usa o espaço que sobrar (sem encavalar) */
            Coord rw, mx = 152, room; Char m[16]; FontID mf = boldFont;
            if (g->status == 1) {
                b[0] = 0; CatN(b, g->gf); Cat(b, "x"); CatN(b, g->ga);
                if (g->pf != 255) Cat(b, "p");
                rw = Txt(b, right, ty, boldFont, sel ? cWHT : ResultColor(g), bg, aR);
            } else if (g->status == 2) {
                rw = Txt("adiado", right, ty, stdFont, sel ? cWHT : cGRY, bg, aR);
            } else {
                DateTimeType dt; TimSecondsToDateTime(g->when, &dt);
                b[0] = 0; Cat2(b, dt.hour); Cat(b, ":"); Cat2(b, dt.minute);
                rw = Txt(b, right, ty, boldFont, sel ? cPINK : cDRED, bg, aR);
            }
            room = right - rw - 8 - mx;
            MatchStr(m, g, false);
            FntSetFont(mf);
            if (FntCharsWidth(m, StrLen(m)) > room) mf = stdFont;      /* não coube em negrito */
            FntSetFont(mf);
            if (FntCharsWidth(m, StrLen(m)) > room) {                  /* ainda não: puxa para a esquerda */
                mx = right - rw - 8 - FntCharsWidth(m, StrLen(m));
                if (mx < 140) mx = 140;
            }
            Txt(m, mx, ty, mf, fg, bg, aL);
        }
        if (!sel) Line(0, y + h - 1, right + 3, y + h - 1, cLGRY);
    } else if (gScreen == scrTabela) {
        Team *t = &gTeams[i];
        Boolean fla = StrCompare(t->name, "Flamengo") == 0;
        UInt32 bg = fla ? cPALE : cWHT, fg = fla ? cDRED : cBLK;
        FontID f = fla ? boldFont : stdFont;
        Box(0, y, right + 4, h, bg, 0);
        b[0] = 0; CatN(b, t->pos); Txt(b, 28, ty, f, fg, bg, aR);
        Txt(t->name, 38, ty, f, fg, bg, aL);
        b[0] = 0; CatN(b, t->pts); Txt(b, 206, ty, boldFont, fg, bg, aR);
        b[0] = 0; CatN(b, t->j);   Txt(b, 238, ty, f, fg, bg, aR);
        b[0] = 0; CatN(b, t->w);   Txt(b, 266, ty, f, fg, bg, aR);
        b[0] = 0; if (t->gd > 0) Cat(b, "+"); CatN(b, t->gd); Txt(b, right, ty, f, fg, bg, aR);
        Line(0, y + h - 1, right + 3, y + h - 1, cLGRY);
    } else if (gScreen == scrElenco) {
        Player *p = &gPlayers[i];
        Box(0, y, right + 4, h, cWHT, 0);
        if (p->num) { b[0] = 0; CatN(b, p->num); Txt(b, 36, ty, boldFont, cDRED, cWHT, aR); }
        Txt(p->pos, 48, ty, stdFont, cGRY, cWHT, aL);
        Txt(p->name, 84, ty, stdFont, cBLK, cWHT, aL);
        Line(0, y + h - 1, right + 3, y + h - 1, cLGRY);
    } else if (gScreen == scrTitulos) {
        const Char *s = kTitulos[i];
        Box(0, y, right + 4, h, cWHT, 0);
        if (s[0] == '#') {
            Fg(cRED); Box(8, y + h / 2 - 5, 10, 10, cRED, 0);
            Txt(s + 1, 26, ty, boldFont, cBLK, cWHT, aL);
        } else Txt(s, 26, ty, stdFont, (s[0] == '*' || s[0] == ' ' || s[0] == 'F') ? cDRED : cGRY, cWHT, aL);
    }
}

static void DrawScrollbar(UInt16 count) {
    Coord x = gW - 14, y0 = gListY0, y1 = gListY1, track = y1 - y0 - 24;
    Box(x, y0, 14, y1 - y0, cWHT, 0);
    if (count <= gRows) return;
    Fg(cBLK);
    { Coord k; for (k = 0; k < 8; k++) { WinDrawLine(x + 7 - k, y0 + 2 + k, x + 7 + k - 1, y0 + 2 + k);
                                           WinDrawLine(x + 7 - k, y1 - 3 - k, x + 7 + k - 1, y1 - 3 - k); } }
    Line(x + 7, y0 + 12, x + 7, y1 - 12, cGRY);
    {
        Coord th = (Coord)(((Int32)track * gRows) / count), ty;
        if (th < 12) th = 12;
        ty = y0 + 12 + (Coord)(((Int32)(track - th) * gTop[gScreen]) / (count - gRows));
        Box(x + 3, ty, 8, th, cBLK, 0);
    }
}

static void DrawTabs(void) {
    static const Char *names[4] = { "Jogos", "Tabela", "Elenco", "Títulos" };
    Coord x0 = 4, w = (gW - 8) / 4, i;
    for (i = 0; i < 4; i++) {
        Boolean sel = (gScreen == scrJogos + i);
        Box(x0 + i * w, 32, w, 28, cBLK, 0);
        if (!sel) Box(x0 + i * w + 2, 34, w - 4, 24, cWHT, 0);
        Txt(names[i], x0 + i * w + w / 2, 32 + (28 - FH(boldFont)) / 2, boldFont, sel ? cWHT : cBLK, sel ? cBLK : cWHT, aC);
    }
}

static void DrawList(void) {
    UInt16 count = RowCount(), i;
    Coord y;
    Int16 maxTop = (Int16)count - (Int16)gRows;
    if (maxTop < 0) maxTop = 0;
    if (gTop[gScreen] > maxTop) gTop[gScreen] = maxTop;
    if (gTop[gScreen] < 0) gTop[gScreen] = 0;
    Box(0, gListY0, gW, gListY1 - gListY0, cWHT, 0);
    for (i = 0, y = gListY0; i < gRows && gTop[gScreen] + i < count; i++, y += gRowH)
        DrawRow(gTop[gScreen] + i, y);
    DrawScrollbar(count);
}

static void DrawTabScreen(void) {
    Char b[40]; Coord hy = 66;
    WinEraseWindow();
    Box(0, 0, gW, gH, cWHT, 0);
    Title("Mengão", cBLK, cWHT);
    Shield(gW - 26, 2, 18, 21, false);
    DrawTabs();
    gNHot = 0;
    gRowH = (gScreen == scrJogos || gScreen == scrTitulos) ? 26 : 25;

    if (gScreen == scrTabela) {
        Box(0, hy, gW, 24, cBLK, 0);
        Txt("#", 28, hy + 1, boldFont, cWHT, cBLK, aR);
        Txt("Brasileirão", 38, hy + 1, boldFont, cWHT, cBLK, aL);
        Txt("P", 206, hy + 1, boldFont, cWHT, cBLK, aR);
        Txt("J", 238, hy + 1, boldFont, cWHT, cBLK, aR);
        Txt("V", 266, hy + 1, boldFont, cWHT, cBLK, aR);
        Txt("SG", gW - 18, hy + 1, boldFont, cWHT, cBLK, aR);
        hy += 26;
    } else if (gScreen == scrJogos || gScreen == scrElenco) {
        if (gScreen == scrJogos) StrCopy(b, "Jogos 2026"); else StrCopy(b, "Elenco");
        Txt(b, 6, hy, boldFont, cBLK, cWHT, aL);
        if (gUpdated) { StrCopy(b, "atual. "); { Char d[24]; FmtDate(d, gUpdated, false, true); Cat(b, d); }
                        Txt(b, gW - 6, hy, stdFont, cGRY, cWHT, aR); }
        Line(0, hy + 23, gW, hy + 23, cBLK);
        hy += 26;
    }
    gListY0 = hy; gListY1 = gH - 46;
    gRows = (UInt16)((gListY1 - gListY0) / gRowH);
    gListY1 = gListY0 + gRows * gRowH;

    if (!gHasData && gScreen != scrTitulos) {
        Txt("Sem dados.", gW / 2, gListY0 + 40, boldFont, cBLK, cWHT, aC);
        Txt("Importe MengaoDados.pdb", gW / 2, gListY0 + 66, stdFont, cGRY, cWHT, aC);
        Txt("(veja o README)", gW / 2, gListY0 + 90, stdFont, cGRY, cWHT, aC);
    } else if (RowCount() == 0) {
        Txt("Nada por aqui.", gW / 2, gListY0 + 40, boldFont, cGRY, cWHT, aC);
    } else DrawList();

    Line(0, gH - 44, gW, gH - 44, cLGRY);
    Button(4, gH - 40, 92, 34, "Início", actHome);
    if (gScreen == scrJogos && gHasData) Button(102, gH - 40, 104, 34, "Detalhes", actDetalhes);
}

/* ---------- buffer fora da tela ----------
 * v1.0 desenhava a tela inicial inteira direto na tela (fundo listrado, depois painéis, depois textos)
 * a cada nilEvent: dava pra ver os passos intermediários = "tela piscando".
 * Agora desenha num offscreen window (densidade nativa) e copia o retângulo pronto com WinCopyRectangle. */
static void FreeOff(void) {
    if (gOff) { WinDeleteWindow(gOff, false); gOff = NULL; }
}
static WinHandle GetOff(void) {
    Err err = errNone; UInt16 old;
    if (gOff && gOffW == gW && gOffH == gH) return gOff;
    FreeOff();                                         /* tamanho mudou (320x480 <-> 320x320): recria */
    old = WinSetCoordinateSystem(kCoordinatesStandard);
    gOff = WinCreateOffscreenWindow(gW / 2, gH / 2, nativeFormat, &err);
    WinSetCoordinateSystem(old);
    if (err != errNone) gOff = NULL;                   /* sem memória: cai no desenho direto */
    if (gOff) { gOffW = gW; gOffH = gH; }
    return gOff;
}

/* desenha com fn() (no buffer, se houver) e copia só o retângulo (x,y,w,h) para a tela */
static void Render(void (*fn)(void), Coord x, Coord y, Coord w, Coord h) {
    FormPtr frm = FrmGetActiveForm();
    WinHandle scr = frm ? FrmGetWindowHandle(frm) : WinGetDrawWindow(), off = GetOff();
    UInt16 old;
    WinSetDrawWindow(off ? off : scr);
    WinPushDrawState();
    old = WinSetCoordinateSystem(kCoordinatesNative);
    fn();
    WinSetCoordinateSystem(old);
    WinPopDrawState();
    if (off) {
        RectangleType r; r.topLeft.x = x; r.topLeft.y = y; r.extent.x = w; r.extent.y = h;
        WinSetDrawWindow(scr);
        WinPushDrawState();
        old = WinSetCoordinateSystem(kCoordinatesNative);
        WinCopyRectangle(off, scr, &r, x, y, winPaint);
        WinSetCoordinateSystem(old);
        WinPopDrawState();
    }
}

static void DrawScreen(void) {
    if (gScreen == scrHome) { DrawHome(); gShownMin = TimGetSeconds() / 60; }
    else DrawTabScreen();
}
static void Draw(void) { Render(DrawScreen, 0, 0, gW, gH); }

/* redraw just the list area (scrolling / selection) */
static void DrawListOnly(void) { Render(DrawList, 0, gListY0, gW, gListY1 - gListY0); }

/* tique do relógio: só redesenha se o minuto mostrado mudou (nilEvents extras não fazem nada) */
static void Tick(void) {
    if (gScreen != scrHome) return;
    if (TimGetSeconds() / 60 == gShownMin) return;
    Draw();
}

/* ---------- ações ---------- */
static void ShowDetails(Int16 i) {
    Char a[48], b[48], c[96];
    Game *g;
    if (i < 0 || i >= (Int16)gNG) return;
    g = &gGames[i];
    FmtDate(a, g->when, true, true);
    { DateTimeType dt; TimSecondsToDateTime(g->when, &dt); Cat(a, "  "); CatN(a, dt.year); }
    b[0] = 0;
    Cat(b, g->home ? "Flamengo" : g->oppname); Cat(b, " x "); Cat(b, g->home ? g->oppname : "Flamengo");
    StrCopy(c, CompName(g->comp));
    if (g->venue[0]) { Cat(c, "\n"); Cat(c, g->venue); }
    if (g->status == 1) {
        Cat(c, "\nPlacar: "); CatN(c, g->home ? g->gf : g->ga); Cat(c, " x "); CatN(c, g->home ? g->ga : g->gf);
        if (g->pf != 255) { Cat(c, " (pên. "); CatN(c, g->home ? g->pf : g->pa); Cat(c, "x"); CatN(c, g->home ? g->pa : g->pf); Cat(c, ")"); }
    } else if (g->status == 2) Cat(c, "\nAdiado");
    FrmCustomAlert(DetailAlert, a, b, c);
}

static void Goto(UInt16 scr) {
    gScreen = scr;
    if (scr == scrJogos && gSel < 0 && gHasData) {
        Int16 n = NextGame(TimGetSeconds());
        gSel = (n >= 0) ? n : (Int16)gNG - 1;
        gTop[scrJogos] = gSel - 3;
    }
    Draw();
}

static void Scroll(Int16 delta) {
    if (gScreen == scrHome || RowCount() <= gRows) return;
    gTop[gScreen] += delta;
    DrawListOnly();
}

static Boolean HandlePen(Coord x, Coord y) {
    UInt16 i;
    for (i = 0; i < gNHot; i++) {
        Hot *h = &gHot[i];
        if (x >= h->x && x < h->x + h->w && y >= h->y && y < h->y + h->h) {
            switch (h->action) {
                case actJogos:   Goto(scrJogos); break;
                case actTabela:  Goto(scrTabela); break;
                case actElenco:  Goto(scrElenco); break;
                case actTitulos: Goto(scrTitulos); break;
                case actHome:    Goto(scrHome); break;
                case actDetalhes: ShowDetails(gSel); break;
            }
            return true;
        }
    }
    if (gScreen == scrHome) return false;
    if (y >= 32 && y < 60) {                                  /* abas */
        Coord w = (gW - 8) / 4; Int16 t = (x - 4) / w;
        if (t >= 0 && t < 4 && gScreen != scrJogos + t) Goto(scrJogos + t);
        return true;
    }
    if (y >= gListY0 && y < gListY1 && gRows) {
        if (x >= gW - 16) { Scroll((y < (gListY0 + gListY1) / 2) ? -(Int16)(gRows - 1) : (Int16)(gRows - 1)); return true; }
        if (gScreen == scrJogos && gHasData) {
            Int16 r = gTop[scrJogos] + (y - gListY0) / gRowH;
            if (r < (Int16)gNG) {
                if (r == gSel) ShowDetails(r); else { gSel = r; DrawListOnly(); }
            }
        }
        return true;
    }
    return false;
}

/* ---------- PINS (área de Graffiti virtual / 320x480) ---------- */
static void ResizeForm(FormPtr frm) {
    Coord w, h; RectangleType r;
    WinGetDisplayExtent(&w, &h);               /* coordenadas padrão (160 x 160/240) */
    r.topLeft.x = 0; r.topLeft.y = 0; r.extent.x = w; r.extent.y = h;
    WinSetBounds(FrmGetWindowHandle(frm), &r);
    gW = w * 2; gH = h * 2;
}

static Boolean MainHandleEvent(EventPtr e) {
    FormPtr frm = FrmGetActiveForm();
    switch (e->eType) {
        case frmOpenEvent:
            if (gPINS) {
                WinSetConstraintsSize(FrmGetWindowHandle(frm), 160, 240, 240, 160, 160, 160);
                FrmSetDIAPolicyAttr(frm, frmDIAPolicyCustom);
                PINSetInputTriggerState(pinInputTriggerEnabled);
                PINSetInputAreaState(pinInputAreaClosed);
            }
            ResizeForm(frm);
            FrmDrawForm(frm);
            Draw();
            return true;
        case frmUpdateEvent:
            Draw();                                     /* sem FrmDrawForm: ele apagava a tela antes (desenho duplo) */
            return true;
        case winDisplayChangedEvent: {
            Coord ow = gW, oh = gH;
            ResizeForm(frm);
            if (gW != ow || gH != oh || !gOff) Draw();  /* só redesenha se o tamanho mudou de verdade */
            return true;
        }
        case frmCloseEvent:
            FreeOff();
            return false;                               /* deixa o FrmHandleEvent fechar o form */
        case penDownEvent:
            return HandlePen(e->screenX * 2, e->screenY * 2);
        case keyDownEvent:
            if (e->data.keyDown.chr == vchrPageUp)   { if (gScreen == scrHome) Goto(scrJogos); else Scroll(-(Int16)(gRows - 1)); return true; }
            if (e->data.keyDown.chr == vchrPageDown) { if (gScreen == scrHome) Goto(scrJogos); else Scroll((Int16)(gRows - 1)); return true; }
            return false;
        case menuEvent:
            switch (e->data.menu.itemID) {
                case MenuHome:    Goto(scrHome);    return true;
                case MenuJogos:   Goto(scrJogos);   return true;
                case MenuTabela:  Goto(scrTabela);  return true;
                case MenuElenco:  Goto(scrElenco);  return true;
                case MenuTitulos: Goto(scrTitulos); return true;
            }
            if (e->data.menu.itemID == MenuAbout) {
                Char b[24];
                if (gUpdated) { FmtDate(b, gUpdated, false, true); } else StrCopy(b, "sem dados");
                FrmCustomAlert(AboutAlert, b, "", "");
                return true;
            }
            return false;
        default:
            return false;
    }
}

static Boolean AppHandleEvent(EventPtr e) {
    if (e->eType == frmLoadEvent) {
        FormPtr frm = FrmInitForm(e->data.frmLoad.formID);
        FrmSetActiveForm(frm);
        FrmSetEventHandler(frm, MainHandleEvent);
        return true;
    }
    return false;
}

static Int32 Timeout(void) {
    /* só a tela inicial anima (relógio): acorda na virada do minuto; nas abas espera para sempre */
    if (gScreen != scrHome || FrmGetActiveFormID() != MainForm) return evtWaitForever;
    return (Int32)(60 - (TimGetSeconds() % 60)) * (Int32)SysTicksPerSecond() + 5;
}

static void EventLoop(void) {
    EventType e; Err err;
    do {
        EvtGetEvent(&e, Timeout());
        if (e.eType == nilEvent && FrmGetActiveFormID() == MainForm) Tick();
        if (SysHandleEvent(&e)) continue;
        if (MenuHandleEvent(0, &e, &err)) continue;
        if (AppHandleEvent(&e)) continue;
        FrmDispatchEvent(&e);
    } while (e.eType != appStopEvent);
}

static Boolean CheckHiRes(void) {
    UInt32 v = 0, dens = 0;
    if (FtrGet(sysFtrCreator, sysFtrNumWinVersion, &v) != errNone || v < 4) return false;
    if (WinScreenGetAttribute(winScreenDensity, &dens) != errNone) return false;
    return dens >= kDensityDouble;
}

UInt32 PilotMain(UInt16 cmd, MemPtr pbp, UInt16 flags) {
    UInt32 v;
    if (cmd != sysAppLaunchCmdNormalLaunch) return 0;
    if (!CheckHiRes()) { FrmAlert(NoHiResAlert); return 0; }
    gPINS = (FtrGet(pinCreator, pinFtrAPIVersion, &v) == errNone);
    LoadData();
    FrmGotoForm(MainForm);
    EventLoop();
    FrmCloseAllForms();
    FreeOff();
    FreeData();
    return 0;
}
