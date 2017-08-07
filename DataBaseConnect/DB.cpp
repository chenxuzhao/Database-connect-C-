#include "DB.h"

DB* DB::_instance = NULL;

DB::DB():database(NULL)
{
}

DB* DB::getDB() {
	if (_instance == NULL) {
		_instance = new DB();
	}
	
	return _instance;
}

bool DB::connect(struct DBConfig config) {
	//TODO
	if (config.driver == "mysql") {
		this->database = new MysqlDatabase();
		return this->database->connect(config);
	}

	if (config.driver == "mongodb") {
		this->database = new MongoDB();
		return this->database->connect(config);
	}

	if (config.driver == "redis") {
		this->database = new RedisDB();
		return this->database->connect(config);
	}
	cout << "not found " << config.driver << endl;
	return false;
		

	
}
bool DB::exec(string sql) {
	if(this->database!=NULL)
	return database->exec(sql);
	return false;
}

void DB::close() {
	if (this->database != NULL)
		this->database->close();
	this->database = NULL;
}

DB::~DB()
{
}

void DB::run() {
	this->database->run();
}
