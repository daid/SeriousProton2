#include <sp2/io/subprocess.h>
#include <sp2/logging.h>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace sp {
namespace io {

class Subprocess::Data
{
public:
#ifdef _WIN32
    HANDLE handle;
    int pid;
#endif
#ifdef __linux__
    pid_t pid;
#endif
};

Subprocess::Subprocess(std::vector<string> command, const string& working_directory)
{
    data = nullptr;

#ifdef _WIN32
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_information;
    
    memset(&startup_info, 0, sizeof(startup_info));
    memset(&process_information, 0, sizeof(process_information));
    
    startup_info.cb = sizeof(startup_info);
    
    string command_string;
    for(string& part : command)
    {
        if (command_string.length() > 0)
            command_string += " ";
        //TODO: This requires more escaping.
        command_string += "\"" + part + "\"";
    }
    
    char command_string_buffer[command_string.length() + 1];
    strcpy(command_string_buffer, command_string.c_str());
    const char* working_directory_ptr = nullptr;
    if (working_directory.length() > 0)
        working_directory_ptr = working_directory.c_str();
    bool success = CreateProcess(nullptr, command_string_buffer, nullptr, nullptr, false, 0, nullptr, working_directory_ptr, &startup_info, &process_information);
    if (success)
    {
        data = new Data();
        data->handle = process_information.hProcess;
        data->pid = process_information.dwProcessId;
    }
    else
    {
        LOG(Warning, "Failed to start:", command);
    }
#endif//_WIN32
#ifdef __linux__
    pid_t pid = fork();
    if (pid == 0)   //Child process.
    {
        //closefrom(3);
        if (working_directory.length() > 0)
        {
            if (chdir(working_directory.c_str()))
                LOG(Warning, "Failed to change to directory:", working_directory);
        }

        char* parameters[command.size() + 1];
        for(unsigned int n=0; n<command.size(); n++)
            parameters[n] = const_cast<char*>(command[n].c_str());
        parameters[command.size()] = nullptr;
        execvp(command[0].c_str(), parameters);
        LOG(Warning, "Failed to start:", command);
        exit(-1);
    }
    
    if (pid != -1)
    {
        data = new Data();
        data->pid = pid;
    }
#endif//__linux__
}

Subprocess::~Subprocess()
{
    kill(true);
    wait();
}

int Subprocess::wait()
{
    if (!data)
        return std::numeric_limits<int>::min();

#ifdef _WIN32
    WaitForSingleObject(data->handle, INFINITE);
    DWORD exit_status;
    if (!GetExitCodeProcess(data->handle, &exit_status))
        exit_status = -1;
    CloseHandle(data->handle);
    delete data;
    data = nullptr;
    return exit_status;
#elif defined(__linux__)
    int exit_status;
    waitpid(data->pid, &exit_status, 0);
    delete data;
    data = nullptr;
    return WEXITSTATUS(exit_status);
#else
    return std::numeric_limits<int>::min();
#endif
}

#ifdef _WIN32
static BOOL CALLBACK terminateApplicationWindows(HWND hwnd, LPARAM param)
{
    DWORD id;
    GetWindowThreadProcessId(hwnd, &id);
    if(id == static_cast<DWORD>(param))
        PostMessage(hwnd, WM_CLOSE, 0, 0) ;
    return true;
}
#endif//_WIN32

int Subprocess::kill(bool forcefuly)
{
    if (!data)
        return std::numeric_limits<int>::min();
#ifdef _WIN32
    if (forcefuly)
        TerminateProcess(data->handle, -1);
    else
        EnumWindows(terminateApplicationWindows, static_cast<LPARAM>(data->pid));
#endif//_WIN32
#ifdef __linux__
    if (forcefuly)
        ::kill(data->pid, SIGKILL);
    else
        ::kill(data->pid, SIGTERM);
#endif//__linux__
    return wait();
}

bool Subprocess::isRunning()
{
    if (!data)
        return false;
#ifdef _WIN32
    DWORD exit_status;
    if (!GetExitCodeProcess(data->handle, &exit_status))
        exit_status = -1;
    if (exit_status == STILL_ACTIVE)
        return true;
#endif//_WIN32
#ifdef __linux__
    int exit_status;
    if (::waitpid(data->pid, &exit_status, WNOHANG) == 0)
        return true;
#endif//__linux__
    delete data;
    data = nullptr;

    return false;
}

}//namespace io
}//namespace sp
