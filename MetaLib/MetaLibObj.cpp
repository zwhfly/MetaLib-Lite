#include "MetaLibObj.h"


MetaLib_Obj MetaLib_Obj_NULL;


static void init()
{
    MetaLib_Obj_init(&MetaLib_Obj_NULL, nullptr);//TODO: make a real class
}

static int init_hook = (init(), 0);
