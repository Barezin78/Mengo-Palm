# Mengão p/ Palm T3 — viabilidade do build (checado 24/10/2026)
- Toolchain: prc-tools-remix 2.3.6 (m68k-palmos-gcc 2.95.3) + pilrc 3.2.91 — .deb oficiais p/ Debian trixie (github.com/jichu4n), INSTALADOS no box (/usr/bin).
- SDK: jichu4n/palm-os-sdk (sdk-5r3, sdk-5r4, PalmOne) clonado em /workspace/palm-mengao/sdk; tem PenInputMgr.h (DIA/hi-res+ 320x480) e PalmDisplayExtent.h.
- Teste: hello.c usando PenInputMgr + build-prc -> hello.prc OK (/workspace/palm-mengao/toolchain-test). Obs: não passar -palmos5; usar -I do sdk-5r3.
- Dados: sem internet no T3 -> PDB gerado por script Python (pip palmdb 1.7.2 ou struct manual), instalado via CloudpilotEmu (drag/import) ou HotSync/cartão SD.
- Riscos: 68k compilado roda via PACE (ok p/ UI simples); hi-res 320x320 via HD APIs (WinSetCoordinateSystem / bitmaps density 144); 320x480 exige PINS (PINSetInputAreaState, FrmSetDIAPolicyAttr, tratar winDisplayChangedEvent); fontes custom -> bitmaps hi-res (NFNT density 144) gerados a partir de TTF; CodeWarrior não roda em Linux (desnecessário).
