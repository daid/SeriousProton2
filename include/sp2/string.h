#ifndef SP2_STRING_H
#define SP2_STRING_H

#include <stdlib.h>
#include <string.h>
#include <string>
#include <limits>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>

namespace sp {
/*
    The improved string class. while this class is not always the most efficient in terms of execution speed.
    It does provide a lot of extra functions which makes it easier to work with strings.
    It implements the same API as python strings where possible. However, python strings are immutable, while C++ strings are mutable.
*/
#define _WHITESPACE " \n\r\t"
class string : public std::string
{
public:
    string() : std::string() {}
    string(const std::string& str) : std::string(str) {}
    string(const char* str) : std::string(str) {}
    string(const char* str, int length) : std::string(str, length) {}

    explicit string(const char c) : std::string()
    {
        push_back(c);
    }

    explicit string(const int nr) : std::string()
    {
        std::ostringstream stream;
        stream << nr;
        *this = stream.str();
    }

    explicit string(const unsigned int nr) : std::string()
    {
        std::ostringstream stream;
        stream << nr;
        *this = stream.str();
    }

    explicit string(const uint64_t nr) : std::string()
    {
        std::ostringstream stream;
        stream << nr;
        *this = stream.str();
    }

    explicit string(const float nr, int decimals = 2) : std::string()
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(decimals);
        stream << nr;
        *this = stream.str();
    }

