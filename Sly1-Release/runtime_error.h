#pragma once

void InstallRuntimeErrorHandlers();
void EnsureRuntimeErrorConsole();
void ReportRuntimeError(const char* subsystem, const char* format, ...);
void ReportFatalRuntimeError(const char* message);
