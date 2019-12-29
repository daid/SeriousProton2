#ifndef SP2_RANDOM_H
#define SP2_RANDOM_H

#include <iterator>

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

}//namespace sp

#endif//RANDOM_H
