#ifndef MetaLibScopeGuard_H_
#define MetaLibScopeGuard_H_


#include "MetaLibPort.h"
#include <utility>


namespace MetaLibScopeGuardPrivate
{


template<typename E>
class ScopeExit
{
    E exit;
    bool valid;
public:
    explicit ScopeExit(E f) :
        exit(f),
        valid(true)
    {}
    ScopeExit(ScopeExit && rhs) ://TODO: MetaLib_NOEXCEPT
        exit(std::move(rhs.exit)),
        valid(rhs.valid)
    {
        rhs.release();
    }
    ~ScopeExit()//TODO: noexcept(noexcept(exit()))
    {
        if (valid) exit();
    }
    void release() { valid = false; }//TODO: MetaLib_NOEXCEPT
#if MetaLib_HAS_DEFAULTED_DELETED_FUNCTIONS
    ScopeExit(ScopeExit const &) = delete;
    ScopeExit & operator=(ScopeExit const &) = delete;
    ScopeExit & operator=(ScopeExit &&) = delete;
#else
private:
    ScopeExit(ScopeExit const &);
    ScopeExit & operator=(ScopeExit const &);
    ScopeExit & operator=(ScopeExit &&);
#endif
};

template<typename E>
auto makeScopeExit(E exit)//TODO: MetaLib_NOEXCEPT
#if (!MetaLib_HAS_RETURN_TYPE_DEDUCTION_FOR_NORMAL_FUNCTION)
    ->decltype(ScopeExit<E>(exit))
#endif
{
    return ScopeExit<E>(exit);
}

#if defined(_MSC_VER) && (_MSC_VER >= 1900)//TODO
template<typename E>
auto makeScopeExitSuccess(E exit)//TODO: MetaLib_NOEXCEPT
{
    return makeScopeExit([=, ec = std::uncaught_exceptions()]()
        noexcept(noexcept(exit(false)) && noexcept(exit(true)))
    {
        exit(!(ec < std::uncaught_exceptions()));
    });
}
#endif


}


namespace MetaLib
{
    using MetaLibScopeGuardPrivate::makeScopeExit;
#if defined(_MSC_VER) && (_MSC_VER >= 1900)//TODO
    using MetaLibScopeGuardPrivate::makeScopeExitSuccess;
#endif
}


#endif
