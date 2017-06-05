#ifndef MetaLibO_H_
#define MetaLibO_H_


//TODO: overhaul this header


#include "MetaLibPort.h"
#include "MetaLibObj.h"
#include "MetaLibScopeGuard.h"
#include <cstddef>
#include <utility>
#include <array>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <map>
#include <string>
#include <tuple>
#include <initializer_list>
#include <functional>
#include <assert.h>

#if (MetaLib_HAS_VARIADIC_TEMPLATE == 0)
#include "MetaLib_VariadicTemplateEmulation.h"
#endif


namespace MetaLibOPrivate
{


class O;


struct StrRefLen
{
    template <MetaLib_USz N>
    MetaLib_CONSTEXPR static MetaLib_USz get(char const (&)[N])
    {
        return N - 1;
    }
};


template <typename T>
struct UnboundedArray
{
    MetaLib_CONSTEXPR static bool const IsUnboundedArray = false;
    typedef T ExtentRemoved;
};
template <typename T>
struct UnboundedArray<T []>
{
    MetaLib_CONSTEXPR static bool const IsUnboundedArray = true;
    typedef T ExtentRemoved;
};


template <typename Type>
class TypeStorage
{
#if MetaLib_HAS_UNRESTRICTED_UNION
    union { Type var; };
public:
    TypeStorage() {}
    ~TypeStorage() {}
#else
    typename std::aligned_storage<sizeof(Type), MetaLib_ALIGNOF(Type)>::type var;
public:
    TypeStorage() {}
#endif
#if MetaLib_HAS_DEFAULTED_DELETED_FUNCTIONS
public:
    TypeStorage(TypeStorage const &) = delete;
    TypeStorage(TypeStorage &&) = delete;
    TypeStorage & operator=(TypeStorage const &) = delete;
    TypeStorage & operator=(TypeStorage &&) = delete;
#else
private:
    TypeStorage(TypeStorage const &);
    TypeStorage(TypeStorage &&);
    TypeStorage & operator=(TypeStorage const &);
    TypeStorage & operator=(TypeStorage &&);
#endif
public:
    Type * get() { return reinterpret_cast<Type *>(&var); }
    Type const * get() const { return reinterpret_cast<Type const *>(&var); }
};

template <>
class TypeStorage<void>
{
public:
    void * get() { return reinterpret_cast<void *>(this); }
    void const * get() const { return reinterpret_cast<void const *>(this); }
};


template <typename Type> class Class_ObjOfType;

template <typename Type>
class ObjOfType : public MetaLib_Obj
{
    static_assert(std::is_same<Type, typename std::decay<Type>::type>::value, "It is not supported when Type is not a decayed type.");
    static_assert(!std::is_void<Type>::value, "It is not supported when Type is void.");

    TypeStorage<Type> storage;

    bool obj_method_table_initialized;
    typedef O SF(ObjOfType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv);
    typedef std::map<std::string, std::function<SF>> ObjMethodTable;
    TypeStorage<ObjMethodTable> obj_method_table;

public:
    Type const * get() const { return storage.get(); }
    Type * get() { return storage.get(); }

    ObjMethodTable const * getMethodTable() const
    {
        if (obj_method_table_initialized)
            return obj_method_table.get();
        else
            return nullptr;
    }

    ObjMethodTable & getInitMethodTable()
    {
        auto r = obj_method_table.get();
        if (!obj_method_table_initialized)
        {
            new (r) ObjMethodTable();
            obj_method_table_initialized = true;
        }
        return *r;
    }

