#include <sp2/logging.h>
#include <sp2/engine.h>
#include <sp2/exception/unhandled.h>

#include <thread>

namespace sp {
namespace exception {

static bool unhandled_handler_set = false;
static bool handling_exception = false;
static UnhandledExceptionCallback unhandled_exception_callback;

void unhandledExceptionHandler(const UnhandledExceptionInfo& info)
{
    bool inception = handling_exception;

    handling_exception = true;
    LOG(Error, "--------------------------------------------------");
    if (inception)
        LOG(Error, "Exception during application specific exception handling!");
    LOG(Error, info.base_report);

    for(const AddressInfo& info : info.stack)
    {
        LOG(Error, info.address, info.module.substr(info.module.replace("\\", "/").rfind("/") + 1), "@", (void*)info.module_offset, "=", info.symbol, "+", info.symbol_offset);
    }
    LOG(Error, "--------------------------------------------------");

    //Try to shutdown our engine, which closes the window. So we drop out of full screen mode.
    //This means any platform specific exception dialog will be easier to access.
    if (sp::Engine::getInstance())
        sp::Engine::getInstance()->shutdown();
    
    if (!inception && unhandled_exception_callback)
    {
        //Run the unhandled exception callback on a new thread, this means if another exception happens, we end up in this function again.
        //Else we will just crash by normal means.
        std::thread t([&info]()
        {
            unhandled_exception_callback(info);
        });
        t.join();
    }
    //Returning here will drop to the platform specific exception handling.
}

void setupUnhandledExceptionHandler();

class SetupUnhandledExceptionHandler
{
public:
    SetupUnhandledExceptionHandler()
    {
        if (!unhandled_handler_set)
        {
            setupUnhandledExceptionHandler();
            unhandled_handler_set = true;
        }
    }
};

static SetupUnhandledExceptionHandler setup_unhandled_exception_handler;

void setUnhandledExceptionCallback(UnhandledExceptionCallback callback)
{
    unhandled_exception_callback = callback;
}

};//!namespace exception
};//!namespace sp
