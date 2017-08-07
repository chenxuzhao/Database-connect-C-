#pragma once
#include "Database.h"

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>


using namespace std;
using namespace sql;



class MysqlDatabase :
	public Database
{
private:
	sql::Driver *driver;
	sql::Connection *con;

	bool exit;
	bool switchDataBase;
	
public:
	MysqlDatabase();
	virtual ~MysqlDatabase();
private:
	bool strAvailable(string &);
	void show(string);
	void use(string);
	void quit();

public:

	void _query(string);
	bool _update(string sql);
	bool _insert(string sql);
	bool _delete(string sql);
	
	virtual bool connect(DBConfig config);
	virtual void close();
	virtual void run();
	virtual bool exec(string sql);
};

