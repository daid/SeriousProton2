#include <sp2/assert.h>
#include <sp2/logging.h>
#include <cstdlib>
#include <cstdio>
#include <exception>

void __sp2assert(const char* e, const char* filename, const char* function, int line_number, const char* msg)
{
    LOG(Error, "--------------------------------------------------");
    LOG(Error, "Assertion failed:", e, "at", filename, function, line_number);
    LOG(Error, msg);

    __builtin_trap();
}