    explicit string(const double nr, int decimals = 2) : std::string()
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(decimals);
        stream << nr;
        *this = stream.str();
    }

    static string hex(const int nr, const int minimal_digits = 0)
    {
        std::ostringstream stream;
        stream << std::hex << std::setfill('0') << std::setw(minimal_digits) << nr;
        return stream.str();
    }

    /*
        substr works the same as the [start:end] operator in python, allowing negative indexes to get the back of the string.
        It is also garanteed to be safe. So if you request an out of range index, you will get an empty string.
    */
    string substr(const int pos = 0, const int endpos = std::numeric_limits<int>::max()) const
    {
        int start = pos;
        int end = endpos;
        int len = length();
        if (start < 0)
            start = len + start;
        if (end < 0)
            end = len + end;
        if (start < 0)
        {
            end += start;
            start = 0;
        }
        len = std::min(end, len);
        if (end <= start || start >= len)
        {
            return "";
        }
        return std::string::substr(start, end - start);
    }

    string operator*(const int count)
    {
        if (count <= 0)
            return "";
        string ret;
        for(int n=0; n<count; n++)
            ret += *this;
        return ret;
    }

    /*
        Return a copy of the string S with only its first character capitalized.
    */
    string capitalize()
    {
        return substr(0, 1).upper() + substr(1).lower();
    }

    /*
        Return S centered in a string of length width. Padding is done using the specified fill character (default is a space)
    */
    string center(const int width, const char fillchar=' ') const
    {
        if (width < int(length()))
            return *this;
        int right = width - length();
        int left = right / 2;
        right -= left;
        return string(fillchar) * left + *this + string(fillchar) * right;
    }

    /*
        Return the number of non-overlapping occurrences of substring sub in
        string S[start:end].  Optional arguments start and end are interpreted
        as in slice notation.
    */
    int count(const string& sub) const
    {
        if (length() < sub.length())
            return 0;
        int count = 0;
        for(unsigned int n=0; n<=length() - sub.length(); n++)
        {
            if (substr(n, n + sub.length()) == sub)
                count++;
        }
        return count;
    }

    /*
        Return True if S ends with the specified suffix, False otherwise.
        With optional start, test S beginning at that position.
        With optional end, stop comparing S at that position.
    */
    bool endswith(const string& suffix) const
    {
        if (suffix.length() == 0)
            return true;
        return substr(-suffix.length()) == suffix;
    }

    /*
        Return a copy of S where all tab characters are expanded using spaces.
        If tabsize is not given, a tab size of 8 characters is assumed.
    */
    string expandtabs(const int tabsize=8) const
    {
        string ret = "";
        int p = 0;
        int t;
        int start = 0;
        int end = find("\r");
        if (find("\n") > -1 && (end == -1 || find("\n") < end))
            end = find("\n");
        while((t = find("\t", p)) > -1)
        {
            while(end != -1 && end < t)
            {
                start = end + 1;
                end = find("\r", start);
                if (find("\n", start) > -1 && (end == -1 || find("\n", start) < end))
                    end = find("\n", start);
            }
            ret += substr(p, t) + string(" ") * (tabsize - ((t - start) % tabsize));
            p = t + 1;
        }
        ret += substr(p);
        return ret;
    }

    /*
        Return the lowest index in S where substring sub is found,
        such that sub is contained within s[start:end].  Optional
        arguments start and end are interpreted as in slice notation.
    */
    int find(const string& sub, int start=0) const
    {
        if (start < 0)
            start = length() + start;
        if (sub.length() + start > length())
            return -1;
        if (sub.length() < 1)
            return start;
        const char* ptr = ::strstr(c_str() + start, sub.c_str());
        if (!ptr)
            return -1;
        return int(ptr - c_str());
    }

    /*
        Return a formatted version of S
    */
    string format(const std::map<string, string>& mapping) const
    {
        string ret;

        //Reserve the target String to the current length plus the length of all the parameters.
        //Which should be a good rough estimate for the final UnicodeString length.
        int itemslength = 0;
        for(auto it : mapping)
        {
            itemslength += it.second.length();
        }
        ret.reserve(length() + itemslength);

        //Run through the source String, find matching brackets.
        for(unsigned int n=0; n<length(); n++)
        {
            char c = this->operator[](n);
            if (c == '{')
            {
                unsigned int end = n;
                while(end < length() && at(end) != '}')
                {
                    end++;
                }
                string key = substr(n + 1, end);
                if (mapping.find(key) != mapping.end())
                {
                    ret += mapping.find(key)->second;
                }

                n = end;
            }
            else if (c == '\\')
            {
                n++;
                ret.push_back(this->operator[](n));
            }
            else
            {
                ret.push_back(c);
            }
        }
        return ret;
    }
    string format(string(*mapping)(const string&)) const
    {
        string ret;

        ret.reserve(length());

        //Run through the source String, find matching brackets.
        for(unsigned int n=0; n<length(); n++)
        {
            char c = this->operator[](n);
            if (c == '{')
            {
                unsigned int end = n;
                while(end < length() && at(end) != '}')
                {
                    end++;
                }
                string key = substr(n + 1, end);
                ret += mapping(key);

                n = end;
            }
            else if (c == '\\')
            {
                n++;
                ret.push_back(this->operator[](n));
            }
            else
            {
                ret.push_back(c);
            }
        }
        return ret;
    }

    /*
        Return True if all characters in S are alphanumeric
        and there is at least one character in S, False otherwise.
    */
    bool isalnum() const
    {
        int count = 0;
        for(unsigned int n=0; n<length(); n++)
        {
            if (!::isalnum((*this)[n]))
                return false;
            count++;
        }
        return count > 0;
    }

    /*
        Return True if all characters in S are alphabetic
        and there is at least one character in S, False otherwise.
    */
    bool isalpha() const
    {
        int count = 0;
        for(unsigned int n=0; n<length(); n++)
        {
            if (!::isalpha((*this)[n]))
                return false;
            count++;
        }
        return count > 0;
    }

    /*
        Return True if all characters in S are digits
        and there is at least one character in S, False otherwise.
    */
    bool isdigit() const
    {
        int count = 0;
        for(unsigned int n=0; n<length(); n++)
        {
            if (!::isdigit((*this)[n]))
                return false;
            count++;
        }
        return count > 0;
    }

    /*
        Return True if all cased characters in S are lowercase and there is
        at least one cased character in S, False otherwise.
    */
    bool islower() const
    {
        int count = 0;
        for(unsigned int n=0; n<length(); n++)
        {
            if ((*this)[n] == '\n')
                continue;
            if (!::islower((*this)[n]))
                return false;
            count++;
        }
        return count > 0;
    }

    /*
        Return True if all characters in S are whitespace
        and there is at least one character in S, False otherwise.
    */
    bool isspace() const
    {
        int count = 0;
        for(unsigned int n=0; n<length(); n++)
        {
            if (!::isspace((*this)[n]))
                return false;
            count++;
        }
        return count > 0;
    }


    /*
        Return True if S is a titlecased string and there is at least one
        character in S, i.e. uppercase characters may only follow uncased
        characters and lowercase characters only cased ones. Return False
        otherwise.
    */
    bool istitle() const
    {
        int count = 0;
        bool need_upper = true;
        for(auto c : *this)
        {
            if (::isalpha(c))
            {
                if (bool(::isupper(c)) != need_upper)
                    return false;
                need_upper = false;
                count += 1;
            }else{
                need_upper = true;
            }
        }
        return count > 0;
    }

    /*
        Return True if all cased characters in S are uppercase and there is
        at least one cased character in S, False otherwise.
    */
    bool isupper() const
    {
        int count = 0;
        for(unsigned int n=0; n<length(); n++)
        {
            if ((*this)[n] == '\n')
                continue;
            if (!::isupper((*this)[n]))
                return false;
            count++;
        }
        return count > 0;
    }


    /*
        Return a string which is the concatenation of the strings in the
        iterable.  The separator between elements is S.
    */
    string join(const std::vector<string>& list) const
    {
        string ret;
        for(unsigned int n=0; n<list.size(); n++)
        {
            if (n > 0)
                ret += *this;
            ret += list[n];
        }
        return ret;
    }

    /*
        Return S left-justified in a string of length width. Padding is
        done using the specified fill character (default is a space).
    */
    string ljust(const int width, const char fillchar=' ') const
    {
        if (int(length()) >= width)
            return *this;
        return *this + string(fillchar) * (width - length());
    }

    /*
        Return a copy of the string S converted to lowercase.
    */
    string lower() const
    {
        string ret = *this;
        for(unsigned int n=0; n<length(); n++)
            ret[n] = tolower(ret[n]);
        return ret;
    }

    /*
        Return a copy of the string S with leading whitespace removed.
        If chars is given and not None, remove characters in chars instead.
    */
    string lstrip(const string& chars=_WHITESPACE) const
    {
        size_t start=0;
        while(start < length() && chars.find(substr(start, start+1)) > -1)
            start++;
        return substr(start);
    }

    /*
        Search for the separator sep in S, and return the part before it, and the part after it.
        If the separator is not found, return S and a empty string.
    */
    std::pair<string, string> partition(const string& sep) const
    {
        int index = find(sep);
        if (index < 0)
        {
            return {*this, ""};
        }
        return {substr(0, index), substr(index + sep.length())};
    }

    /*
        Return a copy of string S with all occurrences of substring
        old replaced by new.  If the optional argument count is
        given, only the first count occurrences are replaced.
    */
    string replace(const string& old, const string& _new, const int count=-1) const
    {
        string result;
        result.reserve(length());
        int start = 0;
        int end = 0;

        if (old.length() < 1)
        {
            if (count == 0)
                return *this;
            result = _new;
            for(int amount=1; amount!=count && start < int(length());amount++)
            {
                result += substr(start, start + 1) + _new;
                start += 1;
            }
            result += substr(start);
            return result;
        }

        for(int amount=0; amount!=count;amount++)
        {
            start = find(old, end);
            if (start < 0)
                break;
            result += substr(end, start) + _new;
            end = start + old.length();
        }
        result += substr(end);
        return result;
    }

    /*
        Return the highest index in S where substring sub is found,
        such that sub is contained within s[start:end].  Optional
        arguments start and end are interpreted as in slice notation.
    */
    int rfind(const string& sub, int start=0) const
    {
        if (sub.length() + start > length())
            return -1;
        for(unsigned int n=length() - sub.length(); int(n)>=start; n--)
        {
            if(substr(n, n+sub.length()) == sub)
                return n;
        }
        return -1;
    }

    /*
        Return S right-justified in a string of length width. Padding is
        done using the specified fill character (default is a space)
    */
    string rjust(const int width, const char fillchar=' ') const
    {
        if (int(length()) >= width)
            return *this;
        return string(fillchar) * (width - length()) + *this;
    }

    /*
        Search for the separator sep in S, starting at the end of S, and return
        the part before it, the separator itand the part after it.  If the
        separator is not found, return two empty strings and S.
    */
    std::vector<string> rpartition(const string& sep) const;

    /*
        Return a list of the words in the string S, using sep as the
        delimiter string, starting at the end of the string and working
        to the front.  If maxsplit is given, at most maxsplit splits are
        done. If sep is not specified or is None, any whitespace string
        is a separator.
    */
    std::vector<string> rsplit(const string& sep=_WHITESPACE, const int maxsplit=-1) const;

    /*
        Return a copy of the string S with trailing whitespace removed.
        If chars is given and not None, remove characters in chars instead.
    */
    string rstrip(const string& chars=_WHITESPACE) const
    {
        int end=length()-1;
        while(end >= 0 && chars.find(substr(end, end+1)) > -1)
            end--;
        return substr(0, end+1);
    }

    /*
        Return a list of the words in the string S, using sep as the
        delimiter string.  If maxsplit is given, at most maxsplit
        splits are done. If sep is not specified or is None, any
        whitespace string is a separator and empty strings are removed
        from the result.
    */
    std::vector<string> split(const string& sep="", int maxsplit=-1) const
    {
        std::vector<string> res;
        int start = 0;
        if(sep == "")
        {
            string cur;
            for(auto c : *this)
            {
                if (::isspace(c) && !cur.empty() && maxsplit != 0)
                {
                    res.emplace_back(std::move(cur));
                    cur.clear();
                    if (maxsplit > 0)
                        maxsplit -= 1;
                }
                else if (!cur.empty() || !::isspace(c))
                {
                    cur.push_back(c);
                }
            }
            if (!cur.empty())
                res.emplace_back(std::move(cur));
            return res;
        }
        while(maxsplit != 0 && start < int(length()))
        {
            int offset = find(sep, start);
            if (offset < 0)
            {
                res.push_back(substr(start));
                return res;
            }
            res.push_back(substr(start, offset));
            start = offset + sep.length();
            if (maxsplit > 0)
                maxsplit--;
        }
        res.push_back(substr(start));
        return res;
    }

    /*
        Return a list of the lines in S, breaking at line boundaries.
        Line breaks are not included in the resulting list unless keepends
        is given and true.
    */
    std::vector<string> splitlines(const bool keepends=false) const;

    /*
        Return True if S starts with the specified prefix, False otherwise.
        With optional start, test S beginning at that position.
        With optional end, stop comparing S at that position.
    */
    bool startswith(const string& prefix) const
    {
        return substr(0, prefix.length()) == prefix;
    }

    /*
        Return a copy of the string S with leading and trailing
        whitespace removed.
        If chars is given and not None, remove characters in chars instead.
    */
    string strip(const string& chars=_WHITESPACE) const
    {
        return lstrip(chars).rstrip(chars);
    }

    /*
        Return a copy of the string S with uppercase characters
        converted to lowercase and vice versa.
    */
    string swapcase() const
    {
        string ret = *this;
        for(unsigned int n=0; n<length(); n++)
            if (::isupper(ret[n]))
                ret[n] = ::tolower(ret[n]);
            else
                ret[n] = ::toupper(ret[n]);
        return ret;
    }

    /*
        Return a titlecased version of S, i.e. words start with uppercase
        characters, all remaining cased characters have lowercase.
    */
    string title() const
    {
        string ret = *this;
        bool needUpper = true;
        for(unsigned int n=0; n<length(); n++)
        {
            if (::isalpha(ret[n]))
            {
                if (needUpper)
                    ret[n] = ::toupper(ret[n]);
                else
                    ret[n] = ::tolower(ret[n]);
                needUpper = false;
            }else{
                needUpper = true;
            }
        }
        return ret;
    }

    /*
        Return a copy of the string S, where all characters occurring
        in the optional argument deletechars are removed, and the
        remaining characters have been mapped through the given
        translation table, which must be a string of length 256.
    */
    string translate(const string& table, const string& deletechars="") const;
    /*
        Return a copy of the string S converted to uppercase.
    */
    string upper() const
    {
        string ret = *this;
        for(unsigned int n=0; n<length(); n++)
            ret[n] = toupper(ret[n]);
        return ret;
    }

    /*
        Pad a numeric string S with zeros on the left, to fill a field
        of the specified width.  The string S is never truncated.
    */
    string zfill(const int width) const
    {
        if (int(length()) > width)
            return *this;
        if ((*this)[0] == '-' || (*this)[0] == '+')
            return substr(0, 1) + string("0") * (width - length()) + substr(1);
        return string("0") * (width - length()) + *this;
    }
};
#undef _WHITESPACE

}//namespace sp

namespace std
{
    //Make a specialization of std::hash for this improved string class which uses the same std::hash as std::string.
    //  (no clue why C++ not decides to use the specialization for the base class)
    template <> struct hash< sp::string>
    {
        std::size_t operator()(const sp::string& k) const
        {
            return hash<std::string>()(k);
        }
    };
}

#endif//STRING_H