    void destroyMethodTable()
    {
        if (obj_method_table_initialized)
        {
            auto r = obj_method_table.get();
            r->~ObjMethodTable();
            obj_method_table_initialized = false;
        }
    }

private:
    ObjOfType()
        :
        obj_method_table_initialized(false)
    {}
#if MetaLib_HAS_DEFAULTED_DELETED_FUNCTIONS
    ~ObjOfType() = default;
#else
    ~ObjOfType() {}
#endif
    friend class Class_ObjOfType<Type>;
};

template <typename Type>
class Class_ObjOfType : public MetaLib_Class
{
public:
    typedef ObjOfType<Type> ObjType;
protected:
    static void drop(MetaLib_Obj const * obj)
    {
        auto pself = static_cast<ObjType const *>(obj);
        delete pself;
    }
protected:
    ObjType * alloc() const
    {
        auto r = new ObjType;
        return r;
    }
protected:
    static MetaLib_Obj const * call(
        MetaLib_Obj const *,
        MetaLib_USz, MetaLib_U8 const *,
        MetaLib_USz, MetaLib_Obj const **,
        MetaLib_Obj const ** p_exception)
    {
        //TODO: make a real exception
        *p_exception = nullptr;
        return ::MetaLib::Null;
    }
    static void const * cast(
        MetaLib_Obj const *,
        MetaLib_USz, MetaLib_U8 const *)
    {
        return nullptr;
    }
protected:
    static void kill(MetaLib_Obj const *)
    {
        return;
    }
public:
    Class_ObjOfType()
    {
        p_call = &call;
        p_cast = &cast;
        p_kill = &kill;
        p_drop = &drop;
    }
public:
    static Class_ObjOfType const Class;
};
template <typename Type> Class_ObjOfType<Type> const Class_ObjOfType<Type>::Class;

template <typename Type>
class Class_ObjOfType_AutoLife : public Class_ObjOfType<Type>
{
    typedef Class_ObjOfType<Type> Base;
public:
    typedef O SO;//object with specific class information
protected:
    static void kill(MetaLib_Obj const * obj)//TODO: exception analysis
    {
        auto & self = *const_cast<ObjType *>(static_cast<ObjType const *>(obj));
        try
        {
            self.destroyMethodTable();
            self.get()->~Type();
        }
        catch (...)
        {
            MetaLib_Intrin_BreakIfDebug();
            std::terminate();
        }
    }
public:
#if MetaLib_HAS_VARIADIC_TEMPLATE
    template <typename ... A>
    MetaLib_Obj const * create(A && ... a) const
    {
        auto pself = alloc();
        auto guard = ::MetaLib::makeScopeExit([pself] { drop(pself); });
        new (pself->get()) Type(std::forward<A>(a) ...);
        guard.release();
        MetaLib_Obj_init(pself, this);
        return pself;
    }
#else
    //TODO
    MetaLib_Obj const * create() const {
        auto pself = alloc(); auto guard = ::MetaLib::makeScopeExit([pself] { drop(pself); });
#ifdef _MSC_VER
        //http://stackoverflow.com/questions/8697105/is-the-c4345-warning-of-visual-studio-wrong
#pragma warning(suppress: 4345)
#endif
        new (pself->get()) Type();
        guard.release(); MetaLib_Obj_init(pself, this); return pself; }
    template <typename A0>
    MetaLib_Obj const * create(A0 && a0) const {
        auto pself = alloc(); auto guard = ::MetaLib::makeScopeExit([pself] { drop(pself); });
        new (pself->get()) Type(std::forward<A0>(a0));
        guard.release(); MetaLib_Obj_init(pself, this); return pself; }
    template <typename A0, typename A1>
    MetaLib_Obj const * create(A0 && a0, A1 && a1) const {
        auto pself = alloc(); auto guard = ::MetaLib::makeScopeExit([pself] { drop(pself); });
        new (pself->get()) Type(std::forward<A0>(a0), std::forward<A1>(a1));
        guard.release(); MetaLib_Obj_init(pself, this); return pself; }
    template <typename A0, typename A1, typename A2>
    MetaLib_Obj const * create(A0 && a0, A1 && a1, A2 && a2) const {
        auto pself = alloc(); auto guard = ::MetaLib::makeScopeExit([pself] { drop(pself); });
        new (pself->get()) Type(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2));
        guard.release(); MetaLib_Obj_init(pself, this); return pself; }
    template <typename A0, typename A1, typename A2, typename A3>
    MetaLib_Obj const * create(A0 && a0, A1 && a1, A2 && a2, A3 && a3) const {
        auto pself = alloc(); auto guard = ::MetaLib::makeScopeExit([pself] { drop(pself); });
        new (pself->get()) Type(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
        guard.release(); MetaLib_Obj_init(pself, this); return pself; }
#endif
#if MetaLib_HAS_VARIADIC_TEMPLATE
    template <typename ... A> SO operator()(A && ... a) const;
#else
    //TODO
    SO operator()() const;
    template <typename A0>
    SO operator()(A0 && a0) const;
    template <typename A0, typename A1>
    SO operator()(A0 && a0, A1 && a1) const;
    template <typename A0, typename A1, typename A2>
    SO operator()(A0 && a0, A1 && a1, A2 && a2) const;
    template <typename A0, typename A1, typename A2, typename A3>
    SO operator()(A0 && a0, A1 && a1, A2 && a2, A3 && a3) const;
#endif
public:
    Type * retrieve(MetaLib_Obj const * obj) const
    {
        if (obj == nullptr) return nullptr;//TODO
        if (obj->class_ != this) return nullptr;
        return const_cast<ObjType *>(static_cast<ObjType const *>(obj))->get();
    }
    static Type * castToType(MetaLib_Obj const * obj)
    {
        return const_cast<ObjType *>(static_cast<ObjType const *>(obj))->get();
    }
    static ObjType * castToObjType(MetaLib_Obj const * obj)
    {
        return const_cast<ObjType *>(static_cast<ObjType const *>(obj));
    }
public:
    Class_ObjOfType_AutoLife()
        :
        Base()
    {
        p_kill = &kill;
    }
public:
    static Class_ObjOfType_AutoLife const Class;
};
template <typename Type> Class_ObjOfType_AutoLife<Type> const Class_ObjOfType_AutoLife<Type>::Class;

template <typename Type>
class Class_ObjOfType_AutoLife_CastToSelf : public Class_ObjOfType_AutoLife<Type>
{
    typedef Class_ObjOfType_AutoLife<Type> Base;
protected:
    static void const * castImpl(
        std::true_type tag_is_ABI, MetaLib_Obj const * obj,
        MetaLib_USz type_name_length, MetaLib_U8 const * type_name_pointer)
    {
        (void)(tag_is_ABI);
        auto & self = *static_cast<ObjType const *>(obj);
        auto type_name = ::MetaLib::ABIType<Type>::getName();
        if ((type_name.len() == type_name_length) && std::equal(type_name.p(), type_name.p() + type_name.len(), type_name_pointer))
        {
            return static_cast<void const *>(self.get());
        }
        else
        {
            return nullptr;
        }
    }
    static void const * castImpl(
        std::false_type tag_is_ABI, MetaLib_Obj const *,
        MetaLib_USz, MetaLib_U8 const *)
    {
        (void)(tag_is_ABI);
        return nullptr;
    }
    static void const * cast(
        MetaLib_Obj const * obj,
        MetaLib_USz type_name_length, MetaLib_U8 const * type_name_pointer)
    {
        return castImpl(std::integral_constant<bool, ::MetaLib::ABIType<Type>::Is_ABI>(), obj, type_name_length, type_name_pointer);
    }
public:
    Class_ObjOfType_AutoLife_CastToSelf()
        :
        Base()
    {
        p_cast = &cast;
    }
public:
    static Class_ObjOfType_AutoLife_CastToSelf const Class;
};
template <typename Type> Class_ObjOfType_AutoLife_CastToSelf<Type> const Class_ObjOfType_AutoLife_CastToSelf<Type>::Class;

template <typename Type>
class Class_ObjOfType_AutoLife_CastToSelf_CallToSelf : public Class_ObjOfType_AutoLife_CastToSelf<Type>
{
    typedef Class_ObjOfType_AutoLife_CastToSelf<Type> Base;
protected:
    static MetaLib_Obj const * call(
        MetaLib_Obj const * obj,
        MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer,
        MetaLib_USz argc, MetaLib_Obj const ** argv,
        MetaLib_Obj const ** p_exception)
    {
        try
        {
            if (method_name_length == 0)
            {
                auto & self = *static_cast<ObjType const *>(obj);
                return (*(self.get()))(argc, argv);
            }
            else
            {
                return Base::call(obj, method_name_length, method_name_pointer, argc, argv, p_exception);
            }
        }
        catch (...)//TODO
        {
            //TODO: make a real exception
            *p_exception = nullptr;
            return ::MetaLib::Null;
        }
    }
public:
    Class_ObjOfType_AutoLife_CastToSelf_CallToSelf()
        :
        Base()
    {
        p_call = &call;
    }
public:
    static Class_ObjOfType_AutoLife_CastToSelf_CallToSelf const Class;
};
template <typename Type> Class_ObjOfType_AutoLife_CastToSelf_CallToSelf<Type> const Class_ObjOfType_AutoLife_CastToSelf_CallToSelf<Type>::Class;




#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename ... Types>
class ObjOfTypeList : public MetaLib_Obj
{
    template <typename T>
    struct ProperType
    {
        static MetaLib_CONSTEXPR bool const value = std::is_same<T, typename std::decay<T>::type>::value;
    };
    template <typename T>
    struct ProperType<T []>
    {
        static MetaLib_CONSTEXPR bool const value = ProperType<T>::value;
    };
    template <>
    struct ProperType<void>
    {
        static MetaLib_CONSTEXPR bool const value = false;
    };
    template <typename T, typename ... TS>
    struct AllProper
    {
        static MetaLib_CONSTEXPR bool const value = ProperType<T>::value && AllProper<TS ...>::value;
    };
    template <typename T>
    struct AllProper<T>
    {
        static MetaLib_CONSTEXPR bool const value = ProperType<T>::value;
    };
    static_assert(AllProper<Types ...>::value, "Types shall be either T or T [] (T is any decayed type except void).");

    //TODO
    //...
    //For any T[], a length and an end position are stored.
    //All offsets to the last T[] are computed in compile time.
};
#endif




class O
{
    MetaLib_Obj const * obj;
    friend class WO;
    struct PlaceHolder {};
public:
    template <typename V>
    O(V && v,
        typename std::enable_if<
        (!(std::is_base_of<O, typename std::decay<V>::type>::value))
        &&
        (!(std::is_convertible<typename std::decay<V>::type, MetaLib_Obj const *>::value))
        , PlaceHolder>::type = PlaceHolder()
    )
        :
        obj(Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<V>::type>::Class.create(std::forward<V>(v)))
    {}

