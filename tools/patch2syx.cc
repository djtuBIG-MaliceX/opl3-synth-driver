
#define DISABLE_HW_SUPPORT 1
#define DISABLE_VGM_LOGGING 1
#include "../driver/src/OPLSynth.h"

#include "../driver/src/patch.h"
//#include "../driver/src/mauipatch.h"
//#include "../driver/src/fmsynthpatch.h"
//#include "../driver/src/2x2patchtest.h"
//#include "../driver/src/ctmidipatch.h"

#ifdef _WIN32
# include <winsock2.h>
#else
# include <arpa/inet.h>  // htons
#endif
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>

class Syx_Writer
{
public:
    Syx_Writer(std::vector<uint8_t> &buf)
        : buf(buf) {}

    size_t pos() const
        { return buf.size(); }

    void byte(unsigned b)
        { buf.push_back(b); }

    void byte2x4(unsigned b)
        { buf.push_back(b & 15);
          buf.push_back(b >> 4); }

    void short2x4(uint16_t s)
        { byte2x4(s & 255);
          byte2x4(s >> 8); }

    void string(const char *str, size_t len)
        { std::copy(str, str + len, std::back_inserter(buf)); }

    void checksum(size_t i1, size_t i2)
        { unsigned sum = 0;
          for (size_t i = i1; i < i2; ++i)
              sum = (sum + buf[i]) & 0x7f;
          byte((128 - sum) & 0x7f);
        }

private:
    std::vector<uint8_t> &buf;
};

void PatchToSyx(Syx_Writer &wr, unsigned insno, const patchStruct &pat)
{
    wr.byte(0xf0);
    wr.byte(0x7d);
    wr.byte(0x10);  // device id
    wr.byte(0x00);  // placeholder for model id
    wr.byte(0x02);  // send

    size_t cs_start = wr.pos();
    wr.string("MaliceX ", 8);
    wr.string("OP3PATCH", 8);

    wr.byte2x4(insno);

    unsigned patchlen = sizeof(patchStruct);
    wr.byte2x4(patchlen);
    for (unsigned i = 0; i < patchlen; ++i)
        wr.byte2x4(((const BYTE *)&pat)[i]);

    wr.checksum(cs_start, wr.pos());
    wr.byte(0xf7);
}

void MelMapToSyx(Syx_Writer &wr, unsigned insno, patchMapStruct map)
{
    wr.byte(0xf0);
    wr.byte(0x7d);
    wr.byte(0x10);  // device id
    wr.byte(0x00);  // placeholder for model id
    wr.byte(0x02);  // send

    size_t cs_start = wr.pos();
    wr.string("MaliceX ", 8);
    wr.string("OP3MLMAP", 8);

    wr.byte(insno);

    map.wBaseTranspose = htons(map.wBaseTranspose);
    map.wSecondTranspose = htons(map.wSecondTranspose);
    map.wPitchEGAmt = htons(map.wPitchEGAmt);
    map.wPitchEGTime = htons(map.wPitchEGTime);
    map.wBaseFineTune = htons(map.wBaseFineTune);
    map.wSecondFineTune = htons(map.wSecondFineTune);

    unsigned maplen = sizeof(patchMapStruct) - sizeof(patchMapStruct::bReservedPadding);
    wr.byte2x4(maplen);
    for (unsigned i = 0; i < maplen; ++i)
        wr.byte2x4(((const BYTE *)&map)[i]);

    wr.checksum(cs_start, wr.pos());
    wr.byte(0xf7);
}

void PercMapToSyx(Syx_Writer &wr, unsigned insno, percMapStruct map)
{
    wr.byte(0xf0);
    wr.byte(0x7d);
    wr.byte(0x10);  // device id
    wr.byte(0x00);  // placeholder for model id
    wr.byte(0x02);  // send

    size_t cs_start = wr.pos();
    wr.string("MaliceX ", 8);
    wr.string("OP3PCMAP", 8);

    wr.byte(insno - 128);

    wr.byte2x4(sizeof(map));
    for (unsigned i = 0; i < sizeof(map); ++i)
        wr.byte2x4(((const BYTE *)&map)[i]);

    wr.checksum(cs_start, wr.pos());
    wr.byte(0xf7);
}

void BankToSyx(Syx_Writer &wr, const patchStruct pats[], unsigned n)
{
    for (unsigned insno = 0; insno < n; ++insno) {
        PatchToSyx(wr, insno, pats[insno]);
        if (insno < 128)
            MelMapToSyx(wr, insno, gbDefaultMelMap[insno]);
        else
            PercMapToSyx(wr, insno, gbDefaultPercMap[insno - 128]);
    }
}

#include <unistd.h>

int main()
{
    std::vector<BYTE> v;
    v.reserve(64 * 1024);

    Syx_Writer writer(v);
    BankToSyx(writer, glpDefaultPatch, 256);

    if (isatty(1))
        return 1;
    write(1, v.data(), v.size());

    return 0;
}
