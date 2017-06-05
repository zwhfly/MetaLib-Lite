#ifndef MetaLibObj_H_
#define MetaLibObj_H_


#include "MetaLibPort.h"
#include "MetaLibRefCount.h"


struct MetaLib_Class;


//All MetaLib_Obj must be created non-const
struct MetaLib_Obj
{
    MetaLib_RefCount _ref_count_content;
    MetaLib_RefCount _ref_count_memory;
    struct MetaLib_Class const * class_;
};


#ifdef __cplusplus
namespace MetaLib
{
    typedef ::MetaLib_Obj Obj;
}
#endif

/* DO NOT increase/decrease reference of this object */
/* DO NOT use directly */
MetaLib_EXTERN_C MetaLib_EXP(MetaLib_Obj) MetaLib_Obj_NULL;
static MetaLib_INLINE MetaLib_CONSTEXPR MetaLib_Obj const * MetaLib_Obj_getNull()
{
    return &MetaLib_Obj_NULL;
}
#ifdef __cplusplus
namespace MetaLib
{
static MetaLib_CONSTEXPR MetaLib_Obj const * const Null = MetaLib_Obj_getNull();
}
#endif


static MetaLib_INLINE void MetaLib_Obj_init(struct MetaLib_Obj * obj, struct MetaLib_Class const * class_)
{
    obj->class_ = class_;
    MetaLib_RefCount_init(&(obj->_ref_count_memory), 1);
    MetaLib_RefCount_init(&(obj->_ref_count_content), 1);
}


typedef MetaLib_Obj const * MetaLib_Class_Op_Call(
    MetaLib_Obj const * obj,
    MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer,//<anything, 0> is one valid method name
    MetaLib_USz argc, MetaLib_Obj const ** argv,
    MetaLib_Obj const ** p_exception);
typedef void const * MetaLib_Class_Op_Cast(
    MetaLib_Obj const * obj,
    MetaLib_USz type_name_length, MetaLib_U8 const * type_name_pointer);
/* TODO
typedef MetaLib_Obj const * MetaLib_Class_Op_Method(
    MetaLib_Obj const * obj,
    MetaLib_USz method_name_length, MetaLib_U8 const * method_name_pointer);//TODO: what if method does not exist
*/
typedef void MetaLib_Class_Op_Kill(MetaLib_Obj const * obj);
typedef void MetaLib_Class_Op_Drop(MetaLib_Obj const * obj);
struct MetaLib_Class
{
    MetaLib_Class_Op_Call * p_call;
    MetaLib_Class_Op_Cast * p_cast;
    /* MetaLib_Class_Op_Method * p_method; */
    MetaLib_Class_Op_Kill * p_kill;//TODO: exception?
    MetaLib_Class_Op_Drop * p_drop;
};


static MetaLib_INLINE MetaLib_USz MetaLib_Obj_getrefMemory(MetaLib_Obj const * obj)
{
    return MetaLib_RefCount_load(&(obj->_ref_count_memory));
}

static MetaLib_INLINE MetaLib_USz MetaLib_Obj_getrefContent(MetaLib_Obj const * obj)
{
    return MetaLib_RefCount_load(&(obj->_ref_count_content));
}

/* return */
static MetaLib_INLINE MetaLib_USz MetaLib_Obj_increfMemoryBy1(MetaLib_Obj const * obj)
{
    MetaLib_RefCount * p_ref_count_memory = (MetaLib_RefCount *)(&(obj->_ref_count_memory));/* const_cast */
    MetaLib_USz result_ref_count_memory;
    if (MetaLib_USz err = MetaLib_RefCount_increase(p_ref_count_memory, &result_ref_count_memory, 1)) return err;
    return 0;
}

/* return */
static MetaLib_INLINE MetaLib_USz MetaLib_Obj_decrefMemoryBy1(MetaLib_Obj const * obj)
{
    MetaLib_RefCount * p_ref_count_memory = (MetaLib_RefCount *)(&(obj->_ref_count_memory));/* const_cast */
    MetaLib_USz result_ref_count_memory;
    if (MetaLib_USz err = MetaLib_RefCount_increase(p_ref_count_memory, &result_ref_count_memory, -1)) return err;
    if (result_ref_count_memory == 0)
    {
        if (0 != MetaLib_Obj_getrefContent(obj)) return 1;/* TODO: error code */
        obj->class_->p_drop(obj);
    }
    return 0;
}

/* return */
static MetaLib_INLINE MetaLib_USz MetaLib_Obj_increfContentBy1(MetaLib_Obj const * obj)
{
    MetaLib_RefCount * p_ref_count_content = (MetaLib_RefCount *)(&(obj->_ref_count_content));/* const_cast */
    MetaLib_USz result_ref_count_content;
    if (MetaLib_USz err = MetaLib_RefCount_increase(p_ref_count_content, &result_ref_count_content, 1)) return err;
    return 0;
}

/* return */
static MetaLib_INLINE MetaLib_USz MetaLib_Obj_decrefContentBy1(MetaLib_Obj const * obj)
{
    MetaLib_RefCount * p_ref_count_content = (MetaLib_RefCount *)(&(obj->_ref_count_content));/* const_cast */
    MetaLib_USz result_ref_count_content;
    if (MetaLib_USz err = MetaLib_RefCount_increase(p_ref_count_content, &result_ref_count_content, -1)) return err;
    if (result_ref_count_content == 0)
    {
        obj->class_->p_kill(obj);
        MetaLib_Obj_decrefMemoryBy1(obj);
    }
    return 0;
}

/* return */
/* p_not_0 */
static MetaLib_INLINE MetaLib_USz MetaLib_Obj_increfContentNot0By1(MetaLib_Obj const * obj, MetaLib_U8 * p_not_0)
{
    MetaLib_RefCount * p_ref_count_content = (MetaLib_RefCount *)(&(obj->_ref_count_content));/* const_cast */
    MetaLib_USz result_ref_count_content;
    if (MetaLib_USz err = MetaLib_RefCount_increaseNot0(p_ref_count_content, &result_ref_count_content, 1)) return err;
    if (result_ref_count_content == 1) return 1;/* TODO: error code */
    *p_not_0 = (result_ref_count_content == 0) ? 0 : 1;
    return 0;
}


#endif
