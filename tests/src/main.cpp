#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_IMPLEMENT

#include "doctest.h"

/*
static void handler(const doctest::AssertData& ad)
{
    using namespace doctest;

    std::cout << Color::LightGrey << skipPathFromFilename(ad.m_file) << "(" << ad.m_line << "): ";
    std::cout << Color::Red << failureString(ad.m_at) << ": ";

    if(ad.m_at & assertType::is_normal)
    {
        std::cout << Color::Cyan << assertString(ad.m_at) << "( " << ad.m_expr << " ) ";
        std::cout << Color::None << (ad.m_threw ? "THREW exception: " : "is NOT correct!\n");
        if(ad.m_threw)
            std::cout << ad.m_exception;
        else
            std::cout << "  values: " << assertString(ad.m_at) << "( " << ad.m_decomp << " )";
    }
    else
    {
        std::cout << Color::None << "an assert dealing with exceptions has failed!";
    }

    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    doctest::Context context(argc, argv);
    context.setAsDefaultForAssertsOutOfTestCases();
    context.setAssertHandler(handler);

    CHECK(true == false);
    CHECK(5 == 10);
}
*/