    O() : obj(nullptr) {}//None
    O(std::nullptr_t) : obj(nullptr) {}//None
    O(O const & rhs) : obj(rhs.obj)
    {
        if (obj) if (auto err = MetaLib_Obj_increfContentBy1(obj)) throw err;
    }
    O(O && rhs) : obj(rhs.obj)
    {
        rhs.obj = nullptr;
    }
    O(MetaLib_Obj const * obj) : obj(obj)
    {}
    O & operator=(O const & rhs)
    {
        return ((*this) = O(rhs));//TODO: log on double exception (std::terminate is called)
    }
    O & operator=(O && rhs)
    {
        assert(this != &rhs);
        clear();
        obj = rhs.release();
        return *this;
    }
    ~O()//TODO: log on exception (std::terminate is called)
    {
        clear();
    }

    friend void swap(O & a, O & b)
    {
        std::swap(a.obj, b.obj);
    }
    void clear()
    {
        if (obj)
        {
            //TODO: handling exception of dtor?
            if (auto err = MetaLib_Obj_decrefContentBy1(obj)) throw err;
            obj = nullptr;
        }
    }
    MetaLib_CPP_EXPLICIT_CONVERSION operator bool() const
    {
        return (nullptr != obj);
    }

    MetaLib_Obj const * get() const
    {
        return obj;
    }
    MetaLib_Obj const * release()
    {
        auto r = obj;
        obj = nullptr;
        return r;
    }
    bool is(MetaLib_Obj const * robj) const
    {
        return obj == robj;
    }
    bool is(O const & rhs) const
    {
        return obj == rhs.obj;
    }
    ::MetaLib::UPtr id() const
    {
        return reinterpret_cast<::MetaLib::UPtr>(static_cast<void const *>(obj));
    }

public:
    MetaLib_USz getUseCount() const
    {
        if (obj == nullptr) return 2;//TODO: magic number
        return MetaLib_Obj_getrefContent(obj);
    }
    bool isUnique() const
    {
        return (getUseCount() == 1);
    }
public:
    //may return nullptr
    template <typename T>
    typename UnboundedArray<T>::ExtentRemoved const *
        castp() const
    {
        if (!obj) return nullptr;
        auto type_name = ::MetaLib::ABIType<T>::getName();
        auto p = obj->class_->p_cast(obj, type_name.len(), type_name.p());
        return static_cast<typename UnboundedArray<T>::ExtentRemoved const *>(p);
    }
    template <typename T>
    typename std::enable_if<std::is_same<T, O>::value, O>::type
        cast() const
#if MetaLib_HAS_REF_QUALIFIERS
        &
#endif
    {
        return O(*this);
    }
#if MetaLib_HAS_REF_QUALIFIERS
    template <typename T>
    typename std::enable_if<std::is_same<T, O>::value, O>::type
        cast() &&
    {
        return O(std::move(*this));
    }
#endif
    template <typename T>
    typename std::enable_if<(!std::is_same<typename std::decay<T>::type, O>::value) && (!UnboundedArray<T>::IsUnboundedArray), T const &>::type
        cast() const
#if MetaLib_HAS_REF_QUALIFIERS
        &
#endif
    {
        auto p = castp<T>();
        if (p == nullptr) throw 1;//TODO
        return *p;
    }
    template <typename T>
    typename std::enable_if<UnboundedArray<T>::IsUnboundedArray, typename UnboundedArray<T>::ExtentRemoved const *>::type
        cast() const
#if MetaLib_HAS_REF_QUALIFIERS
        &
#endif
    {
        auto p = castp<T>();
        if (p == nullptr) throw 1;//TODO
        return p;
    }

private:
    //O call(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, MetaLib_USz argc, MetaLib_Obj const ** argv) const
    //{
    //    if (!obj) throw 1;//TODO
    //    MetaLib_Obj const * exception;
    //    auto r = obj->class_->p_call(obj, method_name_length, method_name_pointer, argc, argv, &exception);
    //    if (r == ::MetaLib::Null) throw O(exception);
    //    return O(r);
    //}
    O call(::MetaLib::StrRef method_name, MetaLib_USz argc, MetaLib_Obj const ** argv) const
    {
        if (!obj) throw 1;//TODO
        MetaLib_Obj const * exception;
        auto r = obj->class_->p_call(obj, method_name.len(), method_name.p(), argc, argv, &exception);
        if (r == ::MetaLib::Null) throw O(exception);
        return O(r);
    }
    //template <typename C, MetaLib_USz N>
    //typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
    //    call(C const (&method_name)[N], MetaLib_USz argc, MetaLib_Obj const ** argv) const
    //{
    //    return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name), argc, argv);
    //}
public:
    O call(::MetaLib::StrRef method_name, MetaLib_USz argc, O * argv) const
    {
        if (!obj) throw 1;//TODO
#define MetaLib_O_call_MAX_ARGC 16
        if (argc > MetaLib_O_call_MAX_ARGC) throw 1;//TODO
        MetaLib_Obj const * aa[MetaLib_O_call_MAX_ARGC] = { 0 };
        auto guard_aa = ::MetaLib::makeScopeExit([&aa, argc]
        {
            for (MetaLib_USz i = 0; i < argc; ++i) O t(aa[i]);
        });
        for (MetaLib_USz i = 0; i < argc; ++i)
            aa[i] = argv[i].release();
        return call(method_name, argc, aa);
    }
#if MetaLib_HAS_VARIADIC_TEMPLATE
private:
    template <typename T = std::initializer_list<int>> static void Helper_doNothing(T) {}
    template <typename ... A, size_t ... I>
    O cImpl(::MetaLib::StrRef method_name, std::index_sequence<I ...>, A && ... a) const
    {
        std::array<MetaLib_Obj const *, sizeof...(A)> aa = { (std::is_void<A>::value, nullptr) ... };
        auto guard_aa = ::MetaLib::makeScopeExit([&aa]
        {
            for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i);
        });
        Helper_doNothing({ (aa[I] = O(std::forward<A>(a)).release(), 0) ... });
        return call(method_name, aa.size(), aa.data());
    }
public:
    template <typename ... A>
    O c(::MetaLib::StrRef method_name, A && ... a) const
    {
        return cImpl(method_name, std::index_sequence_for<A ...>(), std::forward<A>(a) ...);
    }
//private:
//    template <typename T = std::initializer_list<int>> static void Helper_doNothing(T) {}
//    template <typename ... A, size_t ... I>
//    O cImpl(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, std::index_sequence<I ...>, A && ... a) const
//    {
//        std::array<MetaLib_Obj const *, sizeof...(A)> aa = { (std::is_void<A>::value, nullptr) ... };
//        auto guard_aa = ::MetaLib::makeScopeExit([&aa]
//        {
//            for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i);
//        });
//        Helper_doNothing({ (aa[I] = O(std::forward<A>(a)).release(), 0) ... });
//        return call(method_name_length, method_name_pointer, aa.size(), aa.data());
//    }
//public:
//    template <typename ... A>
//    O c(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, A && ... a) const
//    {
//        return cImpl(method_name_length, method_name_pointer, std::index_sequence_for<A ...>(), std::forward<A>(a) ...);
//    }
#else
public:
    //TODO
    O c(::MetaLib::StrRef method_name) const {
        std::array<MetaLib_Obj const *, 0> aa = {};
        return call(method_name, aa.size(), aa.data()); }
    template <typename A0>
    O c(::MetaLib::StrRef method_name, A0 && a0) const {
        std::array<MetaLib_Obj const *, 1> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
        aa[0] = O(std::forward<A0>(a0)).release();
        return call(method_name, aa.size(), aa.data()); }
    template <typename A0, typename A1>
    O c(::MetaLib::StrRef method_name, A0 && a0, A1 && a1) const {
        std::array<MetaLib_Obj const *, 2> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
        aa[0] = O(std::forward<A0>(a0)).release(); aa[1] = O(std::forward<A1>(a1)).release();
        return call(method_name, aa.size(), aa.data()); }
    template <typename A0, typename A1, typename A2>
    O c(::MetaLib::StrRef method_name, A0 && a0, A1 && a1, A2 && a2) const {
        std::array<MetaLib_Obj const *, 3> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
        aa[0] = O(std::forward<A0>(a0)).release(); aa[1] = O(std::forward<A1>(a1)).release(); aa[2] = O(std::forward<A2>(a2)).release();
        return call(method_name, aa.size(), aa.data()); }
    template <typename A0, typename A1, typename A2, typename  A3>
    O c(::MetaLib::StrRef method_name, A0 && a0, A1 && a1, A2 && a2, A3 && a3) const {
        std::array<MetaLib_Obj const *, 4> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
        aa[0] = O(std::forward<A0>(a0)).release(); aa[1] = O(std::forward<A1>(a1)).release(); aa[2] = O(std::forward<A2>(a2)).release(); aa[3] = O(std::forward<A3>(a3)).release();
        return call(method_name, aa.size(), aa.data()); }
    //O c(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer) const {
    //    std::array<MetaLib_Obj const *, 0> aa = {};
    //    return call(method_name_length, method_name_pointer, aa.size(), aa.data()); }
    //template <typename A0>
    //O c(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, A0 && a0) const {
    //    std::array<MetaLib_Obj const *, 1> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
    //    aa[0] = O(std::forward<A0>(a0)).release();
    //    return call(method_name_length, method_name_pointer, aa.size(), aa.data()); }
    //template <typename A0, typename A1>
    //O c(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, A0 && a0, A1 && a1) const {
    //    std::array<MetaLib_Obj const *, 2> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
    //    aa[0] = O(std::forward<A0>(a0)).release(); aa[1] = O(std::forward<A1>(a1)).release();
    //    return call(method_name_length, method_name_pointer, aa.size(), aa.data()); }
    //template <typename A0, typename A1, typename A2>
    //O c(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, A0 && a0, A1 && a1, A2 && a2) const {
    //    std::array<MetaLib_Obj const *, 3> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
    //    aa[0] = O(std::forward<A0>(a0)).release(); aa[1] = O(std::forward<A1>(a1)).release(); aa[2] = O(std::forward<A2>(a2)).release();
    //    return call(method_name_length, method_name_pointer, aa.size(), aa.data()); }
    //template <typename A0, typename A1, typename A2, typename  A3>
    //O c(MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer, A0 && a0, A1 && a1, A2 && a2, A3 && a3) const {
    //    std::array<MetaLib_Obj const *, 4> aa = {}; auto guard_aa = ::MetaLib::makeScopeExit([&aa]() { for (auto i = aa.begin(); i != aa.end(); ++i) O t(*i); });
    //    aa[0] = O(std::forward<A0>(a0)).release(); aa[1] = O(std::forward<A1>(a1)).release(); aa[2] = O(std::forward<A2>(a2)).release(); aa[3] = O(std::forward<A3>(a3)).release();
    //    return call(method_name_length, method_name_pointer, aa.size(), aa.data()); }
#endif
//#if MetaLib_HAS_VARIADIC_TEMPLATE
//public:
//    template <typename C, MetaLib_USz N, typename ... A>
//    typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
//        c(C const (&method_name)[N], A && ... a) const
//    {
//        return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name), std::forward<A>(a) ...);
//    }
//#else
//public:
//    //TODO
//    template <typename C, MetaLib_USz N> typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
//        c(C const (&method_name)[N]) const
//    { return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name)); }
//    template <typename C, MetaLib_USz N, typename A0> typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
//        c(C const (&method_name)[N], A0 && a0) const
//    { return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name), std::forward<A0>(a0)); }
//    template <typename C, MetaLib_USz N, typename A0, typename A1> typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
//        c(C const (&method_name)[N], A0 && a0, A1 && a1) const
//    { return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name), std::forward<A0>(a0), std::forward<A1>(a1)); }
//    template <typename C, MetaLib_USz N, typename A0, typename A1, typename A2> typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
//        c(C const (&method_name)[N], A0 && a0, A1 && a1, A2 && a2) const
//    { return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name), std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)); }
//    template <typename C, MetaLib_USz N, typename A0, typename A1, typename A2, typename A3> typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), O>::type
//        c(C const (&method_name)[N], A0 && a0, A1 && a1, A2 && a2, A3 && a3) const
//    { return c(N - 1, reinterpret_cast<MetaLib_U8 const *>(method_name), std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)); }
//#endif
#if MetaLib_HAS_VARIADIC_TEMPLATE
public:
    template <typename ... A>
    O operator()(A && ... a) const
    {
        return c(::MetaLib::StrRef(0, nullptr), std::forward<A>(a) ...);
    }
#else
public:
    //TODO
    O operator()() const
    { return c(::MetaLib::StrRef(0, nullptr)); }
    template <typename A0>
    O operator()(A0 && a0) const
    { return c(::MetaLib::StrRef(0, nullptr), std::forward<A0>(a0)); }
    template <typename A0, typename A1>
    O operator()(A0 && a0, A1 && a1) const
    { return c(::MetaLib::StrRef(0, nullptr), std::forward<A0>(a0), std::forward<A1>(a1)); }
    template <typename A0, typename A1, typename A2>
    O operator()(A0 && a0, A1 && a1, A2 && a2) const
    { return c(::MetaLib::StrRef(0, nullptr), std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)); }
    template <typename A0, typename A1, typename A2, typename A3>
    O operator()(A0 && a0, A1 && a1, A2 && a2, A3 && a3) const
    { return c(::MetaLib::StrRef(0, nullptr), std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)); }
