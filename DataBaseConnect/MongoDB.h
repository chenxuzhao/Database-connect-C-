#pragma once

#include <mongoc.h>
#include <vector>

#include "Database.h"


struct _Collection {
	mongoc_collection_t * collection;
	string name;
};
struct _Command {
	string collection;
	string command;
	string params;
};

class MongoDB :
	public Database
{

private :
	mongoc_client_t *client;
	mongoc_database_t * selectDatabase;
	_Collection * currentCollection;
	bool switchDataBase;
	bool exit;
public:
	MongoDB();
	virtual ~MongoDB();

	virtual bool exec(string sql);
	virtual bool connect(DBConfig config);
	virtual void close();
	virtual void run();

private:
	bool equal(char, char);
	bool flags(char);
	bool analyseJsonArray(string ,vector<string> * ,string & );
	
	void _show(_Command *command);
	void _quit();
	void _insert(_Command *command);
	void _find(_Command *command);
	void _remove(_Command *command);
	void _update(_Command *command);
	void _use(_Command *command);

	_Command * analyseCommand(string);
	_Collection * _getCollection(string);
	string trim(string);
	void destroyCollection();
};

