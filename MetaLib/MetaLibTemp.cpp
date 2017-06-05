#include "MetaLibTemp.h"


#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <locale>
#include <codecvt>
#include "MetaLibScopeGuard.h"
#include <limits>
#include "MetaLibClass_String.h"


using namespace MetaLib;


static inline void throwLastError()
{
    throw std::system_error(GetLastError(), std::system_category());
}

Obj const * MetaLib_readFile(USz file_path_length, U8 const * file_path_pointer, Obj const ** p_exception)
{
    try
    {
        auto file_path = reinterpret_cast<char const *>(file_path_pointer);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
        auto ws = conv.from_bytes(file_path, file_path + file_path_length);
        HANDLE f = CreateFileW(ws.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f == INVALID_HANDLE_VALUE) throwLastError();
        auto guard_f = makeScopeExitSuccess([f](bool success)
        {
            if ((0 == CloseHandle(f)) && success) throwLastError();
        });

        LARGE_INTEGER size;
        if (0 == GetFileSizeEx(f, &size)) throwLastError();
        if (size.QuadPart > std::numeric_limits<DWORD>::max()) throw 1;//TODO
        DWORD s = static_cast<DWORD>(size.QuadPart);

        std::basic_string<U8> r(s, '\0');
        DWORD read = 0;
        while (s > 0)
        {
            DWORD bytes_read = 0;
            if (0 == ReadFile(f, &r[read], s, &bytes_read, NULL)) throwLastError();
            if (bytes_read > s) throw 1;
            s -= bytes_read;
            read += bytes_read;
        }

        return UString(std::move(r)).release();
    }
    catch (...)//TODO
    {
        //TODO: make a real exception
        *p_exception = nullptr;
        return Null;
    }
}

Obj const * MetaLib_writeFile(USz file_path_length, U8 const * file_path_pointer, USz data_length, U8 const * data_pointer, Obj const ** p_exception)
{
    try
    {
        auto file_path = reinterpret_cast<char const *>(file_path_pointer);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
        auto ws = conv.from_bytes(file_path, file_path + file_path_length);
        HANDLE f = CreateFileW(ws.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f == INVALID_HANDLE_VALUE) throwLastError();
        auto guard_f_fail = makeScopeExitSuccess([&ws](bool success)
        {
            if ((0 == DeleteFileW(ws.c_str())) && success) throwLastError();
        });
        auto guard_f = makeScopeExitSuccess([f](bool success)
        {
            if ((0 == CloseHandle(f)) && success) throwLastError();
        });

        if (data_length > std::numeric_limits<DWORD>::max()) throw 1;//TODO
        DWORD size = static_cast<DWORD>(data_length);

        while (size > 0)
        {
            DWORD bytes_written = 0;
            if (0 == WriteFile(f, data_pointer, size, &bytes_written, NULL)) throwLastError();
            if (bytes_written > size) throw 1;
            size -= bytes_written;
            data_pointer += bytes_written;
        }

        guard_f_fail.release();

        return nullptr;
    }
    catch (...)//TODO
    {
        //TODO: make a real exception
        *p_exception = nullptr;
        return Null;
    }
}
