#include "shock_logger.hpp"

namespace fs = std::filesystem;

ShockLogger::ShockLogger(LogLevel level) {
	_level = level;
#ifdef WIN32
	const std::string appDataDir = _getAppDataDir();
	_directory = appDataDir + "\\shock_sender";
#else
	const std::string homeDir = getenv("HOME");
	_directory = homeDir + "/.local/share/shock_sender";
#endif
	if (!fs::exists(_directory)) {
		fs::create_directory(_directory);
	}

	_workerThread = std::thread(&ShockLogger::_workerLoop, this);
}

ShockLogger::~ShockLogger() {
	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		_stopWorker = true;
	}
	_condition.notify_one();
	if (_workerThread.joinable()) {
		_workerThread.join();
	}
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
	if (!fs::exists(filePath))
		return;
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
	try {
		if (levelToLog < _level)
			return;

		const std::string fileName =
			_directory + _pathDivider + (levelToLog == LogLevel::ERR ? "error.log" : "app.log");

		_createNewFileIfTooBig(fileName);

		auto const currentTime =
			std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
		auto formattedTime = std::format("{:%Y-%m-%d %H:%M:%S}", currentTime);

		const std::string levelStr = _getLevelString(levelToLog);

		std::ofstream log(fileName, std::ios::app | std::ios_base::out);
		if (log.bad()) {
			log.close();
			return;
		}

		log << formattedTime << " - " << levelStr << " - " << message << "\n";
		log.close();
	} catch (std::exception& e) {
		_writeSystemErrorLog(e.what());
	}
}

void ShockLogger::_addToQueue(const std::string& message, LogLevel levelToLog) {
	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		_logQueue.push({message, levelToLog});
	}
	_condition.notify_one();
}

void ShockLogger::debug(const std::string& message) { _addToQueue(message, LogLevel::DEBUG); }

void ShockLogger::info(const std::string& message) { _addToQueue(message, LogLevel::INFO); }

void ShockLogger::warn(const std::string& message) { _addToQueue(message, LogLevel::WARN); }

void ShockLogger::err(const std::string& message) { _addToQueue(message, LogLevel::ERR); }

void ShockLogger::_writeSystemErrorLog(const std::string& message) {
#ifdef WIN32
	HANDLE eventSource = RegisterEventSourceA(nullptr, "Shock Sender Library");
	const char* messagePtr[1] = {message.c_str()};
	ReportEventA(eventSource, EVENTLOG_ERROR_TYPE, 0, 8008, nullptr, 1, 0, messagePtr, nullptr);
#else
	syslog(LOG_ERR, "%s", message);
#endif
}

void ShockLogger::_workerLoop() {
	while (true) {
		std::pair<std::string, LogLevel> logItem;
		{
			std::unique_lock<std::mutex> lock(_queueMutex);
			_condition.wait(lock, [this]() { return !_logQueue.empty() || _stopWorker; });

			if (_stopWorker && _logQueue.empty())
				return;
			logItem = std::move(_logQueue.front());
			_logQueue.pop();
		}
		_log(logItem.first, logItem.second);
	}
}