#ifndef SP2_RANDOM_H
#define SP2_RANDOM_H

#include <iterator>
#include <sp2/pointerList.h>

namespace sp {

float random(float fmin, float fmax);
int irandom(int imin, int imax);
bool chance(float percentage);

template<typename Iter> Iter randomSelect(Iter start, Iter end)
{
    std::advance(start, irandom(0, std::distance(start, end) - 1));
    return start;
}

template<typename Container> typename Container::iterator randomSelect(Container& container)
{
    return randomSelect(container.begin(), container.end());
}

template<typename Container> typename Container::const_iterator randomSelect(const Container& container)
{
    return randomSelect(container.begin(), container.end());
}

template<typename T> P<T> randomSelect(PList<T>& container)
{
    auto n = irandom(0, container.size() - 1);
    for(auto t : container) {
        if (n) n--; else return t;
    }
    return nullptr;
}

}//namespace sp

#endif//RANDOM_H
