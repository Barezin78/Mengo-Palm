/* host preview driver: compiles the real app code (src/main.c) against a shim and dumps draw ops */
#include "main.c"
int main(int argc, char **argv) {
    /* usage: sim pdb|- height screen [sel_tap_y] > ops */
    gPdbPath = strcmp(argv[1], "-") ? argv[1] : NULL;
    gH = atoi(argv[2]); gScreen = atoi(argv[3]);
    shimNow = (UInt32)(time(NULL) - 3 * 3600) + P1904;
    if (argc > 4) shimNow = (UInt32)atol(argv[4]);
    gOps = stdout;
    LoadData();
    { UInt16 s = gScreen; gScreen = scrHome; if (s != scrHome) { gOps = fopen("/dev/null", "w"); Draw(); gOps = stdout; Goto(s); } else Draw(); }
    if (argc > 5) { gOps = fopen("/dev/null", "w"); Scroll(atoi(argv[5])); gOps = stdout; DrawListOnly(); }
    FreeData();
    return 0;
}
