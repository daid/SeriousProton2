#include <sp2/logging.h>
#include <sp2/attributes.h>
#include <iostream>

namespace sp {

std::ostream* Logger::stream = &std::cerr;
Logger::Format Logger::format = Logger::Format::Basic;

void Logger::logStart(Level level, const char* filename, const char* function, int line_number)
{
    switch(level)
    {
    case Level::Debug:
        *stream << "[DEBUG] ";
        break;
    case Level::Info:
        *stream << "[INFO] ";
        break;
    case Level::Warning:
        *stream << "[WARNING] ";
        break;
    case Level::Error:
        *stream << "[ERROR] ";
        break;
    }
    switch(format)
    {
    case Format::Basic:
        break;
    case Format::Function:
        *stream << function << ':' << line_number << ":\n";
        break;
    case Format::Full:
        *stream << filename << ':' << function << ':' << line_number << ":\n";
        break;
    }
}

void Logger::logEnd()
{
    *stream << "\n";
}

};//namespace sp
