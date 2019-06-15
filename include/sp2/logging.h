#ifndef SP2_LOGGING_H
#define SP2_LOGGING_H

//We include iostream here to ensure std::cerr is initialized, else the logging functions can crash in constructors.
#include <iostream>

#include <ostream>
#include <vector>
#include <unordered_map>

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
    enum class Format
    {
        Basic,
        Function,
        Full
    };
    template<typename... ARGS> static void log(Level level, const char* filename, const char* function, int line_number, const ARGS&... args)
    {
        logStart(level, filename, function, line_number);
        logArg(args...);
        logEnd();
    }
private:
    static std::ostream* stream;
    static Format format;

    template<typename A1> static inline void logArg(const A1& a)
    {
        *stream << a;
    }
    template<typename A1> static inline void logArg(const std::vector<A1>& a)
    {
        *stream << "[";
        bool first = true;
        for(const auto& e : a)
        {
            if (first)
                first = false;
            else
                *stream << ", ";
            logArg(e);
        }
        *stream << "]";
    }
    template<typename A1, typename A2> static inline void logArg(const std::unordered_map<A1, A2>& a)
    {
        *stream << "{";
        bool first = true;
        for(auto e : a)
        {
            if (first)
                first = false;
            else
                *stream << ", ";
            logArg(e.first);
            *stream << "=";
            logArg(e.second);
        }
        *stream << "}";
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

};//namespace sp

#define LOG(level, ...) sp::Logger::log(sp::Logger::Level::level, __FILE__, __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)

#endif//SP2_LOGGING_H
