#include "runtime_error.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <csignal>
#include <ctime>
#endif
#include <cstdarg>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <exception>
#include <iostream>
#include <mutex>

namespace
{
	std::mutex s_consoleMutex;
	bool s_consoleInitialized = false;
	std::mutex s_errorMutex;

	#ifdef _WIN32
	LONG WINAPI RuntimeUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers)
	{
		EnsureRuntimeErrorConsole();
		const DWORD code = exceptionPointers != nullptr && exceptionPointers->ExceptionRecord != nullptr
			? exceptionPointers->ExceptionRecord->ExceptionCode : 0;
		const void* address = exceptionPointers != nullptr && exceptionPointers->ExceptionRecord != nullptr
			? exceptionPointers->ExceptionRecord->ExceptionAddress : nullptr;
		std::fprintf(stderr,
			"\n[FATAL] Unhandled Windows exception 0x%08lX at %p on thread %lu.\n",
			code, address, GetCurrentThreadId());
		std::fprintf(stderr,
			"Please copy this message and describe what was happening when the failure occurred.\n");
		std::fflush(stderr);
		MessageBoxW(nullptr,
			L"ProjectCane encountered a fatal error.\n\n"
			L"The error details were printed in the diagnostic console.",
			L"ProjectCane - Fatal Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return EXCEPTION_CONTINUE_SEARCH;
	}
	#else
	void RuntimeSignalHandler(int signal)
	{
		std::fprintf(stderr, "\n[FATAL] Received signal %d.\n", signal);
		std::fflush(stderr);
		std::signal(signal, SIG_DFL);
		raise(signal);
	}
	#endif
}

void EnsureRuntimeErrorConsole()
{
	std::lock_guard<std::mutex> consoleLock(s_consoleMutex);
	if (s_consoleInitialized)
		return;
	#ifdef _WIN32
	if (GetConsoleWindow() == nullptr && AllocConsole() == FALSE)
		return;

	FILE* stream = nullptr;
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
	std::cout.clear();
	std::cerr.clear();
	SetConsoleTitleW(L"ProjectCane - Runtime Diagnostics");

	if (HWND consoleWindow = GetConsoleWindow())
	{
		ShowWindow(consoleWindow, SW_SHOW);
		SetForegroundWindow(consoleWindow);
	}
	#endif
	s_consoleInitialized = true;
}

void ReportRuntimeError(const char* subsystem, const char* format, ...)
{
	EnsureRuntimeErrorConsole();
	std::lock_guard<std::mutex> lock(s_errorMutex);

	#ifdef _WIN32
	SYSTEMTIME time{};
	GetLocalTime(&time);
	std::fprintf(stderr, "[%02u:%02u:%02u.%03u] [ERROR] [%s] ",
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
		subsystem != nullptr ? subsystem : "Runtime");
	#else
	const auto now = std::chrono::system_clock::now();
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;
	const std::time_t rawTime = std::chrono::system_clock::to_time_t(now);
	std::tm localTime{};
	localtime_r(&rawTime, &localTime);
	std::fprintf(stderr, "[%02d:%02d:%02d.%03lld] [ERROR] [%s] ",
		localTime.tm_hour, localTime.tm_min, localTime.tm_sec,
		static_cast<long long>(milliseconds.count()),
		subsystem != nullptr ? subsystem : "Runtime");
	#endif

	va_list arguments;
	va_start(arguments, format);
	std::vfprintf(stderr, format, arguments);
	va_end(arguments);
	std::fputc('\n', stderr);
	std::fflush(stderr);
}

void ReportFatalRuntimeError(const char* message)
{
	ReportRuntimeError("Fatal", "%s", message != nullptr ? message : "Unknown fatal error");
	#ifdef _WIN32
	MessageBoxW(nullptr,
		L"ProjectCane could not continue.\n\n"
		L"The error details were printed in the diagnostic console.",
		L"ProjectCane - Fatal Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	#endif
}

void InstallRuntimeErrorHandlers()
{
	#ifdef _WIN32
	SetUnhandledExceptionFilter(RuntimeUnhandledExceptionFilter);
	#else
	std::signal(SIGSEGV, RuntimeSignalHandler);
	std::signal(SIGABRT, RuntimeSignalHandler);
	std::signal(SIGFPE, RuntimeSignalHandler);
	std::signal(SIGILL, RuntimeSignalHandler);
	#endif
	std::set_terminate([]
	{
		const char* message = "Unhandled C++ exception on a worker thread";
		try
		{
			if (std::exception_ptr exception = std::current_exception())
				std::rethrow_exception(exception);
		}
		catch (const std::exception& exception)
		{
			ReportFatalRuntimeError(exception.what());
			std::abort();
		}
		catch (...)
		{
		}
		ReportFatalRuntimeError(message);
		std::abort();
	});
}
