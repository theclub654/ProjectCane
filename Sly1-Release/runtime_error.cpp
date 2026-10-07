#include "runtime_error.h"

#include <Windows.h>
#include <cstdarg>
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
}

void EnsureRuntimeErrorConsole()
{
	std::lock_guard<std::mutex> consoleLock(s_consoleMutex);
	if (s_consoleInitialized)
		return;
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
	s_consoleInitialized = true;
}

void ReportRuntimeError(const char* subsystem, const char* format, ...)
{
	EnsureRuntimeErrorConsole();
	std::lock_guard<std::mutex> lock(s_errorMutex);

	SYSTEMTIME time{};
	GetLocalTime(&time);
	std::fprintf(stderr, "[%02u:%02u:%02u.%03u] [ERROR] [%s] ",
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
		subsystem != nullptr ? subsystem : "Runtime");

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
	MessageBoxW(nullptr,
		L"ProjectCane could not continue.\n\n"
		L"The error details were printed in the diagnostic console.",
		L"ProjectCane - Fatal Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}

void InstallRuntimeErrorHandlers()
{
	SetUnhandledExceptionFilter(RuntimeUnhandledExceptionFilter);
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
