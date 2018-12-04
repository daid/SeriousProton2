#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/logging.h>

namespace sp {
namespace io {

class ParsingException : public std::exception
{
public:
    string message;

    ParsingException(string message) : message(message) {}
};

KeyValueTreePtr KeyValueTreeLoader::load(string resource_name)
{
    try
    {
        KeyValueTreeLoader loader(resource_name);
        return loader.result;
    }catch(ParsingException& e)
    {
        LOG(Error, "Error loading", resource_name, e.message);
        return nullptr;
    }
}

KeyValueTreeLoader::KeyValueTreeLoader(string resource_name)
{
    result = std::make_shared<KeyValueTree>();

    stream = ResourceProvider::get(resource_name);
    if (!stream)
        throw ParsingException("Failed to open " + resource_name + " for tree loading");
    LOG(Info, "Loading tree", resource_name);

    while(stream->tell() < stream->getSize())
    {
        string line = stream->readLine().strip();
        int comment_start = line.find("//");
        if (comment_start >= 0)
            line = line.substr(0, comment_start);
        if (line == "{")
        {
            //New anomounous node.
            result->root_nodes.emplace_back();
            parseNode(&result->root_nodes.back());
        }
        else if (line.startswith("[") && line.find("]") > -1 && line.endswith("{"))
        {
            //New named node.
            result->root_nodes.emplace_back(line.substr(1, line.find("]")));
            parseNode(&result->root_nodes.back());
        }
        else if (line == "}")
        {
            throw ParsingException("Node close while no node open.");
        }
        else if (line.startswith("#"))
        {
            //Comment line.
        }
        else if (line.length() > 0)
        {
            LOG(Error, "Failed to parse line:", line);
            result->root_nodes.clear();
            return;
        }
    }
}

void KeyValueTreeLoader::parseNode(KeyValueTreeNode* node)
{
    while(stream->tell() < stream->getSize())
    {
        string line = stream->readLine().strip();
        int comment_start = line.find("//");
        if (comment_start >= 0)
            line = line.substr(0, comment_start);
        if (line == "{")
        {
            //New anonymous node.
            node->child_nodes.emplace_back();
            parseNode(&node->child_nodes.back());
        }
        else if (line.startswith("[") && line.find("]") > -1 && line.endswith("{"))
        {
            //New named node.
            node->child_nodes.emplace_back();
            node->child_nodes.back().id = line.substr(1, line.find("]"));
            parseNode(&node->child_nodes.back());
        }
        else if (line == "}")
        {
            return;
        }
        else if (line.startswith("#"))
        {
            //Comment line.
        }
        else if (line.find(":") > 0)
        {
            while(line.endswith("\\"))
                line = line.substr(0, -1) + "\n" + stream->readLine().strip();
            string key = line.substr(0, line.find(":")).strip();
            string value = line.substr(line.find(":") + 1).strip();
            node->items[key] = value;
        }
        else if (line.length() > 0)
        {
            throw ParsingException("Failed to parse line: " + line);
        }
    }
}

};//namespace io
};//namespace sp
