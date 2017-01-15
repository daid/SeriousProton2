#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

PList<ResourceProvider> ResourceProvider::providers;

ResourceProvider::ResourceProvider()
{
    providers.add(this);
}

bool ResourceProvider::searchMatch(const string name, const string search_pattern)
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
            return ret;
        ret += string(c);
    }
}

ResourceStreamPtr ResourceProvider::get(const string filename)
{
    for(ResourceProvider* rp : providers)
    {
        ResourceStreamPtr stream = rp->getStream(filename);
        if (stream)
            return stream;
    }
    return nullptr;
}

std::vector<string> ResourceProvider::find(string search_pattern)
{
    std::vector<string> found_files;
    for(ResourceProvider* rp : providers)
    {
        std::vector<string> res = rp->findFilenames(search_pattern);
        found_files.insert(found_files.end(), res.begin(), res.end());
    }
    return found_files;
}

};//!namespace io
};//!namespace sp
