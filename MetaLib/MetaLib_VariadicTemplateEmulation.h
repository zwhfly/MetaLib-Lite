#ifndef MetaLib_VariadicTemplateEmulation_H_
#define MetaLib_VariadicTemplateEmulation_H_


#include "MetaLibPort.h"


namespace MetaLib
{


//#define MetaLib_VTE_callWithSelectedArgs(count, ...) \
//template <typename F,\
//    typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7,\
//    typename A8, typename A9, typename AA, typename AB, typename AC, typename AD, typename AE, typename AF\
//>\
//static MetaLib_INLINE auto callWithSelectedArgs(std::integral_constant<MetaLib_USz, count> tag_args_count, F && f,\
//    A0 && a0, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7,\
//    A8 && a8, A9 && a9, AA && aA, AB && aB, AC && aC, AD && aD, AE && aE, AF && aF)\
//    -> decltype(std::forward<F>(f)(__VA_ARGS__))\
//{\
//    (void)(tag_args_count);\
//    (void)(a0); (void)(a1); (void)(a2); (void)(a3); (void)(a4); (void)(a5); (void)(a6); (void)(a7);\
//    (void)(a8); (void)(a9); (void)(aA); (void)(aB); (void)(aC); (void)(aD); (void)(aE); (void)(aF);\
//    return std::forward<F>(f)(__VA_ARGS__);\
//}
//
//MetaLib_VTE_callWithSelectedArgs(0x00                                                                                                                                                                                                                                                                                                                                                                )
//MetaLib_VTE_callWithSelectedArgs(0x01, std::forward<A0>(a0)                                                                                                                                                                                                                                                                                                                                          )
//MetaLib_VTE_callWithSelectedArgs(0x02, std::forward<A0>(a0), std::forward<A1>(a1)                                                                                                                                                                                                                                                                                                                    )
//MetaLib_VTE_callWithSelectedArgs(0x03, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)                                                                                                                                                                                                                                                                                              )
//MetaLib_VTE_callWithSelectedArgs(0x04, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)                                                                                                                                                                                                                                                                        )
//MetaLib_VTE_callWithSelectedArgs(0x05, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)                                                                                                                                                                                                                                                  )
//MetaLib_VTE_callWithSelectedArgs(0x06, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5)                                                                                                                                                                                                                            )
//MetaLib_VTE_callWithSelectedArgs(0x07, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6)                                                                                                                                                                                                      )
//MetaLib_VTE_callWithSelectedArgs(0x08, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7)                                                                                                                                                                                )
//MetaLib_VTE_callWithSelectedArgs(0x09, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8)                                                                                                                                                          )
//MetaLib_VTE_callWithSelectedArgs(0x0A, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9)                                                                                                                                    )
//MetaLib_VTE_callWithSelectedArgs(0x0B, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA)                                                                                                              )
//MetaLib_VTE_callWithSelectedArgs(0x0C, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB)                                                                                        )
//MetaLib_VTE_callWithSelectedArgs(0x0D, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC)                                                                  )
//MetaLib_VTE_callWithSelectedArgs(0x0E, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC), std::forward<AD>(aD)                                            )
//MetaLib_VTE_callWithSelectedArgs(0x0F, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC), std::forward<AD>(aD), std::forward<AE>(aE)                      )
//MetaLib_VTE_callWithSelectedArgs(0x10, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC), std::forward<AD>(aD), std::forward<AE>(aE), std::forward<AF>(aF))

//#define MetaLib_VTE_PROLOG_callWithSelectedArgs(count) \
//template <typename F,\
//    typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7,\
//    typename A8, typename A9, typename AA, typename AB, typename AC, typename AD, typename AE, typename AF\
//>\
//static MetaLib_INLINE auto callWithSelectedArgs(std::integral_constant<MetaLib_USz, count> tag_args_count, F && f,\
//    A0 && a0, A1 && a1, A2 && a2, A3 && a3, A4 && a4, A5 && a5, A6 && a6, A7 && a7,\
//    A8 && a8, A9 && a9, AA && aA, AB && aB, AC && aC, AD && aD, AE && aE, AF && aF)\
//{\
//    (void)(tag_args_count);\
//    (void)(a0); (void)(a1); (void)(a2); (void)(a3); (void)(a4); (void)(a5); (void)(a6); (void)(a7);\
//    (void)(a8); (void)(a9); (void)(aA); (void)(aB); (void)(aC); (void)(aD); (void)(aE); (void)(aF);\
//    return std::forward<F>(f)(
////args
//#define MetaLib_VTE_EPILOG_callWithSelectedArgs(count) \
//);\
//}
//
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x00)                                                                                                                                                                                                                                                                                                                                                                MetaLib_VTE_EPILOG_callWithSelectedArgs(0x00)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x01) std::forward<A0>(a0)                                                                                                                                                                                                                                                                                                                                           MetaLib_VTE_EPILOG_callWithSelectedArgs(0x01)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x02) std::forward<A0>(a0), std::forward<A1>(a1)                                                                                                                                                                                                                                                                                                                     MetaLib_VTE_EPILOG_callWithSelectedArgs(0x02)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x03) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)                                                                                                                                                                                                                                                                                               MetaLib_VTE_EPILOG_callWithSelectedArgs(0x03)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x04) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)                                                                                                                                                                                                                                                                         MetaLib_VTE_EPILOG_callWithSelectedArgs(0x04)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x05) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)                                                                                                                                                                                                                                                   MetaLib_VTE_EPILOG_callWithSelectedArgs(0x05)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x06) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5)                                                                                                                                                                                                                             MetaLib_VTE_EPILOG_callWithSelectedArgs(0x06)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x07) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6)                                                                                                                                                                                                       MetaLib_VTE_EPILOG_callWithSelectedArgs(0x07)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x08) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7)                                                                                                                                                                                 MetaLib_VTE_EPILOG_callWithSelectedArgs(0x08)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x09) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8)                                                                                                                                                           MetaLib_VTE_EPILOG_callWithSelectedArgs(0x09)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x0A) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9)                                                                                                                                     MetaLib_VTE_EPILOG_callWithSelectedArgs(0x0A)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x0B) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA)                                                                                                               MetaLib_VTE_EPILOG_callWithSelectedArgs(0x0B)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x0C) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB)                                                                                         MetaLib_VTE_EPILOG_callWithSelectedArgs(0x0C)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x0D) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC)                                                                   MetaLib_VTE_EPILOG_callWithSelectedArgs(0x0D)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x0E) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC), std::forward<AD>(aD)                                             MetaLib_VTE_EPILOG_callWithSelectedArgs(0x0E)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x0F) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC), std::forward<AD>(aD), std::forward<AE>(aE)                       MetaLib_VTE_EPILOG_callWithSelectedArgs(0x0F)
//MetaLib_VTE_PROLOG_callWithSelectedArgs(0x10) std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8), std::forward<A9>(a9), std::forward<AA>(aA), std::forward<AB>(aB), std::forward<AC>(aC), std::forward<AD>(aD), std::forward<AE>(aE), std::forward<AF>(aF) MetaLib_VTE_EPILOG_callWithSelectedArgs(0x10)


