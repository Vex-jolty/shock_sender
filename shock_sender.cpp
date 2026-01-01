#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/optional.hpp>
#include <boost/json.hpp>
#include <boost/describe.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>
#include <regex>
#include <fstream>
#include <chrono>
#include <thread>
#include "shock_sender.h"
#if defined WIN32
#include <windows.h>
#endif

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
net::io_context ioc;

websocket::stream<tcp::socket> ws{ ioc };

std::string debugLogFile = "debug.log";
std::string errorLogFile = "error.log";

/*
*	These values will be populated upon initialization
*/
#ifdef NDEBUG
std::string wsHost = "broker.pishock.com";
#else
std::string wsHost = "localhost";
#endif
std::string psHost = "ps.pishock.com";
std::string authHost = "auth.pishock.com";
std::string username;
std::string apiKey;
int userId = 0;
int clientId = 0;
std::vector<int> shockerIds;
bool isRunning = false; // Whether the websocket is running

bool userWarnedAboutMaxShock = false; // Ensuring warning isn't displayed with every shock
bool userAuthorizedDangerousValue = false;
bool shockCanBeSent = true;

class L {
public:
	L() = default;
	virtual ~L() = default;

private:
	int u;
	std::string ty;
	bool w;
	bool h;
	std::string o;

public:
	const int& get_u() const { return u; }
	int& get_mutable_u() { return u; }
	void set_u(const int& value) { this->u = value; }

	const std::string& get_ty() const { return ty; }
	std::string& get_mutable_ty() { return ty; }
	void set_ty(const std::string& value) { this->ty = value; }

	const bool& get_w() const { return w; }
	bool& get_mutable_w() { return w; }
	void set_w(const bool& value) { this->w = value; }

	const bool& get_h() const { return h; }
	bool& get_mutable_h() { return h; }
	void set_h(const bool& value) { this->h = value; }

	const std::string& get_o() const { return o; }
	std::string& get_mutable_o() { return o; }
	void set_o(const std::string& value) { this->o = value; }

	BOOST_DESCRIBE_CLASS(L, (), (u, ty, w, h, o), (), ());
};

class BodyT {
public:
	BodyT() = default;
	virtual ~BodyT() = default;

private:
	int id;
	std::string m;
	int i;
	int d;
	bool r;
	L l;

public:
	const int get_id() const { return id; }
	int get_mutable_id() { return id; }
	void set_id(const int value) { this->id = value; }

	const std::string& get_m() const { return m; }
	std::string& get_mutable_m() { return m; }
	void set_m(const std::string& value) { this->m = value; }

	const int& get_i() const { return i; }
	int& get_mutable_i() { return i; }
	void set_i(const int& value) { this->i = value; }

	const int& get_d() const { return d; }
	int& get_mutable_d() { return d; }
	void set_d(const int& value) { this->d = value; }

	const bool& get_r() const { return r; }
	bool& get_mutable_r() { return r; }
	void set_r(const bool& value) { this->r = value; }

	const L& get_l() const { return l; }
	L& get_mutable_l() { return l; }
	void set_l(const L& value) { this->l = value; }

	BOOST_DESCRIBE_CLASS(BodyT, (), (id, m, i, d, r, l), (), ());
};

class PublishCommandT {
public:
	PublishCommandT() = default;
	virtual ~PublishCommandT() = default;

private:
	std::string Target;
	BodyT Body;

public:
	const std::string& get_target() const { return Target; }
	std::string& get_mutable_target() { return Target; }
	void set_target(const std::string& value) { this->Target = value; }

	const BodyT& get_body() const { return Body; }
	BodyT& get_mutable_body() { return Body; }
	void set_body(const BodyT& value) { this->Body = value; }

	BOOST_DESCRIBE_CLASS(PublishCommandT, (), (Target, Body), (), ());
};

class PiShockPayload {
public:
	PiShockPayload() = default;
	virtual ~PiShockPayload() = default;

private:
	std::string Operation;
	std::vector<PublishCommandT> PublishCommands;

public:
	const std::string& get_operation() const { return Operation; }
	std::string& get_mutable_operation() { return Operation; }
	void set_operation(const std::string& value) { this->Operation = value; }

	const std::vector<PublishCommandT>& get_publish_commands() const { return PublishCommands; }
	std::vector<PublishCommandT>& get_mutable_publish_commands() { return PublishCommands; }
	void set_publish_commands(const std::vector<PublishCommandT>& value) { this->PublishCommands = value; }

	BOOST_DESCRIBE_CLASS(PiShockPayload, (), (Operation, PublishCommands), (), ());
};

