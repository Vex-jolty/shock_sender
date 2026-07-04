#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#ifdef WIN32
	#include <shlobj.h>
	#include <shlwapi.h>
	#include <knownfolders.h>
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
		void debug(const std::string& message);
		void info(const std::string& message);
		void warn(const std::string& message);
		void err(const std::string& message);

	private:
		std::string _directory;
		LogLevel _level;
		void _log(const std::string& message, LogLevel levelToLog);
		const std::string _getLevelString(LogLevel levelToLog);
		void _createNewFileIfTooBig(const std::string& filePath);
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