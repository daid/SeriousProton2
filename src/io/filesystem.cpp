#include <sp2/io/filesystem.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <SDL.h>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace sp {
namespace io {

static int lastPathSeperator(const string& path)
{
    for(int n = path.length() - 1; n > 0; n--)
    {
#ifdef _WIN32
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
#ifdef _WIN32
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
#ifdef __EMSCRIPTEN__
    EM_ASM(FS.syncfs(false, function(err) {}));
#endif
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

const string& preferencePath()
{
    static string preference_path;

    if (preference_path.length() == 0)
    {
        const char* input_path = nullptr;
#ifdef EMSCRIPTEN
        //For emscripten, we have a virtual filesystem, so we do not really care on the exact location.
        preference_path = "/preferences/";
        return preference_path;
#elif defined(ANDROID)
        //For android, we have local storage per app. SDL_GetPrefPath handles this.
        input_path = nullptr;
#elif defined(__linux__)
        //On linux, we need our application name, grab it from the proc interface
        char executable_name[PATH_MAX] = {0};
        if (readlink("/proc/self/exe", executable_name, sizeof(executable_name) - 1) > 1)
        {
            input_path = strrchr(executable_name, '/');
            if (input_path)
                input_path += 1;
        }
#elif defined(_WIN32)
        //On windows, we need our application name, we grab the name of our executable and use that.
        // And then use the SDL function to get our actual path.
        char executable_name[MAX_PATH] = {0};
        GetModuleFileNameA(nullptr, executable_name, sizeof(executable_name) - 1);
        input_path = strrchr(executable_name, '\\');
        if (input_path)
        {
            input_path += 1;
            if (strrchr(executable_name, '.'))
                *strrchr(executable_name, '.') = '\0';
        }
#endif
        char* pref_path = SDL_GetPrefPath(nullptr, input_path);

        if (!pref_path)
        {
            preference_path = "./";
        }
        else
        {
            preference_path = pref_path;
            SDL_free(pref_path);
        }
    }

    return preference_path;
}

}//namespace io
}//namespace sp
