#if 0

#include "MetaLib.h"
#include <climits>
#include <intrin.h>

using namespace MetaLib;

int f(int, double) { return 0; }

int main()
{
    {
        //MetaLib::Obj_M<std::unique_ptr<int>> a(std::unique_ptr<int>(new int{ 1 }));
        //MetaLib::Obj_M<int> b(1);
        //auto e = (((MetaLib::Obj *)(&a))->getType() == ((MetaLib::Obj *)(&b))->getType());
        (void)("汉字");
    }
    {
        O o1 = makeO<int, void>(123);
        O o2 = makeO<std::unique_ptr<int>, void>(std::unique_ptr<int>(new int{ 123 }));
        auto e = (o1.cast<int>() == o2.cast<int>());
        e = !!e;
        _mm_pause();
    }
    {
        O o1 = O(123);
        O o2 = O(std::unique_ptr<int>(new int{ 123 }));
        auto e = (o1.cast<int>() == o2.cast<int>());
        e = !!e;
        _mm_pause();
    }
    {
        int t = 123;
        O o1 = makeO<int, void>(t);
        O o2 = makeO<double, void>(t);
        O o3 = makeO<int, void>(123);
        O o4 = makeO<double, void>(123);
        O o5 = O(123);
        O o6 = O(123.0);
        O o7 = O(t);
        _mm_pause();
        //o7.cast<double>();
        //O o8 = o1(O());
    }
    {
        O o1 = makeO<int(short, double)>(f);
        o1(1111111, 2);//gotcha
        double t = 1.2;
        O o2 = makeO<double(double)>([t](double a) { return a + t; });
        auto r = o2(2.3);
        _mm_pause();
    }
    return 0;
    //return makeO(123).cast<int>();
}

#endif


#if 0

#include "MetaLibO.h"
using namespace MetaLib;

int main()
{
    auto & c = MetaLibPrivate::Class_Store<int>::Class;
    auto p = c.alloc();
    new(&(p->m)) int;
    //constexpr StaticString a = ABIType<F64>::getName();
    //constexpr auto c = a.first[0];
    //constexpr auto n = a.second;
    auto b = ABIType<F64 []>::getName();
    return 0;
}

#endif


/*
#if 1

int main()
{
    return 0;
}

#endif
*/

