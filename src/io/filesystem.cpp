#include <sp2/io/filesystem.h>

#include <dirent.h>
#include <sys/stat.h>
#include <SDL.h>


namespace sp {
namespace io {

static int lastPathSeperator(const string& path)
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

bool isFile(const string& path)
{
    struct stat s;
    if (stat(path.c_str(), &s))
        return false;
    return S_ISREG(s.st_mode);
}

bool isDirectory(const string& path)
{
    struct stat s;
    if (stat(path.c_str(), &s))
        return false;
    return S_ISDIR(s.st_mode);
}

bool makeDirectory(const string& path)
{
    if (isDirectory(path))
        return true;
    int n = lastPathSeperator(path);
    if (n > 0)
    {
        string base = path.substr(0, n);
        if (!makeDirectory(base))
            return false;
    }
#ifdef __WIN32__
    return mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0777) == 0;
#endif
}

string dirname(const string& path)
{
    int n = lastPathSeperator(path);
    if (n < 0)
        return "";
    return path.substr(0, n);
}

string basename(const string& path)
{
    int n = lastPathSeperator(path);
    return path.substr(n + 1);
}

std::vector<string> listFiles(const string& path)
{
    std::vector<string> result;
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return result;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_name[0] == '.')
            continue;
        result.push_back(entry->d_name);
    }
    closedir(dir);
    return result;
}

bool saveFileContents(const string& filename, const string& contents)
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

string loadFileContents(const string& filename)
{
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return "";
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    string result;
    result.resize(size);
    size = fread(&result[0], 1, size, f);
    result.resize(size);
    fclose(f);
    return result;
}

const string& preferencePath(const string& application_name)
{
    static string preference_path;

    if (preference_path.length() == 0)
    {
        char* path = SDL_GetPrefPath(nullptr, application_name.c_str());
        if (!path)
        {
            preference_path = "./";
        }
        else
        {
            preference_path = path;
            SDL_free(path);
        }
    }

    return preference_path;
}

}//namespace io
}//namespace sp