void logError(std::string message) {
	auto const currentTime = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto formattedTime = std::format("{:%Y-%m-%d %H:%M:%S}", currentTime);
	std::ofstream log(errorLogFile, std::ios::app | std::ios_base::out);
	if (log.bad()) return;
	log << formattedTime << " - " << message << "\n";
	log.close();
}

void logDebug(std::string message) {
	auto const currentTime = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto formattedTime = std::format("{:%Y-%m-%d %H:%M:%S}", currentTime);
	std::ofstream log(debugLogFile, std::ios::app | std::ios_base::out);
	if (log.bad()) return;
	log << formattedTime << " - " << message << "\n";
	log.close();
}

// Warn user about max values above 100
const std::string getWarningMessage(int maxShockValue) {
	std::stringstream ss;
	ss << "Your max shock value is set at " << maxShockValue << "\n" << "This is EXTREMELY DANGEROUS!!\n" << "If this is intentional, make sure to have someone closeby to help you if something goes wrong, along with a first aid kid\n" << "Do you accept the health risks that come with this value, and wish to proceed anyway? If you do not, the game will use a max value of 100 instead";
	return ss.str();
}

const std::string getCredentialsWarningMessage(const std::string& missingKey) {
	return missingKey + " missing from shock_setup.ini file. The mod will not function.";
}

#if WIN32
int showErrorMessage(const std::string& msg, const std::string& type) {
	return MessageBoxA(NULL, msg.c_str(), type == "warn" ? "DANGER" : "ERROR", type == "warn" ? MB_ICONWARNING | MB_YESNO : MB_ICONERROR);
}

void warnAboutMaxShock(int maxShockValue) {
	auto response = showErrorMessage(getWarningMessage(maxShockValue).c_str(), "warn");
	userAuthorizedDangerousValue = (response == IDYES);
	userWarnedAboutMaxShock = true;
}


void warnAboutEmptyValues() {
	if (!username.empty() && !apiKey.empty()) return;
	std::string msg;
	if (username.empty()) {
		msg = getCredentialsWarningMessage("Username");
	} else if (apiKey.empty()) {
		msg = getCredentialsWarningMessage("API Key");
	}
	if (msg.empty()) return;
	showErrorMessage(msg, "err");
}

#elif __linux__
bool commandExists(const char* cmd) {
	std::string s = "command -v ";
	s += cmd;
	s += " >/dev/null 2>&1";
	return std::system(s.c_str()) == 0;
}

const std::string getDialogCommand(const std::string& msg, const std::string& type) {
	std::string cmd;
	if (commandExists("zenity")) {
		cmd = "zenity ";
		cmd += type == "warn" ? "--warning --ok-label=\"Yes\" --extra-button=\"No\"" : "--error";
		cmd += " --text=\"";
		cmd += msg;
		cmd += "\"";
	} else if (commandExists("kdialog")) {
		cmd = "kdialog " + type == "warn" ? "--warningyesno \"" : "--error\"";
		cmd += msg;
		cmd += "\"";
	} else if (commandExists("xmessage")) {
		cmd = "xmessage \"";
		cmd += msg;
		cmd += "\"";
	} else {
		type == "warn" ? fprintf(stderr, "WARNING: %s\n", msg.c_str()) : fprintf(stderr, "ERROR: %s\n", msg.c_str());
		return "";
	}
	return cmd;
}

void showWarningDialog(const std::string& msg) {
	const std::string cmd = getDialogCommand(msg, "warn");
	if (cmd.empty()) return;

	int response = std::system(cmd.c_str());
	userAuthorizedDangerousValue = (response == 0);
}

void warnAboutMaxShock(int maxShockValue) {
	showWarningDialog(getWarningMessage(maxShockValue));
	userWarnedAboutMaxShock = true;
}

void warnAboutEmptyValues() {
	if (!username.empty() && !apiKey.empty()) return;
	std::string msg;
	if (username.empty()) {
		msg = getCredentialsWarningMessage("Username");
	} else if (apiKey.empty()) {
		msg = getCredentialsWarningMessage("API Key");
	}
	if (msg.empty()) return;
	std::string cmd = getDialogCommand(msg, "err");
	std::system(cmd.c_str());
}
#endif