#endif
};


static MetaLib_INLINE O acquireObj(MetaLib_Obj const * obj)//acquire MetaLib_Obj as O
{
    if (obj == nullptr) return O();
    if (auto err = MetaLib_Obj_increfContentBy1(obj)) throw err;
    return O(obj);
}


class WO//weak reference of O
{
    //Do not expose the type MetaLib_Obj.
    //Because the internal implementation of WO is subject to change in the future.
    //e.g., using standalone control block, instead of Obj pointer.

    MetaLib_Obj const * obj;
public:
    WO() : obj(nullptr) {}//None
    WO(std::nullptr_t) : obj(nullptr) {}//None
    WO(WO const & rhs) : obj(rhs.obj)
    {
        if (obj) if (auto err = MetaLib_Obj_increfMemoryBy1(obj)) throw err;
    }
    WO(WO && rhs) : obj(rhs.obj)
    {
        rhs.obj = nullptr;
    }
    WO & operator=(WO const & rhs)
    {
        return ((*this) = WO(rhs));//TODO: log on double exception (std::terminate is called)
    }
    WO & operator=(WO && rhs)
    {
        assert(this != &rhs);
        clear();
        obj = rhs.obj;
        rhs.obj = nullptr;
        return *this;
    }
    ~WO()//TODO: log on exception (std::terminate is called)
    {
        clear();
    }

