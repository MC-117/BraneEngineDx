#pragma once
#undef max
#undef min
#include "oneapi/tbb.h"

template<class Index, class Func>
void parallelFor(Index start, Index end, Func func)
{
    oneapi::tbb::parallel_for(start, end, func);
}

template<class Iter>
void parallelSort(Iter begin, Iter end)
{
    oneapi::tbb::parallel_sort(begin, end);
}

template<class Iter, class Comp>
void parallelSort(Iter begin, Iter end, const Comp& comp)
{
    oneapi::tbb::parallel_sort(begin, end, comp);
}