net::awaitable<boost::json::value> sendGetRequest(std::string path, std::string subPath) {
	auto ex = co_await net::this_coro::executor;

	logDebug("Sending GET to " + path + subPath);

	tcp::resolver resolver(ex);
	beast::tcp_stream stream(ex);

	auto results = co_await resolver.async_resolve(
		path,
		"80",
		net::use_awaitable
	);

	co_await stream.async_connect(
		results,
		net::use_awaitable
	);

	http::request<http::string_body> req{
		http::verb::get,
		subPath,
		11
	};
	req.set(http::field::host, path);
	req.set(http::field::user_agent, "beast");

	co_await http::async_write(stream, req, net::use_awaitable);

	beast::flat_buffer buffer;
	http::response<http::string_body> res;

	co_await http::async_read(stream, buffer, res, net::use_awaitable);

	stream.socket().shutdown(tcp::socket::shutdown_both);

	logDebug("HTTP response received");
	auto body = res.body();
	logDebug(body);

	co_return boost::json::parse(body);
}

net::awaitable<boost::json::value> sendGetRequest(std::string path, std::string subPath, std::unordered_map<std::string, std::string> query) {
	std::size_t i = 0;
	for (auto& [key, val] : query) {
		std::string separator = i == 0 ? "?" : "&";
		subPath += separator + key + "=" + val;
		i++;
	}
	co_return co_await sendGetRequest(path, subPath);
}

net::awaitable<void> getUserId() {
	auto result = co_await sendGetRequest(authHost, "/Auth/GetUserIfAPIKeyValid", { {"apiKey", apiKey}, {"username", username} });
	auto& resBody = result.as_object();
	userId = resBody.find("UserId")->value().as_int64();
	logDebug("User id = " + std::to_string(userId));
}

net::awaitable<void> getShockerIds() {
	auto result = co_await sendGetRequest(psHost, "/PiShock/GetUserDevices", { {"UserId", std::to_string(userId)}, {"Token", apiKey}, {"api", "true"} });
	auto& resBody = result.as_array();
	auto& client = resBody[0].as_object();
	clientId = client.find("clientId")->value().as_int64();
	auto shockersArray = client.find("shockers")->value().as_array();
	for (auto& item : shockersArray) {
		auto shockerData = item.as_object();
		int id = shockerData.find("shockerId")->value().as_int64();
		shockerIds.push_back(id);
	}
}

void getUsernameAndApiKey(std::string& pathToFile) {
	std::ifstream configFile(pathToFile);
	if (configFile.bad()) {
		configFile.close();
		return;
	}
	std::string line;
	while (std::getline(configFile, line)) {
		std::vector<std::string> splitLine;
		boost::split(splitLine, line, boost::is_any_of("="));
		std::string& key = splitLine[0];
		std::string& value = splitLine[1];
		if (boost::starts_with(key, "username")) {
			username = value;
		} else if (boost::starts_with(key, "api_key")) {
			apiKey = value;
		}
	}
	configFile.close();
	boost::trim(username);
	boost::trim(apiKey);
}

net::awaitable<void> startWebsocket() {
	logDebug("This is the start websocket function");
	auto executor = co_await net::this_coro::executor;

	tcp::resolver resolver(executor);
	std::string target = "/v2?username=" + username + "&apiKey=" + apiKey;
	auto results = co_await resolver.async_resolve(
		wsHost,
		#ifdef NDEBUG
		"80",
		#else
		"5000",
		#endif
		net::use_awaitable
	);
	co_await net::async_connect(
		ws.next_layer(),
		results,
		net::use_awaitable
	);
	co_await ws.async_handshake(
		wsHost,
		target,
		net::use_awaitable
	);
	isRunning = true;
	logDebug("Websocket is running");
	beast::flat_buffer buffer;
	while (ws.is_open()) {
		std::size_t n = co_await ws.async_read(
			buffer,
			net::use_awaitable
		);
		std::string msg{
			static_cast<char const*>(buffer.data().data()),
			n
		};
		buffer.consume(n);
		logDebug("received " + msg);
	}
}

net::awaitable<void> initializeClient() {
	co_await getUserId();
	co_await getShockerIds();
	co_await startWebsocket();
}

void startLibrary() {
	net::co_spawn(ioc, initializeClient(), [&](std::exception_ptr e) {
		if (e) {
			try { std::rethrow_exception(e); } catch (const std::exception& ex) {
				logError(ex.what());
			}
		}
		});
	ioc.run();
	logDebug("Connection loop broken");
}

void checkMaxShockOk(std::string pathToFile) {
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::ifstream configFile(pathToFile);
	std::string line;
	while (std::getline(configFile, line)) {
		if (boost::starts_with(line, "max_shock")) break;
	}
	configFile.close();
	boost::trim(line);
	std::vector<std::string> splitLine;
	boost::split(splitLine, line, boost::is_any_of("="));
	int value = std::stoi(splitLine[1]);
	if (value > 100) warnAboutMaxShock(value);
}

