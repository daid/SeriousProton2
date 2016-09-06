#include <sp2/logging.h>
#include <iostream>

namespace sp {

std::ostream* Logger::logging_stream = &std::cerr;

void Logger::logStart(Level level, const char* filename, const char* function, int line_number)
{
    switch(level)
    {
    case Level::Debug:
        *logging_stream << "[DEBUG] ";
        break;
    case Level::Info:
        *logging_stream << "[INFO] ";
        break;
    case Level::Warning:
        *logging_stream << "[WARNING] ";
        break;
    case Level::Error:
        *logging_stream << "[ERROR] ";
        break;
    }
    //*logging_stream << filename << ':' << function << ':' << line_number << ": ";
    *logging_stream << function << ':' << line_number << ":\n";
}

void Logger::logEnd()
{
    *logging_stream << "\n";
}

};//!namespace sp
