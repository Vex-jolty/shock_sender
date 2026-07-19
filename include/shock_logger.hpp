#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#ifdef WIN32
	#include <shlobj.h>
	#include <shlwapi.h>
	#include <knownfolders.h>
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
	#include <syslog.h>
#endif

enum class LogLevel {
	DEBUG = 0,
	INFO,
	WARN,
	ERR,
};

class ShockLogger {
	public:
		ShockLogger(LogLevel level);
		~ShockLogger();
		void debug(const std::string& message);
		void info(const std::string& message);
		void warn(const std::string& message);
		void err(const std::string& message);

	private:
		std::string _directory;
		LogLevel _level;
		void _addToQueue(const std::string& message, LogLevel levelToLog);
		void _log(const std::string& message, LogLevel levelToLog);
		const std::string _getLevelString(LogLevel levelToLog);
		void _createNewFileIfTooBig(const std::string& filePath);
		void _writeSystemErrorLog(const std::string& message);
		void _workerLoop();
		std::mutex _queueMutex;
		std::queue<std::pair<std::string, LogLevel>> _logQueue;
		std::condition_variable _condition;
		std::atomic<bool> _stopWorker{false};
		std::thread _workerThread;
#ifdef WIN32
		const std::string _pathDivider = "\\";
		std::string _getAppDataDir() {
			wchar_t* path;
			SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
			std::wstring pathStr(path);
			int sizeNeeded = WideCharToMultiByte(
				CP_UTF8,
				0,
				&pathStr[0],
				(int)pathStr.size(),
				NULL,
				0,
				NULL,
				NULL
			);
			std::string str(sizeNeeded, 0);
			WideCharToMultiByte(
				CP_UTF8,
				0,
				&pathStr[0],
				(int)pathStr.size(),
				&str[0],
				sizeNeeded,
				NULL,
				NULL
			);
			return str;
		}
#else
		const std::string _pathDivider = "/";
#endif
};