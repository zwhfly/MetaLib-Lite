#include "MetaLibModule.h"


#include <Windows.h>
#include <string>
#include <locale>
#include <codecvt>
#include <system_error>


using namespace MetaLib;


static inline void throwLastError()
{
    throw std::system_error(GetLastError(), std::system_category());
}

Obj const * MetaLib_import(MetaLib_USz module_name_length, MetaLib_U8 const * module_name_pointer, MetaLib_Obj const ** p_exception)
{
    try
    {
        auto module_name = reinterpret_cast<char const *>(module_name_pointer);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
        auto ws = conv.from_bytes(module_name, module_name + module_name_length);
        auto h_dll = LoadLibraryW(ws.c_str());
        if (h_dll == NULL) throwLastError();
        auto init = GetProcAddress(h_dll, u8"MetaLib_initModule");
        if (init == NULL) throwLastError();
        return reinterpret_cast<MetaLib_InitModule *>(init)(p_exception);
    }
    catch (...)//TODO
    {
        //TODO: make a real exception
        *p_exception = nullptr;
        return Null;
    }
}

