#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <random>

#include <sp2/random.h>

namespace sp {

static std::mt19937_64 random_engine;

class StaticRandomInitializer
{
public:
    StaticRandomInitializer()
    {
        random_engine.seed(time(NULL));
    }
};
static StaticRandomInitializer static_random_initializer;

float random(float fmin, float fmax)
{
    return std::uniform_real_distribution<>(fmin, fmax)(random_engine);
}

int irandom(int imin, int imax)
{
    return std::uniform_int_distribution<>(imin, imax)(random_engine);
}

bool chance(float percentage)
{
    return std::bernoulli_distribution(percentage / 100.0f)(random_engine);
}

}//namespace sp
