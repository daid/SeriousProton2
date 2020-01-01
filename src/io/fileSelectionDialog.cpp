#include <sp2/io/fileSelectionDialog.h>

#if defined(__WIN32__)
#include <windows.h>
#elif defined(__linux__) && !defined(__ANDROID__) && 0
#include <gtk/gtk.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten.h>

extern "C" void sp2fileSelectionCallback(const char* str) __attribute__((used));
static std::function<void(const sp::string&)> sp2_file_selection_callback;
#endif

namespace sp {
namespace io {

void openFileDialog(const sp::string& filter, std::function<void(const sp::string&)> callback)
{
#if defined(__WIN32__)
    sp::string filter_string = "*" + filter + sp::string('\0') + "*" + filter + sp::string('\0');

    OPENFILENAMEA openfilename;
    char filename[MAX_PATH];
    memset(&openfilename, 0, sizeof(openfilename));
    memset(&filename, 0, sizeof(filename));

    openfilename.lStructSize = sizeof(openfilename);
    if (filter != "")
        openfilename.lpstrFilter = filter_string.c_str();
    openfilename.lpstrFile = filename;
    openfilename.nMaxFile = sizeof(filename);
    openfilename.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    GetOpenFileNameA(&openfilename);
    if (strlen(filename) > 0)
        callback(filename);
#elif defined(__linux__) && !defined(__ANDROID__) && 0
    if (!gtk_init_check(nullptr, nullptr))
    {
        return;
    }
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File", nullptr, GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, nullptr);
    
    if (filter != "")
    {
        GtkFileFilter* file_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(file_filter, ("*" + filter).c_str());
		gtk_file_filter_add_pattern(file_filter, ("*" + filter).c_str());
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), file_filter);
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        callback(filename);
        g_free(filename);
    }

	while (gtk_events_pending())
		gtk_main_iteration();
    gtk_widget_destroy(dialog);
	while (gtk_events_pending())
		gtk_main_iteration();
#elif defined(__EMSCRIPTEN__)
    sp2_file_selection_callback = callback;

    //The following javascript code does a bunch of things:
    // * It creates an invisible file selection input field if it does not exists yet.
    // * It opens up the dialog that belongs with the file selection.
    // * If a file is selected, the contents of that file is copied into the virtual filesystem at /upload/
    // * A call is made back to C code to call the callback async.
    EM_ASM({
        if (typeof(sp2_file_input_element) == "undefined")
        {
            sp2_file_input_element = document.createElement('input');
            sp2_file_input_element.type = "file";
            sp2_file_input_element.style.display = "none";
            document.body.appendChild(sp2_file_input_element);

            sp2_file_input_element.addEventListener("change", function()
            {
                var filename = "/upload/" + this.files[0].name;
                var name = this.files[0].name;
                this.files[0].arrayBuffer().then(function(buffer)
                {
                    try { FS.unlink(filename); } catch (exception) { }
                    FS.createDataFile("/upload/", name, new Uint8Array(buffer), true, true, true);
                    var stack = Module.stackSave();
                    var name_ptr = Module.stackAlloc(filename.length * 4 + 1);
                    stringToUTF8(filename, name_ptr, filename.length * 4 + 1);
                    Module._sp2fileSelectionCallback(name_ptr);
                    stackRestore(stack);
                });
            }, false);
            FS.mkdir("/upload");
        }
        sp2_file_input_element.value = "";
        sp2_file_input_element.accept = UTF8ToString($0);
        sp2_file_input_element.click();
    }, filter.c_str());
#endif
}

}//namespace io
}//namespace sp

#ifdef __EMSCRIPTEN__
void sp2fileSelectionCallback(const char* str)
{
    if (sp2_file_selection_callback)
        sp2_file_selection_callback(str);
}
#endif//__EMPSCRIPTEN__
