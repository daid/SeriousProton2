#ifndef SP2_STRINGUTIL_I18N_H
#define SP2_STRINGUTIL_I18N_H

#include <sp2/string.h>
#include <unordered_map>

namespace sp {

//Translate a string with a loaded translation.
// If no translation was loaded, return the origonal string unmodified.
// There functions are not in the i18n namespace to prevent very long identifiers.
const string& tr(const string& input);
const string& tr(const string& context, const string& input);

namespace i18n {

//Load a translation file.
//  This can be a gettext .po or .mo file.
//  Multiple files can be loaded and the translations will be merged.
//  Returns true if loading was succesful.
bool load(const string& resource_name);
void reset();

class Catalogue
{
public:
    bool load(const string& resource_name);

    const string& tr(const string& input);
    const string& tr(const string& context, const string& input);
private:
    std::unordered_map<string, string> entries;
    std::unordered_map<string, std::unordered_map<string, string>> context_entries;
};

}//!namespace i18n
}//!namespace sp

#endif//SP2_STRINGUTIL_I18N_H
