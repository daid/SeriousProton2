#include <sp2/stringutil/luaTokenizer.h>

namespace sp::stringutil::tokenizer {

Lua::Lua(const string& s)
: Base(s), keywords({
        "do", "else", "elseif", "end", "for", "function", "if", "in",
        "local", "repeat", "return", "then", "until", "while", "or", "and", "not"
}) {}

void Lua::next(Token& token)
{
    token.start = token.end;
    token.type = Token::Type::Text;
    //Skip whitespace
    while(token.start < s.size() && strchr(" \t\n\r", s[token.start])) token.start++;
    token.end = token.start;
    if (token.start >= s.size()) return;
    if (s[token.start] == '-' && s[token.start + 1] == '-') {
        token.type = Token::Type::Comment;
        //Comment
        if (s[token.start + 2] == '[') {
            int equals_count = 0;
            while(s[token.start + 3 + equals_count] == '=') equals_count++;
            if (s[token.start + 3 + equals_count] == '[') {
                //Multiline comment
                token.end = token.start + 3 + equals_count;
                while(token.end < s.size() && !isMultilineStringEnd(token.end, equals_count)) token.end++;
                token.end += 2 + equals_count;
                return;
            }
        }
        while(token.end < s.size() && strchr("\n\r", s[token.end]) == nullptr) token.end++;
        return;
    }
    if (s[token.start] == '"') {
        token.type = Token::Type::String;
        token.end++;
        while(token.end < s.size() && s[token.end] != '"') { token.end++; if (s[token.end] == '\\') token.end++; }
        if (token.end < s.size()) token.end++;
        return;
    }
    if (s[token.start] == '\'') {
        token.type = Token::Type::String;
        token.end++;
        while(token.end < s.size() && s[token.end] != '\'') { token.end++; if (s[token.end] == '\\') token.end++; }
        if (token.end < s.size()) token.end++;
        return;
    }
    if (s[token.start] == '[') {
        int equals_count = 0;
        while(s[token.start + 1 + equals_count] == '=') equals_count++;
        if (s[token.start + 1 + equals_count] == '[') {
            //Multiline string
            token.type = Token::Type::String;
            token.end = token.start + 1 + equals_count;
            while(token.end < s.size() && !isMultilineStringEnd(token.end, equals_count)) token.end++;
            token.end += 2 + equals_count;
            return;
        }
    }
    if (!isalnum(s[token.start] & 0x7F) && s[token.start] != '_') {
        token.type = Token::Type::Operator;
        token.end++;
        return;
    }
    if (isdigit(s[token.start] & 0x7F)) {
        token.type = Token::Type::Number;
        token.end = token.start + 1;
        if (s[token.end] == 'x') token.end ++;
        while(token.end < s.size() && (isdigit(s[token.end] & 0x7F) || s[token.end] == '.')) token.end++;
        return;
    }

    //Text
    while(token.end < s.size() && (isalnum(s[token.end] & 0x7F) || s[token.end] == '_')) token.end++;
    auto sub = s.substr(token.start, token.end);
    if (keywords.find(sub) != keywords.end()) token.type = Token::Type::Keyword;
}

bool Lua::isMultilineStringEnd(int index, int equals_count) {
    if (s[index] != ']') return false;
    index++;
    while(equals_count > 0) {
        if (s[index] != '=') return false;
        index++;
        equals_count--;
    }
    if (s[index] != ']') return false;
    return true;
}

}
