#include "shock_logger.hpp"

namespace fs = std::filesystem;

ShockLogger::ShockLogger(LogLevel level) {
	_level = level;
#ifdef WIN32
	const std::string appDataDir = _getAppDataDir();
	_directory = appDataDir + "\\shock_sender";
	if (!fs::exists(_directory)) {
		fs::create_directory(_directory);
	}

#else

#endif
}

const std::string ShockLogger::_getLevelString(LogLevel levelToLog) {
	switch (levelToLog) {
		case LogLevel::INFO:
			return "[INFO]";
		case LogLevel::WARN:
			return "[WARN]";
		case LogLevel::ERR:
			return "[ERR]";
		default:
			return "[DEBUG]";
	}
}

void ShockLogger::_createNewFileIfTooBig(const std::string& filePath) {
	uintmax_t size = fs::file_size(filePath);
	if (size >= 5 * 1000 * 1000) {
		const std::string oldLogFilePath = filePath + ".old";
		if (fs::exists(oldLogFilePath)) {
			fs::remove(oldLogFilePath);
		}
		fs::rename(filePath, oldLogFilePath);
	}
}

void ShockLogger::_log(const std::string& message, LogLevel levelToLog) {
	if (levelToLog < _level)
		return;

	const std::string fileName =
		_directory + _pathDivider + (levelToLog == LogLevel::ERR ? "error.log" : "app.log");

	_createNewFileIfTooBig(fileName);

	auto const currentTime = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto formattedTime = std::format("{:%Y-%m-%d %H:%M:%S}", currentTime);

	const std::string levelStr = _getLevelString(levelToLog);

	std::ofstream log(fileName, std::ios::app | std::ios_base::out);
	if (log.bad()) {
		log.close();
		return;
	}

	log << formattedTime << " - " << levelStr << " - " << message << "\n";
	log.close();
}

void ShockLogger::debug(const std::string& message) { _log(message, LogLevel::DEBUG); }

void ShockLogger::info(const std::string& message) { _log(message, LogLevel::INFO); }

void ShockLogger::warn(const std::string& message) { _log(message, LogLevel::WARN); }

void ShockLogger::err(const std::string& message) { _log(message, LogLevel::ERR); }