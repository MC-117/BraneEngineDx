#pragma once
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "Unit.h"

class Console
{
public:
	enum LogState
	{
		Log_Normal, Log_Warning, Log_Error
	};

	struct _LOG
	{
		LogState state;
		Time timeStamp;
		string text;
	};

	static list<_LOG> logs;
	static list<_LOG> pyLogs;
	static unsigned int maxLog;
	static stringstream STDOUT;
	static stringstream STDERR;
	static string Console::PYLOGBUF;
	static string Console::PYERRBUF;

	static map<string, Timer> timers;

	Console();
	static void pushLog(list<_LOG>& buf, LogState state, const std::string fmt_str, va_list ap, unsigned int maxLog);
	static void pushLog(LogState state, const std::string fmt_str, va_list ap);
	static void log(const std::string fmt_str, ...);
	static void warn(const std::string fmt_str, ...);
	static void error(const std::string fmt_str, ...);
	static void pyLog(const std::string fmt_str, ...);
	static void pyError(const std::string fmt_str, ...);
	static void pyLogWrite(const std::string str);
	static void pyLogFlush();
	static void pyErrWrite(const std::string str);
	static void pyErrFlush();
	static Timer& getTimer(const string& name);
	static void writeToFile(const string& str);

	static void setMaxLog(unsigned int maxLog);
	static void resetNewLogCount();
	static unsigned int getNewLogCount();
	static void resetNewPyLogCount();
	static unsigned int getNewPyLogCount();
protected:
	static Console console;
	static unsigned int newLogNum;
	static unsigned int newPyLogNum;
};

#endif // !_CONSOLE_H_
