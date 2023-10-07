#pragma once

#include "Unit.h"

enum struct LogState : unsigned char
{
	Log_Normal,
	Log_Warning,
	Log_Error
};

struct LogItem
{
	LogState state;
	Time timeStamp;
	string text;
};

class ENGINE_API IConsoleBackend
{
public:
	virtual ~IConsoleBackend() = default;
	
	virtual void open(const string& path) = 0;
	virtual void write(const LogItem& item) = 0;
	virtual void close() = 0;
};

class ENGINE_API Console
{
public:
	static list<LogItem> logs;
	static list<LogItem> pyLogs;
	static unsigned int maxLog;
	static string Console::PYLOGBUF;
	static string Console::PYERRBUF;
	static vector<shared_ptr<IConsoleBackend>> backends;

	static map<string, Timer> timers;

	static void pushLog(list<LogItem>& buf, LogState state, const std::string fmt_str, va_list ap, unsigned int maxLog);
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

	static void setMaxLog(unsigned int maxLog);
	static void resetNewLogCount();
	static unsigned int getNewLogCount();
	static void resetNewPyLogCount();
	static unsigned int getNewPyLogCount();
protected:
	static mutex lock;
	static unsigned int newLogNum;
	static unsigned int newPyLogNum;
};
