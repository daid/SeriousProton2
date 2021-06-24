#include <sp2/io/resourceProvider.h>
#include <sp2/io/directoryResourceProvider.h>
#include <sp2/io/zipResourceProvider.h>
#include <sp2/io/filesystem.h>
#include <sp2/attributes.h>

#ifdef _WIN32
#include <windows.h>
#endif//_WIN32

#ifdef ANDROID
#include <SDL.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class AndroidAssetResourceStream : public sp::io::ResourceStream
{
public:
    AndroidAssetResourceStream(AAsset* asset)
    : asset(asset)
    {
    }

    virtual ~AndroidAssetResourceStream()
    {
        AAsset_close(asset);
    }
    
    virtual int64_t read(void* data, int64_t size) override
    {
        return AAsset_read(asset, data, size);
    }
    virtual int64_t seek(int64_t position) override
    {
        return AAsset_seek(asset, position, SEEK_SET);
    }
    virtual int64_t tell() override
    {
        return AAsset_seek(asset, 0, SEEK_CUR);
    }
    virtual int64_t getSize() override
    {
        return AAsset_getLength64(asset);
    }

private:
    AAsset* asset;
};

class AndroidAssetsResourceProvider : public sp::io::ResourceProvider
{
public:
    AndroidAssetsResourceProvider(const sp::string& base_path, int priority=0)
    : ResourceProvider(priority), base_path(base_path)
    {
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        jobject activity = (jobject)SDL_AndroidGetActivity();
        jclass clazz(env->GetObjectClass(activity));
        jmethodID method_id = env->GetMethodID(clazz, "getAssets", "()Landroid/content/res/AssetManager;");
        asset_manager_jobject = env->CallObjectMethod(activity, method_id);
        asset_manager = AAssetManager_fromJava(env, asset_manager_jobject);

        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(clazz);
    }

    virtual ~AndroidAssetsResourceProvider()
    {
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        env->DeleteLocalRef(asset_manager_jobject);
    }
    
    virtual sp::io::ResourceStreamPtr getStream(const sp::string& filename) override
    {
        AAsset* asset = AAssetManager_open(asset_manager, (base_path + "/" + filename).c_str(), AASSET_MODE_UNKNOWN);
        if (asset)
            return std::make_shared<AndroidAssetResourceStream>(asset);
        return nullptr;
    }

    virtual std::vector<sp::string> findResources(const sp::string& search_pattern) override
    {
        std::vector<sp::string> results;

        int idx = search_pattern.rfind("/");
        sp::string path = base_path;
        sp::string prefix = "";
        if (idx > -1)
        {
            prefix = search_pattern.substr(0, idx);
            path += "/" + prefix;
            prefix += "/";
        }

        AAssetDir* dir = AAssetManager_openDir(asset_manager, (path).c_str());
        if (dir)
        {
            const char* filename;
            while ((filename = AAssetDir_getNextFileName(dir)) != nullptr)
            {
                if (searchMatch(prefix + filename, search_pattern))
                    results.push_back(prefix + filename);
            }
            AAssetDir_close(dir);
        }
        return results;
    }

private:
    sp::string base_path;
    
    jobject asset_manager_jobject;
    AAssetManager* asset_manager;
};
#endif//ANDROID

namespace sp {
namespace io {

PList<ResourceProvider> ResourceProvider::providers SP2_INIT_EARLY;

ResourceProvider::ResourceProvider(int priority)
: priority(priority)
{
    providers.add(this);
    providers.sort([](P<ResourceProvider> a, P<ResourceProvider> b)
    {
        return b->priority - a->priority;
    });
}

bool ResourceProvider::searchMatch(const string& name, const string& search_pattern)
{
    std::vector<string> parts = search_pattern.split("*");
    int pos = 0;
    if (parts[0].length() > 0)
    {
        if (name.find(parts[0]) != 0)
            return false;
    }
    for(unsigned int n=1; n<parts.size(); n++)
    {
        if (parts[n] == "" && n == parts.size() - 1)
            return true;
        int offset = name.find(parts[n], pos);
        if (offset < 0)
            return false;
        pos = offset + parts[n].length();
    }
    return pos == int(name.length());
}

string ResourceStream::readLine()
{
    string ret;
    char c;
    while(true)
    {
        if (read(&c, 1) < 1)
            return ret;
        if (c == '\n')
        {
            if (!ret.empty() && ret.back() == '\r')
                ret.pop_back();
            return ret;
        }
        ret += string(c);
    }
}

string ResourceStream::readAll()
{
    unsigned int size = getSize();
    string result;
    result.resize(size);
    read(&result[0], size);
    return result;
}

bool ResourceStream::hasFlag(const string& name)
{
    return flags.find(name) != flags.end();
}

string ResourceStream::getFlag(const string& name)
{
    auto it = flags.find(name);
    if (it == flags.end())
        return "";
    return it->second;
}

ResourceStreamPtr ResourceProvider::get(string filename)
{
    string flags;
    if (filename.find("#") > -1)
    {
        flags = filename.substr(filename.find("#") + 1);
        filename = filename.substr(0, filename.find("#"));
    }
    for(P<ResourceProvider> rp : providers)
    {
        ResourceStreamPtr stream = rp->getStream(filename);
        if (stream)
        {
            for(auto item : flags.split(","))
            {
                if (!item.empty())
                {
                    auto parts = item.partition("=");
                    stream->flags[parts.first] = parts.second;
                }
            }
            return stream;
        }
    }
    return nullptr;
}

std::chrono::system_clock::time_point ResourceProvider::getModifyTime(const string& filename)
{
    for(P<ResourceProvider> rp : providers)
    {
        std::chrono::system_clock::time_point time = rp->getResourceModifyTime(filename);
        if (time != std::chrono::system_clock::time_point())
            return time;
    }
    return std::chrono::system_clock::time_point();
}

std::vector<string> ResourceProvider::find(const string& search_pattern)
{
    std::vector<string> found_files;
    for(P<ResourceProvider> rp : providers)
    {
        std::vector<string> res = rp->findResources(search_pattern);
        found_files.insert(found_files.end(), res.begin(), res.end());
    }
    return found_files;
}

void ResourceProvider::createDefault()
{
#ifdef _WIN32
    // Check if our executable is possibly a zip file. And if it is, add a zipResourceProvider to read from it.
    char path[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));

    FILE* f = fopen(path, "rb");
    if (f)
    {
        fseek(f, -22, SEEK_END);
        uint32_t signature;
        if (fread(&signature, sizeof(signature), 1, f) == 1)
        {
            if (signature == 0x06054b50)
            {
                new ZipResourceProvider(path);
            }
        }
        fclose(f);
    }
#endif//_WIN32
#ifdef ANDROID
    new AndroidAssetsResourceProvider("resources");
#endif

    if (isDirectory("resources"))
    {
        new DirectoryResourceProvider("resources");
    }
}

}//namespace io
}//namespace sp
