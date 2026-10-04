#pragma once
#include <atomic>
#include <cerrno>
#include <functional>

#include "platform.h"
#ifndef _WIN32
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#endif
namespace cane {
using LogFunction = std::function<void(const std::string&)>;
#ifdef _WIN32
inline std::wstring quoteWindows(const std::wstring& argument) {
    std::wstring result = L"\"";
    size_t slashes = 0;
    for (auto c : argument) {
        if (c == L'\\') {
            ++slashes;
            continue;
        }
        if (c == L'"') {
            result.append(slashes * 2 + 1, L'\\');
            result += c;
        } else {
            result.append(slashes, L'\\');
            result += c;
        }
        slashes = 0;
    }
    result.append(slashes * 2, L'\\');
    return result + L'"';
}
#endif
inline int runProcess(const std::vector<std::string>& arguments, const fs::path& directory,
                      const std::atomic<bool>& cancelled, const LogFunction& log) {
    if (arguments.empty()) throw std::runtime_error("Empty process command");
    std::string pending;
    auto consume = [&](const char* bytes, size_t count) {
        pending.append(bytes, count);
        size_t end;
        while ((end = pending.find('\n')) != std::string::npos) {
            auto line = pending.substr(0, end);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            log(line);
            pending.erase(0, end + 1);
        }
        if (pending.size() > 65536) {
            log(pending);
            pending.clear();
        }
    };
    int code = 1;
#ifdef _WIN32
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE reader = nullptr, writer = nullptr;
    if (!CreatePipe(&reader, &writer, &security, 0))
        throw std::runtime_error("Cannot create process output pipe");
    SetHandleInformation(reader, HANDLE_FLAG_INHERIT, 0);
    auto nullInput = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 &security, OPEN_EXISTING, 0, nullptr);
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = nullInput;
    startup.hStdOutput = writer;
    startup.hStdError = writer;
    std::wstring command;
    for (auto& a : arguments) {
        if (!command.empty()) command += L' ';
        command += quoteWindows(path(a).wstring());
    }
    PROCESS_INFORMATION process{};
    auto job = CreateJobObjectW(nullptr, nullptr);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    bool ready = job && SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits,
                                                sizeof(limits));
    bool created = ready && CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE,
                                           CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr,
                                           directory.empty() ? nullptr : directory.c_str(),
                                           &startup, &process);
    CloseHandle(writer);
    CloseHandle(nullInput);
    if (!created) {
        CloseHandle(reader);
        if (job) CloseHandle(job);
        throw std::runtime_error("Cannot start " + arguments[0] + " (Windows error " +
                                 std::to_string(GetLastError()) + ")");
    }
    if (!AssignProcessToJobObject(job, process.hProcess)) {
        TerminateProcess(process.hProcess, 1);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        CloseHandle(job);
        CloseHandle(reader);
        throw std::runtime_error("Cannot supervise process");
    }
    ResumeThread(process.hThread);
    CloseHandle(process.hThread);
    bool terminated = false;
    char buffer[8192];
    for (;;) {
        if (cancelled && !terminated) {
            TerminateJobObject(job, 1);
            terminated = true;
        }
        DWORD available = 0;
        if (PeekNamedPipe(reader, nullptr, 0, nullptr, &available, nullptr) && available) {
            DWORD n = 0;
            if (ReadFile(reader, buffer, std::min<DWORD>(available, sizeof(buffer)), &n, nullptr) &&
                n)
                consume(buffer, n);
            continue;
        }
        if (WaitForSingleObject(process.hProcess, 50) == WAIT_OBJECT_0) {
            DWORD result = 1;
            GetExitCodeProcess(process.hProcess, &result);
            code = static_cast<int>(result);
            break;
        }
    }
    // Closing the job also stops descendants that outlive the root process,
    // so they cannot keep the output pipe open indefinitely.
    CloseHandle(job);
    DWORD n = 0;
    while (ReadFile(reader, buffer, sizeof(buffer), &n, nullptr) && n) consume(buffer, n);
    CloseHandle(process.hProcess);
    CloseHandle(reader);
#else
    int descriptors[2];
    if (pipe(descriptors) != 0) throw std::runtime_error("Cannot create process pipe");
    // Prepare argv before fork: the launcher has other threads.
    std::vector<char*> argv;
    for (auto& a : arguments) argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);
    const std::string cwd = utf8(directory);
    const pid_t parentPid = getpid();
    pid_t pid = fork();
    if (pid < 0) {
        close(descriptors[0]);
        close(descriptors[1]);
        throw std::runtime_error("Cannot create process");
    }
    if (pid == 0) {
#ifdef __linux__
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        if (getppid() != parentPid) _exit(125);
#endif
        setpgid(0, 0);
        close(descriptors[0]);
        dup2(descriptors[1], STDOUT_FILENO);
        dup2(descriptors[1], STDERR_FILENO);
        close(descriptors[1]);
        int input = open("/dev/null", O_RDONLY);
        if (input >= 0) {
            dup2(input, STDIN_FILENO);
            close(input);
        }
        if (!cwd.empty() && chdir(cwd.c_str()) != 0) _exit(126);
        execvp(argv[0], argv.data());
        const char error[] = "Could not execute requested program\n";
        write(STDERR_FILENO, error, sizeof(error) - 1);
        _exit(127);
    }
    close(descriptors[1]);
    setpgid(pid, pid);
    fcntl(descriptors[0], F_SETFL, O_NONBLOCK);
    bool exited = false, eof = false, terminated = false;
    int status = 0;
    auto deadline = std::chrono::steady_clock::time_point::max();
    char buffer[8192];
    while (!exited || !eof) {
        if (cancelled && !terminated) {
            kill(-pid, SIGTERM);
            terminated = true;
            deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
        }
        if (terminated && !eof && std::chrono::steady_clock::now() > deadline) kill(-pid, SIGKILL);
        pollfd fd{descriptors[0], POLLIN | POLLHUP, 0};
        poll(&fd, 1, 50);
        for (;;) {
            auto n = read(descriptors[0], buffer, sizeof(buffer));
            if (n > 0)
                consume(buffer, n);
            else {
                if (n == 0) eof = true;
                break;
            }
        }
        if (!exited) {
            auto result = waitpid(pid, &status, WNOHANG);
            if (result == pid)
                exited = true;
            else if (result < 0 && errno != EINTR) {
                exited = true;
                status = 1 << 8;
            }
        }
    }
    close(descriptors[0]);
    code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
#endif
    if (!pending.empty()) log(pending);
    if (cancelled) throw std::runtime_error("Cancelled");
    return code;
}
inline void checkedProcess(const std::vector<std::string>& arguments, const fs::path& directory,
                           const std::atomic<bool>& cancelled, const LogFunction& log) {
    auto code = runProcess(arguments, directory, cancelled, log);
    if (code)
        throw std::runtime_error(arguments[0] + " failed (exit " + std::to_string(code) +
                                 "). See the log for details.");
}
inline void openFolder(const fs::path& directory) {
#ifdef _WIN32
    auto result = reinterpret_cast<intptr_t>(
        ShellExecuteW(nullptr, L"open", directory.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
    if (result <= 32) throw std::runtime_error("Could not open folder");
#else
    auto pid = fork();
    if (pid == 0) {
        auto second = fork();
        if (second > 0) _exit(0);
        if (second < 0) _exit(1);
        setsid();
        auto text = utf8(directory);
        execlp("xdg-open", "xdg-open", text.c_str(), nullptr);
        _exit(127);
    }
    if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
#endif
}
}  // namespace cane
