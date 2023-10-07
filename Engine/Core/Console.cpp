#include "Console.h"
#include "InitializationManager.h"
#include <memory>
#include <fstream>

#include "Engine.h"

list<LogItem> Console::logs;
list<LogItem> Console::pyLogs;
unsigned int Console::maxLog = 1000;
string Console::PYLOGBUF;
string Console::PYERRBUF;
vector<shared_ptr<IConsoleBackend>> Console::backends;
map<string, Timer> Console::timers;
mutex Console::lock;
unsigned int Console::newLogNum = 0;
unsigned int Console::newPyLogNum = 0;

class FileConsoleBackend : public IConsoleBackend
{
public:
	string path;

	virtual void open(const string& path) override
	{
		this->path = path;
		ofstream file("log.txt", ios::out | ios::trunc);
		file << "Log file opened\n";
		file.close();
	}
	virtual void write(const LogItem& item) override
	{
		if (path.empty())
			return;
		ofstream file("log.txt", ios::out | ios::app);
		if (!file.is_open())
			return;
		switch (item.state)
		{
		case LogState::Log_Normal:
			file << "[log][";
			break;
		case LogState::Log_Warning:
			file << "[wrn][";
			break;
		case LogState::Log_Error:
			file << "[err][";
			break;
		}
		file << item.timeStamp.toString() << ']' << item.text << endl;
		file.close();
	}
	virtual void close() override
	{
		ofstream file("log.txt", ios::out | ios::app);
		if (!file.is_open())
			return;
		file << "Log file closed\n";
		file.close();
		path.clear();
	}
};

class StandardConsoleBackend : public IConsoleBackend
{
public:
	HANDLE hConsole = NULL;
	virtual void open(const string& path) override
	{
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	virtual void write(const LogItem& item) override
	{
#ifdef WIN32
		switch (item.state)
		{
		case LogState::Log_Normal:
			if (hConsole)
				SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			cout << "[log][";
			break;
		case LogState::Log_Warning:
			if (hConsole)
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			cout << "[wrn][";
			break;
		case LogState::Log_Error:
			if (hConsole)
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			cout << "[err][";
			break;
		}
		cout << item.timeStamp.toString() << ']' << item.text << endl;
#else
		switch (item.state)
		{
		case LogState::Log_Normal:
			cout << "\033[0m[log][";
			break;
		case LogState::Log_Warning:
			cout << "\033[0;33m[wrn][";
			break;
		case LogState::Log_Error:
			cout << "\033[0;31m[err][";
			break;
		}
		cout << item.timeStamp.toString() << ']' << item.text << "\033[0m\n";
#endif
	}
	virtual void close() override
	{
		hConsole = NULL;
	}
};

class ConsoleInitialization : public Initialization
{
public:
	static ConsoleInitialization instance;
	
	ConsoleInitialization()
	: Initialization(InitializeStage::BeforeEngineConfig, -1
		, FinalizeStage::AfterEngineRelease, 1000) { }

	virtual bool initialize()
	{
		string logFilePath;
		Engine::engineConfig.configInfo.get("logFilePath", logFilePath);
		if (logFilePath.empty())
			logFilePath = "log.txt";
		Console::backends.emplace_back(
			make_shared<StandardConsoleBackend>())->open(logFilePath);
		Console::backends.emplace_back(
			make_shared<FileConsoleBackend>())->open(logFilePath);
		return true;
	}

	virtual bool finalize()
	{
		for (auto& backend : Console::backends)
			backend->close();
		Console::backends.clear();
		return true;
	}
};

ConsoleInitialization ConsoleInitialization::instance;

void Console::pushLog(list<LogItem>& buf, LogState state, const std::string fmt_str, va_list ap, unsigned int maxLog)
{
	Time dur = Time::duration();
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::unique_ptr<char[]> formatted;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		memcpy_s(&formatted[0], fmt_str.size(), fmt_str.c_str(), fmt_str.size());
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	while (buf.size() >= maxLog)
		buf.pop_front();
	LogItem& item = buf.emplace_back(LogItem{ state, dur, string(formatted.get()) });
	for (shared_ptr<IConsoleBackend> backend : backends)
		backend->write(item);
}

void Console::pushLog(LogState state, const std::string fmt_str, va_list ap)
{
	pushLog(logs, state, fmt_str, ap, maxLog);
	newLogNum++;
}

void Console::log(const std::string fmt_str, ...)
{
	va_list ap;
	va_start(ap, fmt_str);
	pushLog(logs, LogState::Log_Normal, fmt_str, ap, maxLog);
	va_end(ap);
	newLogNum++;
}

void Console::warn(const std::string fmt_str, ...)
{
	va_list ap;
	va_start(ap, fmt_str);
	pushLog(logs, LogState::Log_Warning, fmt_str, ap, maxLog);
	va_end(ap);
	newLogNum++;
}

void Console::error(const std::string fmt_str, ...)
{
	va_list ap;
	va_start(ap, fmt_str);
	pushLog(logs, LogState::Log_Error, fmt_str, ap, maxLog);
	va_end(ap);
	newLogNum++;
}

void Console::pyLog(const std::string fmt_str, ...)
{
	va_list ap;
	va_start(ap, fmt_str);
	pushLog(pyLogs, LogState::Log_Normal, fmt_str, ap, maxLog);
	va_end(ap);
	newPyLogNum++;
}

void Console::pyError(const std::string fmt_str, ...)
{
	va_list ap;
	va_start(ap, fmt_str);
	pushLog(pyLogs, LogState::Log_Error, fmt_str, ap, maxLog);
	va_end(ap);
	newPyLogNum++;
}

void Console::pyLogWrite(const std::string str)
{
	for (auto b = str.begin(), e = str.end(); b != e; b++) {
		if (*b == '\n')
			pyLogFlush();
		else
			PYLOGBUF += *b;
	}
}

void Console::pyLogFlush()
{
	pyLog(PYLOGBUF.c_str());
	PYLOGBUF.clear();
}

void Console::pyErrWrite(const std::string str)
{
	for (auto b = str.begin(), e = str.end(); b != e; b++) {
		if (*b == '\n')
			pyErrFlush();
		else
			PYERRBUF += *b;
	}
}

void Console::pyErrFlush()
{
	pyError(PYERRBUF.c_str());
	PYERRBUF.clear();
}

Timer & Console::getTimer(const string & name)
{
	lock_guard guard(lock);
	auto iter = timers.find(name);
	if (iter == timers.end())
		return timers.insert(pair<string, Timer>(name, Timer())).first->second;
	return iter->second;
}

void Console::setMaxLog(unsigned int maxLog)
{
	Console::maxLog = maxLog;
	while (logs.size() >= maxLog)
		logs.pop_front();
}

void Console::resetNewLogCount()
{
	newLogNum = 0;
}

unsigned int Console::getNewLogCount()
{
	return newLogNum;
}

void Console::resetNewPyLogCount()
{
	newPyLogNum = 0;
}

unsigned int Console::getNewPyLogCount()
{
	return newPyLogNum;
}
