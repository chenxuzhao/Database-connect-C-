#include "RedisDB.h"
#include <sstream>

RedisDB::RedisDB() :client(NULL),
exit(true)
{

}

RedisDB::~RedisDB() {

}


bool RedisDB::exec(string  str) {
	vector<string> params;
	this->analysisCommand(str, params);
	this->client->send(params, [](cpp_redis::reply& reply) {
		if (reply.is_error()) {
			cout << reply.error() << endl;
			cout << "redis";
		}
		else {
			cout << "command ok" << endl;
			cout << "redis";
		}
	});
	//this->client->send({"set"})
	this->client->commit();
	return true;
}

bool RedisDB::connect(DBConfig config) {

#ifdef _WIN32
	//! Windows netword DLL init
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0) {
		std::cout << "WSAStartup() failure" << std::endl;
		return false;
	}
#endif /* _WIN32 */
	client = new cpp_redis::redis_client();
	client->connect("127.0.0.1", 6379, [](cpp_redis::redis_client&) {
		std::cout << "client disconnected (disconnection handler)" << std::endl;
	});
	client->auth("root", [](cpp_redis::reply&reply) {
		if (reply.is_error()) {
			std::cerr << "Authentication failed: " << reply.as_string() << std::endl;
		}
		else {
			;
			std::cout << "successful authentication" << std::endl;
		}
	});

	return true;
}

void RedisDB::close() {
	delete client;
#ifdef _WIN32
	WSACleanup();
#endif /* _WIN32 */
	delete this;

}

bool RedisDB::analysisCommand(string command, vector<string> & params) {
	int begin = command.find_first_not_of(" ", 0);
	int end = command.find_first_of(" ", begin);

	while (begin < end && begin >= 0) {
		params.push_back(command.substr(begin, end - begin));
		begin = command.find_first_not_of(" ", end);
		if (begin > 0) {
			end = command.find_first_of(" ", begin);
			if (end < 0)
				end = command.size();
		}
			
	}
	//command = this->buildCommand(&params);
	//params.clear();
	//params.push_back(command);
	return true;
}

string RedisDB::buildCommand(const vector<string> * params) {

	stringstream ss;
	int begin = 1;
	int size = params->size();
	if (size > 1) {
		ss << "{" << params->at(0);
		while (begin < size) {
			ss << "," << params->at(begin);
			begin++;
		}
		ss << "}";
	}

	return ss.str();
}
bool RedisDB::insert() {
	client->set("hello", "42", [](cpp_redis::reply& reply) {
		std::cout << "set hello 42: " << reply << std::endl;
		// if (reply.is_string())
		//   do_something_with_string(reply.as_string())
	});

	// same as client.send({ "DECRBY", "hello", 12 }, ...)
	client->decrby("hello", 12, [](cpp_redis::reply& reply) {
		std::cout << "decrby hello 12: " << reply << std::endl;
		// if (reply.is_integer())
		//   do_something_with_integer(reply.as_integer())
	});

	// same as client.send({ "GET", "hello" }, ...)
	client->get("hello", [](cpp_redis::reply& reply) {
		std::cout << "get hello: " << reply << std::endl;
		// if (reply.is_string())
		//   do_something_with_string(reply.as_string())
	});

	client->zadd("zhello", {}, { { "1", "a" },
	{ "2", "b" },
	{ "3", "c" },
	{ "4", "d" } },
		[](cpp_redis::reply& reply) {
		std::cout << "zadd zhello 1 a 2 b 3 c 4 d: " << reply << std::endl;
	});

	// commands are pipelined and only sent when client.commit() is called
	// client.commit();

	// synchronous commit, no timeout
	client->sync_commit();

	return true;

}
string RedisDB::trim(string str) {
	int begin = str.find_first_not_of(" ");
	int end = str.find_last_not_of(" ");
	if(end>begin)
	return str.substr(begin, end - begin + 1);
	return "";
}

void RedisDB::run() {
	this->exit = false;
	std::cout << "redis>";
	while (!exit) {
		string str;
		getline(cin, str);
		str = trim(str);
		if (str.length() > 0)
			this->exec(str);
		std::cout << "redis>";
	}
	this->close();
}