int run(char* filePath) {
	std::string pathToFile(filePath);
	logDebug("File path " + pathToFile);
	std::jthread checkThread(checkMaxShockOk, pathToFile);
	checkThread.detach();
	getUsernameAndApiKey(pathToFile);
	warnAboutEmptyValues();
	logDebug("Username " + username);
	logDebug("API key " + apiKey);
	std::jthread ioThread(startLibrary);

	return 0;
}

net::awaitable<void> sendMessage(std::string message) {
	ws.text(true);
	co_await ws.async_write(net::buffer(message), net::use_awaitable);
}

MaxShockAndIntensityPerQuarter getMaxShockAndIntensityPerQuarter() {
	std::ifstream configFile("shock_config.ini");
	int maxShock = 100;
	int intensityPerQuarter = 0;
	int intensityPerHealth = 0;
	int durationMilliseconds = 0;
	if (configFile.bad()) {
		configFile.close();
		return MaxShockAndIntensityPerQuarter{
			maxShock,
			4,
			100,
		};
	}
	std::string line;
	while (std::getline(configFile, line)) {
		std::vector<std::string> splitLine;
		boost::split(splitLine, line, boost::is_any_of("="));
		std::string& key = splitLine[0];
		std::string& value = splitLine[1];
		bool isValid = false;
		for (auto& c : value) {
			isValid = std::isdigit(c);
		}
		if (!isValid) continue;
		if (boost::starts_with(key, "max_shock")) {
			maxShock = std::stoi(value);
		} else if (boost::starts_with(key, "intensity_per_quarter")) {
			intensityPerQuarter = std::stoi(value);
		} else if (boost::starts_with(key, "intensity_per_health")) {
			intensityPerHealth = std::stoi(value);
		} else if (boost::starts_with(key, "duration_ms")) {
			durationMilliseconds = std::stoi(value);
		}
	}
	configFile.close();
	if (intensityPerQuarter == 0) {
		intensityPerQuarter = maxShock / 4;
	}
	return MaxShockAndIntensityPerQuarter{
		maxShock > 100 && !userAuthorizedDangerousValue ? 100 : maxShock,
		intensityPerQuarter <= 0 ? 1 : intensityPerQuarter,
		intensityPerHealth <= 0 ? 1 : intensityPerHealth,
		durationMilliseconds <= 0 ? 100 : durationMilliseconds
	};
}

int sendShock(int amount, bool useQuarters) {
	if (clientId == 0 || !isRunning) return 1;
	MaxShockAndIntensityPerQuarter maxShockAndIntensityPerQuarter = getMaxShockAndIntensityPerQuarter();
	int limit = maxShockAndIntensityPerQuarter.maxShockIntensity;
	int intensity = 0;
	if (useQuarters) {
		intensity = maxShockAndIntensityPerQuarter.intensityPerQuarter * amount;
	} else {
		intensity = maxShockAndIntensityPerQuarter.intensityPerHealth * amount;
	}
	bool okayToSend = maxShockAndIntensityPerQuarter.maxShockIntensity <= 100 || (userAuthorizedDangerousValue && userWarnedAboutMaxShock);
	if (!okayToSend) {
		logDebug("Max value too high, aborting.");
		return 1;
	}

	if (intensity > limit) {
		intensity = limit;
	}

	std::vector<PublishCommandT> commands;

	for (auto& id : shockerIds) {
		L l;
		l.set_u(userId);
		l.set_ty("api");
		l.set_w(false);
		l.set_h(false);
		l.set_o("Shock sender library");

		BodyT body;
		body.set_l(l);
		body.set_d(maxShockAndIntensityPerQuarter.durationMilliseconds);
		body.set_m("s");
		body.set_i(intensity);
		body.set_r(true);
		body.set_id(id);

		PublishCommandT command;
		command.set_target("c" + std::to_string(clientId) + "-ops");
		command.set_body(body);
		commands.push_back(command);
	}


	PiShockPayload payload;
	payload.set_operation("PUBLISH");
	payload.set_publish_commands(commands);
	logDebug("Preparing " + std::to_string(commands.size()) + " shocks with an intensity of " + std::to_string(intensity) + " and a duration of " + std::to_string(maxShockAndIntensityPerQuarter.durationMilliseconds) + "ms");

	boost::json::value jsonPayload = boost::json::value_from(payload);
	std::string msg = boost::json::serialize(jsonPayload);
	logDebug("Payload below\n" + msg);
	auto executor = ws.get_executor();
	net::co_spawn(executor, sendMessage(msg), net::detached);
	logDebug("Shock sent");
	return 0;
}


int stop() {
	try {
		ws.close(websocket::close_code::normal);
	} catch (const std::exception& e) {
		logError(e.what());
	}
	std::terminate();
}

bool getIsRunning() {
	return isRunning;
}