struct Unused {};
template <
    typename A0 = Unused,
    typename A1 = Unused,
    typename A2 = Unused,
    typename A3 = Unused,
    typename A4 = Unused,
    typename A5 = Unused,
    typename A6 = Unused,
    typename A7 = Unused,
    typename A8 = Unused,
    typename A9 = Unused,
    typename AA = Unused,
    typename AB = Unused,
    typename AC = Unused,
    typename AD = Unused,
    typename AE = Unused,
    typename AF = Unused
>
struct UsedCount
{
    static MetaLib_CONSTEXPR MetaLib_USz const value =
        std::is_same<typename std::decay<A0>::type, Unused>::value ? 0x0 : (
        std::is_same<typename std::decay<A1>::type, Unused>::value ? 0x1 : (
        std::is_same<typename std::decay<A2>::type, Unused>::value ? 0x2 : (
        std::is_same<typename std::decay<A3>::type, Unused>::value ? 0x3 : (
        std::is_same<typename std::decay<A4>::type, Unused>::value ? 0x4 : (
        std::is_same<typename std::decay<A5>::type, Unused>::value ? 0x5 : (
        std::is_same<typename std::decay<A6>::type, Unused>::value ? 0x6 : (
        std::is_same<typename std::decay<A7>::type, Unused>::value ? 0x7 : (
        std::is_same<typename std::decay<A8>::type, Unused>::value ? 0x8 : (
        std::is_same<typename std::decay<A9>::type, Unused>::value ? 0x9 : (
        std::is_same<typename std::decay<AA>::type, Unused>::value ? 0xA : (
        std::is_same<typename std::decay<AB>::type, Unused>::value ? 0xB : (
        std::is_same<typename std::decay<AC>::type, Unused>::value ? 0xC : (
        std::is_same<typename std::decay<AD>::type, Unused>::value ? 0xD : (
        std::is_same<typename std::decay<AE>::type, Unused>::value ? 0xE : (
        std::is_same<typename std::decay<AF>::type, Unused>::value ? 0xF : (
        0x10))))))))))))))));
};

}


#endif