    WO(O const & o) : obj(o.obj)
    {
        if (obj) if (auto err = MetaLib_Obj_increfMemoryBy1(obj)) throw err;
    }
    WO & operator=(O const & rhs)
    {
        return ((*this) = WO(rhs));//TODO: log on double exception (std::terminate is called)
    }
    bool valid() const
    {
        if (nullptr == obj) return true;//TODO: 'None' is always valid?
        return (MetaLib_Obj_getrefContent(obj) > 0);
    }
    O lock() const
    {
        if (nullptr == obj) return O();
        MetaLib_U8 not_0;
        if (auto err = MetaLib_Obj_increfContentNot0By1(obj, &not_0)) throw err;
        return not_0 ? O(obj) : O();
    }

    friend void swap(WO & a, WO & b)
    {
        std::swap(a.obj, b.obj);
    }
    void clear()
    {
        if (obj)
        {
            if (auto err = MetaLib_Obj_decrefMemoryBy1(obj)) throw err;
            obj = nullptr;
        }
    }

    bool is(O const & rhs) const
    {
        return obj == rhs.obj;
    }
    bool is(WO const & rhs) const
    {
        return obj == rhs.obj;
    }
};

//TODO:
//compares between:
// O objects;
// WO objects;
// O object and WO object,
//of:
// equality;
// order.

struct lessOid
{
    bool operator()(O const & lhs, O const & rhs) const
    {
        return (lhs.id()) < (rhs.id());
    }
};




#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename Type>
template <typename ... A>
MetaLib_INLINE typename Class_ObjOfType_AutoLife<Type>::SO Class_ObjOfType_AutoLife<Type>::operator()(A && ... a) const
{
    return SO(create(std::forward<A>(a) ...));
}
#else
//TODO
template <typename Type>
MetaLib_INLINE typename Class_ObjOfType_AutoLife<Type>::SO Class_ObjOfType_AutoLife<Type>::operator()() const {
    return SO(create()); }
template <typename Type> template <typename A0>
MetaLib_INLINE typename Class_ObjOfType_AutoLife<Type>::SO Class_ObjOfType_AutoLife<Type>::operator()(A0 && a0) const {
    return SO(create(std::forward<A0>(a0))); }
template <typename Type> template <typename A0, typename A1>
MetaLib_INLINE typename Class_ObjOfType_AutoLife<Type>::SO Class_ObjOfType_AutoLife<Type>::operator()(A0 && a0, A1 && a1) const {
    return SO(create(std::forward<A0>(a0), std::forward<A1>(a1))); }
template <typename Type> template <typename A0, typename A1, typename A2>
MetaLib_INLINE typename Class_ObjOfType_AutoLife<Type>::SO Class_ObjOfType_AutoLife<Type>::operator()(A0 && a0, A1 && a1, A2 && a2) const {
    return SO(create(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2))); }
template <typename Type> template <typename A0, typename A1, typename A2, typename A3>
MetaLib_INLINE typename Class_ObjOfType_AutoLife<Type>::SO Class_ObjOfType_AutoLife<Type>::operator()(A0 && a0, A1 && a1, A2 && a2, A3 && a3) const {
    return SO(create(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3))); }
#endif


MetaLib_INLINE O getArg(MetaLib_Obj const ** pobj)
{
    MetaLib_Obj const * r = nullptr;
    std::swap(r, *pobj);
    return O(r);
}


template <typename Type, typename Func> class Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf;

#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename Type, typename R, typename ... A>
class Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<Type, R (A ...)> : public Class_ObjOfType_AutoLife_CastToSelf<Type>
#else
//TODO
template <typename Type, typename R, typename A0, typename A1, typename A2, typename A3>
class Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<Type, R (A0, A1, A2, A3)> : public Class_ObjOfType_AutoLife_CastToSelf<Type>
#endif
{
    typedef Class_ObjOfType_AutoLife_CastToSelf<Type> Base;
private:
#if MetaLib_HAS_VARIADIC_TEMPLATE
    template <size_t ... I>
    static O callImpl(std::true_type tag_is_void, ObjType & self, MetaLib_Obj const ** argv, std::index_sequence<I ...>)
    {
        (void)(tag_is_void);
        (void)(argv);
        (*(self.get()))(getArg(argv + I).cast<A>() ...);
        return O();
    }
#else
    //TODO
    static O callImpl(std::true_type tag_is_void, std::integral_constant<MetaLib_USz, 0> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        (*(self.get()))(); return O();
    }
    static O callImpl(std::true_type tag_is_void, std::integral_constant<MetaLib_USz, 1> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        (*(self.get()))(getArg(argv + 0).cast<A0>()); return O();
    }
    static O callImpl(std::true_type tag_is_void, std::integral_constant<MetaLib_USz, 2> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        (*(self.get()))(getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>()); return O();
    }
    static O callImpl(std::true_type tag_is_void, std::integral_constant<MetaLib_USz, 3> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        (*(self.get()))(getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>()); return O();
    }
    static O callImpl(std::true_type tag_is_void, std::integral_constant<MetaLib_USz, 4> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        (*(self.get()))(getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>(), getArg(argv + 3).cast<A3>()); return O();
    }
#endif
#if MetaLib_HAS_VARIADIC_TEMPLATE
    template <size_t ... I>
    static O callImpl(std::false_type tag_is_void, ObjType & self, MetaLib_Obj const ** argv, std::index_sequence<I ...>)
    {
        (void)(tag_is_void);
        (void)(argv);
        return O(R((*(self.get()))(getArg(argv + I).cast<A>() ...)));
    }
#else
    //TODO
    static O callImpl(std::false_type tag_is_void, std::integral_constant<MetaLib_USz, 0> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        return O(R((*(self.get()))()));
    }
    static O callImpl(std::false_type tag_is_void, std::integral_constant<MetaLib_USz, 1> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        return O(R((*(self.get()))(getArg(argv + 0).cast<A0>())));
    }
    static O callImpl(std::false_type tag_is_void, std::integral_constant<MetaLib_USz, 2> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        return O(R((*(self.get()))(getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>())));
    }
    static O callImpl(std::false_type tag_is_void, std::integral_constant<MetaLib_USz, 3> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        return O(R((*(self.get()))(getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>())));
    }
    static O callImpl(std::false_type tag_is_void, std::integral_constant<MetaLib_USz, 4> tag_used_args, ObjType & self, MetaLib_Obj const ** argv) {
        (void)(tag_is_void); (void)(tag_used_args); (void)(argv);
        return O(R((*(self.get()))(getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>(), getArg(argv + 3).cast<A3>())));
    }
#endif
protected:
    static MetaLib_Obj const * call(
        MetaLib_Obj const * obj,
        MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer,
        MetaLib_USz argc, MetaLib_Obj const ** argv,
        MetaLib_Obj const ** p_exception)
    {
        try
        {
            if (method_name_length == 0)
            {
                //TODO: perhaps const_cast should be in ObjType class.
                auto & self = *const_cast<ObjType *>(static_cast<ObjType const *>(obj));
                //TODO: perhaps the whole constness thing is not necessary.
#if MetaLib_HAS_VARIADIC_TEMPLATE
                if (sizeof...(A) != argc) throw 1;//TODO
                return callImpl(std::integral_constant<bool, std::is_void<R>::value>(), self, argv, std::index_sequence_for<A ...>()).release();
#else
                //TODO
                if (::MetaLib::UsedCount<A0, A1, A2, A3>::value != argc) throw 1;//TODO
                return callImpl(std::integral_constant<bool, std::is_void<R>::value>(),
                    std::integral_constant<MetaLib_USz, ::MetaLib::UsedCount<A0, A1, A2, A3>::value>(),
                    self, argv).release();
#endif
            }
            else
            {
                return Base::call(obj, method_name_length, method_name_pointer, argc, argv, p_exception);
            }
        }
        catch (...)//TODO
        {
            //TODO: make a real exception
            *p_exception = nullptr;
            return ::MetaLib::Null;
        }
    }
public:
    Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf()
        :
        Base()
    {
        p_call = &call;
    }
public:
    static Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf const Class;
};
#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename Type, typename R, typename ... A>
Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<Type, R (A ...)> const
Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<Type, R (A ...)>::Class;
#else
//TODO
template <typename Type, typename R, typename A0, typename A1, typename A2, typename A3>
Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<Type, R (A0, A1, A2, A3)> const
Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<Type, R (A0, A1, A2, A3)>::Class;
#endif




