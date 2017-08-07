#pragma once
#include <string>
using namespace std;

struct DBConfig {
	string driver;
	string hostname;
	string port;
	string name;
	string username;
	string password;
};

class Database
{

public:
	Database();
	virtual ~Database();
	virtual bool connect(DBConfig config) = 0;
	virtual void close() = 0;
	virtual void run() = 0;
	virtual bool exec(string) = 0;
};

