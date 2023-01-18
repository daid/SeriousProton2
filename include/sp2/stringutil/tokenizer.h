#ifndef SP2_STRINGUTIL_TOKENIZER_H
#define SP2_STRINGUTIL_TOKENIZER_H

#include <sp2/string.h>

namespace sp::stringutil::tokenizer {
class Token {
public:
    enum class Type {
        Comment,
        String,
        Number,
        Operator,
        Keyword,
        Text,
    };
    Type type = Type::Text;
    size_t start = 0;
    size_t end = 0;
};

class Base {
public:
    class Iterator {
    public:
        const Token& operator*() { return token; }
        void operator++() { tokenizer->next(token); }
        bool operator!=(const Iterator& other) { return token.start < other.token.start; }

    private:
        Iterator(Base* t, size_t offset) : tokenizer(t) { token.start = offset; token.end = offset; }
        Base* tokenizer;
        Token token;

        friend class Base;
    };

    Iterator begin() { auto it = Iterator{this, 0}; ++it; return it; }
    Iterator end() { return {this, s.size()}; }

protected:
    Base(const string& s) : s(s) {}

    virtual void next(Token& t) = 0;
    string s;

    friend class Iterator;
};

}//namespace

#endif//SP2_STRINGUTIL_TOKENIZER_H
