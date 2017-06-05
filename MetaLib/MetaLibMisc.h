#ifndef MetaLibMisc_H_
#define MetaLibMisc_H_


#ifdef __cplusplus


#include <string>
#include <utility>
#include "MetaLibPort.h"


namespace MetaLibMiscPrivate
{


template <typename String>
class StringSplitIter
{
    typedef String S;
    typedef typename S::size_type Sz;
    S const & input;
    S const & sep;
    Sz begin_pos;
    Sz end_pos;
public:
    StringSplitIter(S const & input, S const & sep) :
        input(input),
        sep(sep),
        begin_pos(0),
        end_pos(0)
    {
        if (sep.size() == 0) throw 1;
        while (begin_pos == (end_pos = input.find(sep, begin_pos))) begin_pos += sep.size();
        if (end_pos == S::npos) end_pos = input.size();
    }
    operator bool() const
    {
        return begin_pos < input.size();
    }
    StringSplitIter & operator++()
    {
        if (end_pos == input.size())
        {
            begin_pos = end_pos;
        }
        else
        {
            begin_pos = end_pos + sep.size();
            while (begin_pos == (end_pos = input.find(sep, begin_pos))) begin_pos += sep.size();
            if (end_pos == S::npos) end_pos = input.size();
        }
        return *this;
    }
    std::pair<Sz, Sz> getPos() const
    {
        return { begin_pos, end_pos };
    }
    S get() const
    {
        return input.substr(begin_pos, end_pos - begin_pos);
    }
};

template <typename String>
MetaLib_INLINE StringSplitIter<String> startStringSplit(String const & str, String const & sep)
{
    return StringSplitIter<String>(str, sep);
}


template <typename String>
MetaLib_INLINE void replaceAllString(String & str, String const & find, String const & replace)//TODO: this is not the most optimized algorithm
{
    for (typename String::size_type i = 0; String::npos != (i = str.find(find, i)); i += replace.length())
    {
        str.replace(i, find.length(), replace);
    }
}


}


namespace MetaLib
{
    using MetaLibMiscPrivate::startStringSplit;
    using MetaLibMiscPrivate::replaceAllString;
}


#endif


#endif
