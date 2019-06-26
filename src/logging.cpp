#include <sp2/logging.h>
#include <sp2/attributes.h>
#include <iostream>
#include <fstream>

namespace sp {

static std::ofstream log_file_stream;
std::ostream* Logger::stream = &std::cerr;
Logger::Format Logger::format = Logger::Format::Basic;
Logger::Level Logger::level = Logger::Level::Debug;


void Logger::setOutputFile(const string& filename)
{
    if (log_file_stream.is_open())
        log_file_stream.close();
    log_file_stream.open(filename.c_str());
    if (log_file_stream.is_open())
    {
        stream = &log_file_stream;
    }
    else
    {
        stream = &std::cerr;
        LOG(Error, "Failed to open log file:", filename);
    }
}

void Logger::setOutputStream(std::ostream* stream)
{
    Logger::stream = stream;
}

void Logger::setFormat(Format format)
{
    Logger::format = format;
}

void Logger::setLevel(Level level)
{
    Logger::level = level;
}

void Logger::logStart(Level level, const char* filename, const char* function, int line_number)
{
    if (level < Logger::level)
        return;
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
