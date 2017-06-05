#ifndef MetaLibClass_String_H_
#define MetaLibClass_String_H_


#include "MetaLibO.h"
#include "MetaLibPort.h"
#include <string>
#include <type_traits>


namespace MetaLibClass_String_Private
{


template <typename C>
class TStringClass : public ::MetaLib::Class_Type<std::basic_string<C>>::Type
{
private:
    struct GetLength
    {
        template <typename ObjType, typename StringType>
        MetaLib_USz operator()(ObjType &&, StringType && str) const
        {
            return str.length();
        }
    };
    struct Cast
    {
        template <typename ObjPtr, typename StringType>
        C const * operator()(ObjPtr, StringType && str) const
        {
            return &str[0];
        }
    };
public:
    TStringClass()
    {
        def<MetaLib_USz ()>("length", GetLength());
        addCastFunc<C []>(Cast());
    }
public:
    static TStringClass const Class;
};
template <typename C> TStringClass<C> const TStringClass<C>::Class;


}


namespace MetaLib
{
    static auto & UString = MetaLibClass_String_Private::TStringClass<U8>::Class;//TODO: inline
#if MetaLib_HAS_VARIABLE_TEMPLATE
    template <typename C>
    auto & TString = MetaLibClass_String_Private::TStringClass<C>::Class;
#endif

    MetaLib_INLINE auto makeOUString(StrRef str) -> decltype(UString(str.p<MetaLib_U8>(), str.len()))
    {
        return UString(str.p<MetaLib_U8>(), str.len());
    }

    //look out for object life time
    MetaLib_INLINE StrRef OUStringToStrRef(O const & o)
    {
        return StrRef(o.c("length").cast<MetaLib_USz>(), o.cast<MetaLib_U8 []>());
    }

    template <typename CTo, typename CFrom>
    MetaLib_INLINE typename std::enable_if<
        std::is_integral<CFrom>::value && std::is_integral<CTo>::value && sizeof(CFrom) == sizeof(CTo),
        std::basic_string<CTo>
    >::type
        aliasString(std::basic_string<CFrom> const & s)
    {
        return std::basic_string<CTo>(reinterpret_cast<CTo const *>(s.data()), s.length());
    }
}


#endif
