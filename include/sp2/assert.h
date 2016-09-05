#ifndef SP2_ASSERT_H
#define SP2_ASSERT_H

#ifndef NO_ASSERT
#define sp2assert(e) do { if (!(e)) { __sp2assert(#e, __FILE__, __FUNCTION__, __LINE__); } } while(0)
#else
#define sp2assert(e) do { } while(0)
#endif

void __sp2assert(const char* e, const char* filename, const char* function, int line_number);

#endif//SP2_ASSERT_H