template <typename BaseClass>
class Class_WithCastTable : public BaseClass
{
    typedef BaseClass Base;
    typedef void const * SF(typename Base::ObjType const & self);
private:
    typedef std::map<std::string, std::function<SF>> CastTable;
    bool cast_table_initialized;
    TypeStorage<CastTable> cast_table;

    CastTable const * getCastTable() const
    {
        if (cast_table_initialized)
            return cast_table.get();
        else
            return nullptr;
    }

    CastTable & getInitCastTable()
    {
        auto r = cast_table.get();
        if (!cast_table_initialized)
        {
            new (r) CastTable();
            cast_table_initialized = true;
        }
        return *r;
    }

    void destroyCastTable()
    {
        if (cast_table_initialized)
        {
            auto r = cast_table.get();
            r->~CastTable();
            cast_table_initialized = false;
        }
    }

protected:
    static void const * cast(
        MetaLib_Obj const * obj,
        MetaLib_USz type_name_length, MetaLib_U8 const * type_name_pointer)
    {
        switch (0)
        {
        case 0:
            auto class_ = static_cast<Class_WithCastTable const *>(obj->class_);//TODO: Is triangle static_cast safe?
            auto cast_table = class_->getCastTable();
            if (cast_table == nullptr)
                break;
            auto i = cast_table->find(std::string(reinterpret_cast<char const *>(type_name_pointer), type_name_length));
            if (i == cast_table->end())
                break;
            auto & self = *static_cast<ObjType const *>(obj);
            return (i->second)(self);
        }
        return Base::cast(obj, type_name_length, type_name_pointer);
    }
public:
    template <typename Type, typename F>
    Class_WithCastTable & addCastFunc(F f)
    {
        MetaLib_STATIC_ASSERT(::MetaLib::ABIType<Type>::Is_ABI, "'Type' must be ABI type.");
        auto name = ::MetaLib::ABIType<Type>::getName();
        getInitCastTable()[std::string(name.p<char>(), name.len())] = std::function<SF>([f](ObjType const & self) -> void const *
        {
            auto r = f(&self, *(self.get()));
            typedef decltype(O().castp<Type>()) RetType;
            MetaLib_STATIC_ASSERT((std::is_convertible<decltype(r), RetType>::value), "Return type of 'f' is not compatible with 'Type'.");
            return static_cast<RetType>(r);
        });
        return *this;
    }
public:
    Class_WithCastTable()
        :
        Base(),
        cast_table_initialized(false)
    {
        p_cast = &cast;
    }
    ~Class_WithCastTable()
    {
        destroyCastTable();
    }
public:
    static Class_WithCastTable const Class;
};
template <typename BaseClass> Class_WithCastTable<BaseClass> const Class_WithCastTable<BaseClass>::Class;


template <typename BaseClass>
class Class_WithMethodTable : public BaseClass
{
    typedef BaseClass Base;
    typedef O SF(typename Base::ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv);
public:
    class SO : public Base::SO
    {
    public:
        template <typename BSO>
        SO(BSO && bso)
            :
            Base::SO(std::forward<BSO>(bso))
        {}
        template <typename UnpackSignature, typename F>
        void setf(::MetaLib::StrRef method_name, F f)
        {
            static_cast<Class_WithMethodTable const *>(Base::SO::get()->class_)->setf<UnpackSignature>(*this, method_name, f);
        }
    };
#if MetaLib_HAS_VARIADIC_TEMPLATE
    template <typename ... A>
    SO operator()(A && ... a) const
    {
        return SO(Base::operator()(std::forward<A>(a) ...));
    }
#else
    //TODO
    SO operator()() const
    { return SO(Base::operator()()); }
    template <typename A0>
    SO operator()(A0 && a0) const
    { return SO(Base::operator()(std::forward<A0>(a0))); }
    template <typename A0, typename A1>
    SO operator()(A0 && a0, A1 && a1) const
    { return SO(Base::operator()(std::forward<A0>(a0), std::forward<A1>(a1))); }
    template <typename A0, typename A1, typename A2>
    SO operator()(A0 && a0, A1 && a1, A2 && a2) const
    { return SO(Base::operator()(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2))); }
    template <typename A0, typename A1, typename A2, typename A3>
    SO operator()(A0 && a0, A1 && a1, A2 && a2, A3 && a3) const
    { return SO(Base::operator()(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3))); }
#endif
private:
    typedef std::map<std::string, std::function<SF>> MethodTable;
    bool method_table_initialized;
    TypeStorage<MethodTable> method_table;

    MethodTable const * getMethodTable() const
    {
        if (method_table_initialized)
            return method_table.get();
        else
            return nullptr;
    }

    MethodTable & getInitMethodTable()
    {
        auto r = method_table.get();
        if (!method_table_initialized)
        {
            new (r) MethodTable();
            method_table_initialized = true;
        }
        return *r;
    }

    void destroyMethodTable()
    {
        if (method_table_initialized)
        {
            auto r = method_table.get();
            r->~MethodTable();
            method_table_initialized = false;
        }
    }

