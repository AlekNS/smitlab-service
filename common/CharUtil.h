
#ifndef CHAR_UTIL_H
#define CHAR_UTIL_H


namespace common {


//============================================================================//
class CharUtil
{

public:

    enum CHARACTERS
    {
        NUL = 0,
        SOH = 1,
        STX = 2,
        ETX = 3,
        EOT = 4,
        ENQ = 5,
        ACK = 6,
        BEL = 7,
        BS  = 8,
        HT  = 9,
        LF  = 0xa,
        VT  = 0xb,
        FF  = 0xc,
        CR  = 0xd,
        SO  = 0xe,
        SI  = 0xf,

        DLE = 0x10,
        DC1 = 0x11,
        DC2 = 0x12,
        DC3 = 0x13,
        DC4 = 0x14,
        NACK= 0x15,
        SYN = 0x16,
        ETB = 0x17,
        CAN = 0x18,
        EM  = 0x19,
        SUB = 0x1a,
        ESC = 0x1b,
        FS  = 0x1c,
        GS  = 0x1d,
        RS  = 0x1e,
        US  = 0x1f,

        SP  = 0x20,
    };

};


}


#endif
