#include "Live2DConsole.h"

void CubismPrintMessage_Impl(const char* msg)
{
	Console::log(msg);
}

void CubismLogVerbose_Impl(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Console::pushLog(LogState::Log_Normal, fmt, ap);
	va_end(ap);
}

void CubismLogDebug_Impl(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Console::pushLog(LogState::Log_Normal, fmt, ap);
	va_end(ap);
}

void CubismLogInfo_Impl(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Console::pushLog(LogState::Log_Normal, fmt, ap);
	va_end(ap);
}

void CubismLogWarning_Impl(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Console::pushLog(LogState::Log_Warning , fmt, ap);
	va_end(ap);
}

void CubismLogError_Impl(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	Console::pushLog(LogState::Log_Error, fmt, ap);
	va_end(ap);
}