#if 0
private://bind obj to method
    class BoundMethod
    {
        O o;//hold a reference, and store 'self'
        std::function<SF> function;
    public:
        BoundMethod(O o, std::function<SF> function)
            :
            o(std::move(o)),
            function(std::move(function))
        {}
    public:
        //This method may throw out exceptions, and Class_ObjOfType_AutoLife_CastToSelf_CallToSelf would catch them.
        MetaLib_Obj const * operator()(MetaLib_USz argc, MetaLib_Obj const ** argv) const
        {
            auto & self = *static_cast<ObjType const *>(o.get());
            return function(self, argc, argv).release();
        }
    };
    //TODO: this function is not finished
    static MetaLib_Obj const * method(
        MetaLib_Obj const * obj,
        MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer)
    {
        try
        {
            auto class_ = static_cast<Class_WithMethodTable const *>(obj->class_);//TODO: Is triangle static_cast safe?
            auto & method_table = class_->method_table;
            auto i = method_table.find(std::string(reinterpret_cast<char const *>(method_name_pointer), method_name_length));//method_name_length can be 0
            if (i == method_table.end())
            {
                //return Base::method(obj, method_name_pointer, method_name_length);//TODO: ?
                return 0;//TODO
            }
            else
            {
                return Class_ObjOfType_AutoLife_CastToSelf_CallToSelf<BoundMethod>::Class(O(obj), i->second).release();
            }
        }
        catch (...)//TODO
        {
            //TODO
            return 0;
        }
    }
#endif
protected:
    static MetaLib_Obj const * call(
        MetaLib_Obj const * obj,
        MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer,
        MetaLib_USz argc, MetaLib_Obj const ** argv,
        MetaLib_Obj const ** p_exception)
    {
        try
        {
            auto & self = *static_cast<ObjType const *>(obj);
            switch (0)
            {
            case 0:
                auto obj_type = Base::castToObjType(obj);
                switch (0)
                {
                case 0:
                    auto obj_method_table = obj_type->getMethodTable();
                    if (obj_method_table == nullptr)
                        break;
                    auto i = obj_method_table->find(std::string(reinterpret_cast<char const *>(method_name_pointer), method_name_length));//method_name_length can be 0
                    if (i == obj_method_table->end())
                        break;
                    return (i->second)(self, argc, argv).release();
                }
                auto class_ = static_cast<Class_WithMethodTable const *>(obj->class_);//TODO: Is triangle static_cast safe?
                auto method_table = class_->getMethodTable();
                if (method_table == nullptr)
                    break;
                auto i = method_table->find(std::string(reinterpret_cast<char const *>(method_name_pointer), method_name_length));//method_name_length can be 0
                if (i == method_table->end())
                    break;
                return (i->second)(self, argc, argv).release();
            }
            return Base::call(obj, method_name_length, method_name_pointer, argc, argv, p_exception);
        }
        catch (...)//TODO
        {
            //TODO: make a real exception
            *p_exception = nullptr;
            return ::MetaLib::Null;
        }
    }
private:
    template <typename F> struct FunctionMaker;
#if MetaLib_HAS_VARIADIC_TEMPLATE
private:
    template <typename R, typename ... A>
    struct FunctionMaker<R (A ...)>
    {
        template <typename F, size_t ... I>
        static std::function<SF> makeImpl(std::false_type tag_is_void, F f, std::index_sequence<I ...>)
        {
            (void)(tag_is_void);
            return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv)
            {
                (void)(argv);
                if (sizeof...(A) != argc) throw 1;//TODO
                return O(R(f(&self, *Base::castToType(&self), getArg(argv + I).cast<A>() ...)));//TODO: rethink 'castToType' and 'self.get'
            });
        }
        template <typename F, size_t ... I>
        static std::function<SF> makeImpl(std::true_type tag_is_void, F f, std::index_sequence<I ...>)
        {
            (void)(tag_is_void);
            return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv)
            {
                (void)(argv);
                if (sizeof...(A) != argc) throw 1;//TODO
                f(&self, *Base::castToType(&self), getArg(argv + I).cast<A>() ...);//TODO: rethink 'castToType' and 'self.get'
                return O();
            });
        }
        template <typename F>
        static std::function<SF> make(F f)
        {
            return makeImpl(std::is_void<R>(), f, std::index_sequence_for<A ...>());
        }
    };
#else
private:
    //TODO
    template <typename R>
    struct FunctionMaker<R (void)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const **     ) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((0 == argc) || (throw 1, true),//TODO
                O(R(f(&self, *Base::castToType(&self))))); }); } };
    template <typename R, typename A0>
    struct FunctionMaker<R (A0)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((1 == argc) || (throw 1, true),//TODO
                O(R(f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>())))); }); } };
    template <typename R, typename A0, typename A1>
    struct FunctionMaker<R (A0, A1)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((2 == argc) || (throw 1, true),//TODO
                O(R(f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>())))); }); } };
    template <typename R, typename A0, typename A1, typename A2>
    struct FunctionMaker<R (A0, A1, A2)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((3 == argc) || (throw 1, true),//TODO
                O(R(f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>())))); }); } };
    template <typename R, typename A0, typename A1, typename A2, typename A3>
    struct FunctionMaker<R (A0, A1, A2, A3)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((4 == argc) || (throw 1, true),//TODO
                O(R(f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>(), getArg(argv + 3).cast<A3>())))); }); } };
    //TODO
    template <>
    struct FunctionMaker<void ()> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const **     ) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((0 == argc) || (throw 1, true),//TODO
                f(&self, *Base::castToType(&self)), O()); }); } };
    template <typename A0>
    struct FunctionMaker<void (A0)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((1 == argc) || (throw 1, true),//TODO
                f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>()), O()); }); } };
    template <typename A0, typename A1>
    struct FunctionMaker<void (A0, A1)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((2 == argc) || (throw 1, true),//TODO
                f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>()), O()); }); } };
    template <typename A0, typename A1, typename A2>
    struct FunctionMaker<void (A0, A1, A2)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((3 == argc) || (throw 1, true),//TODO
                f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>()), O()); }); } };
    template <typename A0, typename A1, typename A2, typename A3>
    struct FunctionMaker<void (A0, A1, A2, A3)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((4 == argc) || (throw 1, true),//TODO
                f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>(), getArg(argv + 3).cast<A3>()), O()); }); } };
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    struct FunctionMaker<void (A0, A1, A2, A3, A4, A5, A6, A7)> { template <typename F> static std::function<SF> make(F f) {
        return std::function<SF>([f](ObjType const & self, MetaLib_USz argc, MetaLib_Obj const ** argv) -> O { typedef Base Base;//VS2010 needs this. This is probably a compiler bug.
            return ((8 == argc) || (throw 1, true),//TODO
                f(&self, *Base::castToType(&self), getArg(argv + 0).cast<A0>(), getArg(argv + 1).cast<A1>(), getArg(argv + 2).cast<A2>(), getArg(argv + 3).cast<A3>(), getArg(argv + 4).cast<A4>(), getArg(argv + 5).cast<A5>(), getArg(argv + 6).cast<A6>(), getArg(argv + 7).cast<A7>()), O()); }); } };
