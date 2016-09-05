#ifndef SP2_LOGGING_H
#define SP2_LOGGING_H

#include <ostream>

namespace sp {

class Logger
{
public:
    enum class Level
    {
        Debug,
        Info,
        Warning,
        Error
    };
    template<typename... ARGS> static void log(Level level, const char* filename, const char* function, int line_number, const ARGS&... args)
    {
        logStart(level, filename, function, line_number);
        logArg(args...);
        logEnd();
    }
private:
    static std::ostream* logging_stream;

    template<typename A1> static inline void logArg(const A1& a)
    {
        *logging_stream << a;
    }
    template<typename A1, typename... ARGS> static inline void logArg(const A1& a, const ARGS&... args)
    {
        logArg(a);
        logArg(' ');
        logArg(args...);
    }
    
    static void logStart(Level level, const char* filename, const char* function, int line_number);
    static void logEnd();
};

};//!namespace sp

#define LOG(level, ...) sp::Logger::log(sp::Logger::Level::level, __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)

#endif//SP2_LOGGING_H
