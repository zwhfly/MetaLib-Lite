#ifndef MetaLibRandom_H_
#define MetaLibRandom_H_


#include "MetaLibPort.h"


MetaLib_EXTERN_C MetaLib_EXP(MetaLib_U8) MetaLib_generateCryptRandom(MetaLib_U64 * p);


static MetaLib_INLINE MetaLib_U64 MetaLib_xoroshiro128plus_rotl(MetaLib_U64 x, int k)
{
    return (x << k) | (x >> (64 - k));
}

struct MetaLib_xoroshiro128plus_Status
{
    MetaLib_U64 v[2];
};

static MetaLib_INLINE MetaLib_U64 MetaLib_xoroshiro128plus_next(MetaLib_xoroshiro128plus_Status * s)
{
    MetaLib_U64 v0 = s->v[0];
    MetaLib_U64 v1 = s->v[1];
    MetaLib_U64 result = v0 + v1;

    v1 ^= v0;
    s->v[0] = MetaLib_xoroshiro128plus_rotl(v0, 55) ^ v1 ^ (v1 << 14); // a, b
    s->v[1] = MetaLib_xoroshiro128plus_rotl(v1, 36); // c

    return result;
}


#ifdef __cplusplus

namespace MetaLib
{

;
class Xoroshiro128plus
{
    MetaLib_xoroshiro128plus_Status status;
public:
    Xoroshiro128plus(MetaLib_U64 init1, MetaLib_U64 init2)
    {
        status.v[0] = init1;
        status.v[1] = init2;
    }
    //Xoroshiro128plus()
    //{
    //    if (0 == MetaLib_generateCryptRandom(&(status.v[0])))
    //        throw 1;
    //    if (0 == MetaLib_generateCryptRandom(&(status.v[1])))
    //        throw 1;
    //}
    MetaLib_U64 next()
    {
        return MetaLib_xoroshiro128plus_next(&status);
    }
};

}

#endif


#endif
