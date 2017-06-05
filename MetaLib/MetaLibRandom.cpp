#define _CRT_RAND_S

#include "MetaLibRandom.h"


#include "MetaLibPort.h"
using namespace MetaLib;
#ifdef _MSC_VER
#include <stdlib.h>
#endif

U8 MetaLib_generateCryptRandom(U64 * p)
{
    U64 r;
    //if (!(MetaLib_Intrin_CPURandom64(&r)))
#ifdef _MSC_VER
    unsigned int a, b;
    if (rand_s(&a) || rand_s(&b))
    {
        return 0;
    }
    r = (((U64)(a)) << 32) + b;
#else
#error "Not implemented on this compiler yet."
#endif
    *p = r;
    return 1;
}
