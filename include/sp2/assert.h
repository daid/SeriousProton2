#ifndef SP2_ASSERT_H
#define SP2_ASSERT_H

//We include iostream here to ensure std::cerr is initialized, else the logging functions can crash in constructors.
#include <iostream>
#include <sp2/attributes.h>

#ifndef NO_ASSERT
#define sp2assert(e, msg) do { if (!(e)) { __sp2assert(#e, __FILE__, __FUNCTION__, __LINE__, msg); } } while(0)
#else
#define sp2assert(e, msg) do { } while(0)
#endif

void __sp2assert(const char* e, const char* filename, const char* function, int line_number, const char* msg) SP2_NO_RETURN;

#endif//SP2_ASSERT_H
