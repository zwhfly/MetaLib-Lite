#ifndef MetaLibTemp_H_
#define MetaLibTemp_H_


#include "MetaLibPort.h"
#include "MetaLibObj.h"


MetaLib_EXTERN_C MetaLib_EXP(MetaLib_Obj const *) MetaLib_readFile(MetaLib_USz file_path_length, MetaLib_U8 const * file_path_pointer, MetaLib_Obj const ** p_exception);
MetaLib_EXTERN_C MetaLib_EXP(MetaLib_Obj const *) MetaLib_writeFile(MetaLib_USz file_path_length, MetaLib_U8 const * file_path_pointer, MetaLib_USz data_length, MetaLib_U8 const * data_pointer, MetaLib_Obj const ** p_exception);


#ifdef __cplusplus


#include "MetaLibO.h"
#include <string>


namespace MetaLibTempPrivate
{


MetaLib_INLINE ::MetaLib::O readFile(::MetaLib::StrRef file_path)
{
    MetaLib_Obj const * exception;
    auto obj = MetaLib_readFile(file_path.len(), file_path.p(), &exception);
    if (obj == ::MetaLib::Null) throw ::MetaLib::O(exception);
    return ::MetaLib::O(obj);
}

MetaLib_INLINE void writeFile(::MetaLib::StrRef file_path, ::MetaLib::StrRef data)
{
    MetaLib_Obj const * exception;
    auto obj = MetaLib_writeFile(file_path.len(), file_path.p(), data.len(), data.p(), &exception);
    if (obj == ::MetaLib::Null) throw ::MetaLib::O(exception);
    ::MetaLib::O t(obj);
}

//MetaLib_INLINE ::MetaLib::O readFile(MetaLib_USz file_path_length, MetaLib_U8 const * file_path_pointer)
//{
//    MetaLib_Obj const * exception;
//    auto obj = MetaLib_readFile(file_path_length, file_path_pointer, &exception);
//    if (obj == ::MetaLib::Null) throw ::MetaLib::O(exception);
//    return ::MetaLib::O(obj);
//}
//
//template <typename C, MetaLib_USz N>
//typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), ::MetaLib::O>::type
//readFile(C const (&file_path)[N])
//{
//    return readFile(N - 1, reinterpret_cast<MetaLib_U8 const *>(file_path));
//}
//
//template <typename O>
//typename std::enable_if<std::is_same<O, ::MetaLib::O>::value, ::MetaLib::O>::type
//readFile(O const & file_path)
//{
//    return readFile(file_path.c("length").cast<MetaLib_USz>(), file_path.cast<MetaLib_U8 []>());
//}
//
//template <typename C, typename T, typename A>
//typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), ::MetaLib::O>::type
//readFile(std::basic_string<C, T, A> const & file_path)
//{
//    return readFile(file_path.length(), reinterpret_cast<MetaLib_U8 const *>(&(file_path[0])));
//}
//
//#if (MetaLib_HAS_EXPRESSION_SFINAE || (defined(_MSC_VER) && (_MSC_FULL_VER >= 190024210)))/* Visual Studio 2015 Update 3 is fine here. */
//template <typename S>//type with length() and data()
//auto readFile(S const & file_path)
//-> typename std::enable_if<
//    std::is_integral<typename std::decay<decltype(*(file_path.data()))>::type>::value &&
//    (sizeof(decltype(*(file_path.data()))) == 1) &&
//    std::is_integral<decltype(file_path.length())>::value,
//    ::MetaLib::O>::type
//{
//    return readFile(file_path.length(), reinterpret_cast<MetaLib_U8 const *>(&(*(file_path.data()))));
//}
//#endif


}


namespace MetaLib
{
    using MetaLibTempPrivate::readFile;
    using MetaLibTempPrivate::writeFile;
}


#endif


#endif
