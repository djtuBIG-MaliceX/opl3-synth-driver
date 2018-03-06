
#define DISABLE_HW_SUPPORT 1
#define DISABLE_VGM_LOGGING 1
#include "../driver/src/OPLSynth.h"
typedef bool BOOLEAN;
#include "../driver/src/patch.h"
//#include "../driver/src/mauipatch.h"
//#include "../driver/src/fmsynthpatch.h"
//#include "../driver/src/2x2patchtest.h"
//#include "../driver/src/ctmidipatch.h"

std::vector<BYTE> BankToSyx(const patchStruct pats[], unsigned n)
{
    std::vector<BYTE> v;
    v.reserve(8192);

    for (unsigned insno = 0; insno < n; ++insno) {
        v.push_back(0xf0);
        v.push_back(0x7d);
        v.push_back(0x10);  // device id
        v.push_back(0x00);  // placeholder for model id
        v.push_back(0x02);  // send

        for (unsigned i = 0; i < 8; ++i)
            v.push_back("MaliceX "[i]);
        for (unsigned i = 0; i < 8; ++i)
            v.push_back("OP3PATCH"[i]);

        v.push_back(insno & 15);
        v.push_back(insno >> 4);

        unsigned patchlen = sizeof(patchStruct);
        v.push_back(patchlen & 15);
        v.push_back(patchlen >> 4);

        const BYTE *src = (const BYTE *)&pats[insno];
        for (unsigned i = 0; i < patchlen; ++i) {
            v.push_back(src[i] & 15);
            v.push_back(src[i] >> 4);
        }

        v.push_back(0x00);  // placeholder for checksum, TODO
        v.push_back(0xf7);
    }

    return v;
}

#include <unistd.h>

int main()
{
    std::vector<BYTE> v = BankToSyx(glpDefaultPatch, 256);

    if (isatty(1))
        return 1;
    write(1, v.data(), v.size());
    return 0;
}
