#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <string>

const int _OUTPUT_DEBUG_STRING_LENGTH = 1024;

#define TEST_LOG Log

static int __stdcall OutputDebugF(const LPCTSTR fmt, ...)
{
	va_list Args;
	va_start(Args, fmt);

	TCHAR charBuffer[_OUTPUT_DEBUG_STRING_LENGTH];
#ifdef UNICODE
	int numCharsWritten = std::vswprintf(charBuffer, _OUTPUT_DEBUG_STRING_LENGTH, fmt, Args);
#else
	int numCharsWritten = std::vsnprintf(charBuffer, _OUTPUT_DEBUG_STRING_LENGTH, fmt, Args);
#endif
	OutputDebugString(charBuffer);

	va_end(Args);
	return numCharsWritten;
}

static int __stdcall Log(const LPCTSTR fmt, ...)
{
	va_list Args;
	va_start(Args, fmt);

	TCHAR charBuffer[_OUTPUT_DEBUG_STRING_LENGTH];
#ifdef UNICODE
	int numCharsWritten = std::vswprintf(charBuffer, _OUTPUT_DEBUG_STRING_LENGTH, fmt, Args);
#else
	int numCharsWritten = std::vsnprintf(charBuffer, _OUTPUT_DEBUG_STRING_LENGTH, fmt, Args);
#endif
	OutputDebugF(TEXT("[TEST] %s\n"), charBuffer);

	va_end(Args);
	return numCharsWritten;
}
