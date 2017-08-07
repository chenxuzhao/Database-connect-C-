#pragma once
#include "Database.h"
#include "MysqlDatabase.h"
#include "MongoDB.h"
#include "RedisDB.h"

class DB
{
private:
	static DB * _instance;
	Database * database;

	DB();
	

public:
	~DB();
	static DB * getDB();
	bool exec(string sql);
	bool insert();
	bool connect(struct DBConfig config);
	void close();
	void run();
};

