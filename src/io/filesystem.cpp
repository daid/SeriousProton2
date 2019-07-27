#include <sp2/io/filesystem.h>

#include <sys/stat.h>


namespace sp {
namespace io {

static int lastPathSeperator(const sp::string& path)
{
    for(int n = path.length() - 1; n > 0; n--)
    {
#ifdef __WIN32__
        if (path[n] == '/' || path[n] == '\\')
            return n;
#else
        if (path[n] == '/')
            return n;
#endif
    }
    return -1;
}

bool isFile(const sp::string& path)
{
    struct stat s;
    if (stat(path.c_str(), &s))
        return false;
    return S_ISREG(s.st_mode);
}

bool isDirectory(const sp::string& path)
{
    struct stat s;
    if (stat(path.c_str(), &s))
        return false;
    return S_ISDIR(s.st_mode);
}

bool makeDirectory(const sp::string& path)
{
    if (isDirectory(path))
        return true;
    int n = lastPathSeperator(path);
    if (n > 0)
    {
        sp::string base = path.substr(0, n);
        if (!makeDirectory(base))
            return false;
    }
#ifdef __WIN32__
    return mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0777) == 0;
#endif
}

sp::string dirname(const sp::string& path)
{
    int n = lastPathSeperator(path);
    if (n < 0)
        return "";
    return path.substr(0, n);
}

sp::string basename(const sp::string& path)
{
    int n = lastPathSeperator(path);
    return path.substr(n + 1);
}

bool saveFileContents(const sp::string& filename, const sp::string& contents)
{
    FILE* f = fopen(filename.c_str(), "wb");
    if (!f)
        return false;
    if (fwrite(contents.data(), contents.length(), 1, f) < 1)
    {
        fclose(f);
        remove(filename.c_str());
        return false;
    }
    fclose(f);
    return true;
}

sp::string loadFileContents(const sp::string& filename)
{
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return "";
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    string result;
    result.resize(size);
    fread(&result[0], size, 1, f);
    fclose(f);
    return result;
}

};//namespace io
};//namespace sp
