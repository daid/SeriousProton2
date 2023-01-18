#ifndef SP2_STRINGUTIL_LUA_TOKENIZER_H
#define SP2_STRINGUTIL_LUA_TOKENIZER_H

#include <sp2/stringutil/tokenizer.h>
#include <unordered_set>

namespace sp::stringutil::tokenizer {

class Lua : public Base {
public:
    Lua(const string& s);
protected:
    virtual void next(Token& t) override;

    bool isMultilineStringEnd(int index, int equals_count);
    std::unordered_set<sp::string> keywords;
};

}//namespace

#endif//SP2_STRINGUTIL_LUA_TOKENIZER_H