#endif
public:
    template <typename UnpackSignature, typename F>
    Class_WithMethodTable & def(::MetaLib::StrRef method_name, F f)
    {
        getInitMethodTable()[method_name.toStdString<char>()] = FunctionMaker<UnpackSignature>::make(f);
        return *this;
    }
    //template <typename UnpackSignature, typename C, MetaLib_USz N, typename F>
    //typename std::enable_if<std::is_integral<C>::value && (sizeof(C) == 1), Class_WithMethodTable &>::type
    //    def(C const (&method_name)[N], F f)
    //{
    //    method_table[std::basic_string<::MetaLib::U8>(reinterpret_cast<::MetaLib::U8 const *>(method_name), N - 1)] = FunctionMaker<UnpackSignature>::make(f);
    //    return *this;
    //}
public:
    template <typename UnpackSignature, typename F>
    void setf(O const & o, ::MetaLib::StrRef method_name, F f) const
    {
        auto p = Base::castToObjType(o.get());
        p->getInitMethodTable()[method_name.toStdString<char>()] = FunctionMaker<UnpackSignature>::make(f);
    }
public:
    Class_WithMethodTable()
        :
        Base()
    {
        p_call = &call;
#if 0
        auto p = &Class_WithMethodTable::method;//TODO
        (void)(p);
#endif
    }
public:
    static Class_WithMethodTable const Class;
};
template <typename BaseClass> Class_WithMethodTable<BaseClass> const Class_WithMethodTable<BaseClass>::Class;




#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename T, typename ... A>
auto makeO(A && ... a)
{
    return Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<T>::type>::Class(std::forward<A>(a) ...);
}
#else
//TODO: return 'SO' type
//TODO
template <typename T>
O makeO() {
    return Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<T>::type>::Class(
        ); }
template <typename T, typename A0>
O makeO(A0 && a0) {
    return Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<T>::type>::Class(
        std::forward<A0>(a0)); }
template <typename T, typename A0, typename A1>
O makeO(A0 && a0, A1 && a1) {
    return Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<T>::type>::Class(
        std::forward<A0>(a0), std::forward<A1>(a1)); }
template <typename T, typename A0, typename A1, typename A2>
O makeO(A0 && a0, A1 && a1, A2 && a2) {
    return Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<T>::type>::Class(
        std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)); }
template <typename T, typename A0, typename A1, typename A2, typename A3>
O makeO(A0 && a0, A1 && a1, A2 && a2, A3 && a3) {
    return Class_ObjOfType_AutoLife_CastToSelf<typename std::decay<T>::type>::Class(
        std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)); }
#endif


//callable O
#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename F> struct Helper_O_C_Maker;
template <typename R, typename ... A>
struct Helper_O_C_Maker<R (A ...)>
{
    template <typename T, typename ... MA>
    static O make(MA && ... ma)
    {
        return O(Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<T, R (A ...)>::Class.create(std::forward<MA>(ma) ...));
    }
};
template <typename F, typename A>
O makeCO(A && a)
{
    return Helper_O_C_Maker<F>::make<typename std::decay<A>::type>(std::forward<A>(a));
}
template <typename F, typename T, typename ... A>
O makeCO(A && ... a)
{
    return Helper_O_C_Maker<F>::make<typename std::decay<T>::type>(std::forward<A>(a) ...);
}
#else
template <typename F> struct FSigExpandTo4;
template <typename R> struct FSigExpandTo4<R ()> { typedef void Type(::MetaLib::Unused, ::MetaLib::Unused, ::MetaLib::Unused, ::MetaLib::Unused); };
template <typename R, typename A0> struct FSigExpandTo4<R (A0)> { typedef void Type(A0, ::MetaLib::Unused, ::MetaLib::Unused, ::MetaLib::Unused); };
template <typename R, typename A0, typename A1> struct FSigExpandTo4<R (A0, A1)> { typedef void Type(A0, A1, ::MetaLib::Unused, ::MetaLib::Unused); };
template <typename R, typename A0, typename A1, typename A2> struct FSigExpandTo4<R (A0, A1, A2)> { typedef void Type(A0, A1, A2, ::MetaLib::Unused); };
template <typename R, typename A0, typename A1, typename A2, typename A3> struct FSigExpandTo4<R (A0, A1, A2, A3)> { typedef void Type(A0, A1, A2, A3); };
template <typename F, typename A>
O makeCO(A && a)
{
    return O(Class_ObjOfType_AutoLife_CastToSelf_UnpackCallToSelf<typename std::decay<A>::type, typename FSigExpandTo4<F>::Type>::Class
        .create(std::forward<A>(a)));
}
#endif


template <typename T>
struct Class_Type
{
    typedef Class_WithMethodTable<Class_WithCastTable<Class_ObjOfType_AutoLife_CastToSelf<T>>> Type;
};
#if MetaLib_HAS_TEMPLATE_ALIAS
template<typename T>
using Class = typename Class_Type<T>::Type;
#endif


//dynamic O
#if MetaLib_HAS_VARIADIC_TEMPLATE
template <typename T, typename ... A>
auto makeDO(A && ... a)
{
    return Class<typename std::decay<T>::type>::Class(std::forward<A>(a) ...);
}
#else
//TODO
template <typename T>
struct Helper_makeDO
{
    typedef typename Class_Type<typename std::decay<T>::type>::Type::SO SO;
};
template <typename T>
typename Helper_makeDO<T>::SO makeDO() {
    return Class_Type<typename std::decay<T>::type>::Type::Class(
        ); }
template <typename T, typename A0>
typename Helper_makeDO<T>::SO makeDO(A0 && a0) {
    return Class_Type<typename std::decay<T>::type>::Type::Class(
        std::forward<A0>(a0)); }
template <typename T, typename A0, typename A1>
typename Helper_makeDO<T>::SO makeDO(A0 && a0, A1 && a1) {
    return Class_Type<typename std::decay<T>::type>::Type::Class(
        std::forward<A0>(a0), std::forward<A1>(a1)); }
template <typename T, typename A0, typename A1, typename A2>
typename Helper_makeDO<T>::SO makeDO(A0 && a0, A1 && a1, A2 && a2) {
    return Class_Type<typename std::decay<T>::type>::Type::Class(
        std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2)); }
template <typename T, typename A0, typename A1, typename A2, typename A3>
typename Helper_makeDO<T>::SO makeDO(A0 && a0, A1 && a1, A2 && a2, A3 && a3) {
    return Class_Type<typename std::decay<T>::type>::Type::Class(
        std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3)); }
#endif


struct Ignore
{
    template <typename A>
    Ignore(A &&)
    {}
    template <typename A>
    Ignore & operator=(A &&)
    {}
};


}


namespace MetaLib
{
    using MetaLibOPrivate::O;
    using MetaLibOPrivate::acquireObj;
    using MetaLibOPrivate::WO;
    using MetaLibOPrivate::lessOid;
    using MetaLibOPrivate::makeO;
    using MetaLibOPrivate::makeCO;
    using MetaLibOPrivate::makeDO;
    using MetaLibOPrivate::Class_Type;
#if MetaLib_HAS_TEMPLATE_ALIAS
    using MetaLibOPrivate::Class;
#endif

    using MetaLibOPrivate::Ignore;
}


#endif
