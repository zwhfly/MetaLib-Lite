#ifndef MetaLibModule_H_
#define MetaLibModule_H_


#include "MetaLibObj.h"
#include "MetaLibPort.h"


typedef MetaLib_Obj const * MetaLib_InitModule(MetaLib_Obj const ** p_exception);


#ifdef MetaLib_MakeModule
#ifdef __cplusplus//C++ is needed to make a MetaLib module

#include "MetaLibO.h"

extern ::MetaLib::O initMetaLibModule();

namespace MetaLibModulePrivate
{

MetaLib_INLINE::MetaLib::O initModuleNoExport()
{
    static ::MetaLib::O const export_object = initMetaLibModule();
    return export_object;
}

}

MetaLib_EXTERN_C MetaLib_INLINE MetaLib_DL_EXP(MetaLib_Obj const *) MetaLib_initModule(MetaLib_Obj const ** p_exception)//DO NOT call directly
{
    try
    {
        MetaLib_InitModule * ensure_type = &MetaLib_initModule;
        (void)(ensure_type);

        return MetaLibModulePrivate::initModuleNoExport().release();
    }
    catch (...)//TODO
    {
        //TODO: make a real exception
        *p_exception = nullptr;
        return ::MetaLib::Null;
    }
}

#endif
#endif


MetaLib_EXTERN_C MetaLib_EXP(MetaLib_Obj const *) MetaLib_import(MetaLib_USz module_name_length, MetaLib_U8 const * module_name_pointer, MetaLib_Obj const ** p_exception);


#ifdef __cplusplus

#include "MetaLibO.h"
#include <string>

namespace MetaLibModulePrivate
{


MetaLib_INLINE::MetaLib::O import(::MetaLib::StrRef module_name)
{
    MetaLib_Obj const * exception;
    auto r = MetaLib_import(module_name.len(), module_name.p(), &exception);
    if (r == ::MetaLib::Null) throw ::MetaLib::O(exception);
    return ::MetaLib::O(r);
}

//MetaLib_INLINE ::MetaLib::O import(MetaLib_USz module_name_length, MetaLib_U8 const * module_name_pointer)
//{
//    MetaLib_Obj const * exception;
//    auto r = MetaLib_import(module_name_length, module_name_pointer, &exception);
//    if (r == ::MetaLib::Null) throw ::MetaLib::O(exception);
//    return ::MetaLib::O(r);
//}
//
//template <typename C, MetaLib_USz N>
//typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), ::MetaLib::O>::type
//import(C const (&module_name)[N])
//{
//    return import(N - 1, reinterpret_cast<MetaLib_U8 const *>(module_name));
//}
//
//template <typename O>
//typename std::enable_if<std::is_same<O, ::MetaLib::O>::value, ::MetaLib::O>::type
//import(O const & module_name)
//{
//    return import(module_name.c("length").cast<MetaLib_USz>(), module_name.cast<MetaLib_U8 []>());
//}
//
//template <typename C, typename T, typename A>
//typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), ::MetaLib::O>::type
//import(std::basic_string<C, T, A> const & module_name)
//{
//    return import(module_name.length(), reinterpret_cast<MetaLib_U8 const *>(&(module_name[0])));
//}
//
//#if (MetaLib_HAS_EXPRESSION_SFINAE || (defined(_MSC_VER) && (_MSC_FULL_VER >= 190024210)))/* Visual Studio 2015 Update 3 is fine here. */
//template <typename S>//type with length() and data()
//auto import(S const & module_name)
//-> typename std::enable_if<
//    std::is_integral<typename std::decay<decltype(*(module_name.data()))>::type>::value &&
//    (sizeof(decltype(*(module_name.data()))) == 1) &&
//    std::is_integral<decltype(module_name.length())>::value,
//    ::MetaLib::O>::type
//{
//    return import(module_name.length(), reinterpret_cast<MetaLib_U8 const *>(&(*(module_name.data()))));
//}
//#endif


}


namespace MetaLib
{
    using MetaLibModulePrivate::import;
}

#endif


#endif
