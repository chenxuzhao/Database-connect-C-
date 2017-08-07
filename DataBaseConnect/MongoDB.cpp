#include "MongoDB.h"
#include <exception>  
#include <iostream>

#include <stack>
MongoDB::MongoDB() :client(NULL),
selectDatabase(NULL),
currentCollection(NULL),
switchDataBase(false),
exit(true)
{

}


MongoDB::~MongoDB()
{
}

bool MongoDB::equal(char ch1, char ch2) {
	switch (ch1)
	{
	case '{':
		return ch2 == '}';
	case '[':
		return ch2 == ']';
	case '*':
		return ch2 == ',';
	default:
		return false;
	}
}

bool MongoDB::flags(char ch) {

	switch (ch)
	{
	case '{':
		return true;
	case '}':
		return true;
	case '[':
		return true;
	case ']':
		return true;
	case ',':
		return true;
	default:
		return false;
	}
}

bool MongoDB::analyseJsonArray(string str, vector<string> *params, string & error) {
	if (str == "") {
		params->push_back("{}");
		return true;
	}

	stack<char> chars;
	chars.push('#');
	chars.push('*');
	int begin = 0;
	int i = 0;
	for (int size = str.length(); i < size; i++) {
		if (flags(str[i])) {
			char tmp = chars.top();
			if (this->equal(tmp, str[i])) {

				if (tmp != '*') {
					chars.pop();
				}
				else {
					params->push_back(str.substr(begin, i - begin));
					begin = i + 1;
				}
			}
			else {
				chars.push(str[i]);
			}
		}
	}

	if (chars.top() == '*') {
		params->push_back(str.substr(begin, i - begin));
		return true;
	}

	if (chars.top() != '*' || chars.top() != '#')
		error = "json format error";
	return false;

}

_Command * MongoDB::analyseCommand(string str) {
	struct _Command *command = new _Command();
	string header = str.substr(0, 2);
	int begin = str.find_first_of(".");
	if (header == "db" && begin > 0) {
		str = trim(str.substr(begin + 1));
		begin = str.find_first_of(".");
		if (begin > 0)
			command->collection = trim(str.substr(0, begin));
		else {
			delete command;
			return NULL;
		}
		str = trim(str.substr(begin + 1));
		begin = str.find_first_of("(");
		if (begin >= 0)
			command->command = trim(str.substr(0, begin));
		else {
			delete command;
			return NULL;
		}

		str = str.substr(begin + 1);
		begin = str.find_last_of(")");
		if (begin == 0)
			command->params = "";
		else if (begin > 0)
			command->params = trim(str.substr(0, begin));
		else {
			delete command;
			return NULL;
		}
		return command;
	}
	else {
		int space = str.find_first_of(" ");
		command->command = str.substr(0, space);
		str = str.substr(space);
		command->params = trim(str);
		return command;
	}
}

string MongoDB::trim(string str) {
	int begin = str.find_first_not_of(" ");
	int end = str.find_last_not_of(" ");
	return str.substr(begin, end - begin + 1);
}

bool MongoDB::exec(string sql) {

	return true;

}

bool MongoDB::connect(DBConfig config) {
	mongoc_init();
	string mg = "mongodb://root:root@127.0.0.1:27017";
	client = mongoc_client_new(mg.c_str());
	if (!client)
		return false;
	mongoc_client_set_error_api(client, 2);
	return true;

}

_Collection * MongoDB::_getCollection(string collection) {
	///线程不安全
	if (this->currentCollection != NULL && this->currentCollection->name == collection)
		return this->currentCollection;
	else {
		destroyCollection();
		this->currentCollection = new _Collection();
		this->currentCollection->collection = mongoc_database_get_collection(this->selectDatabase, collection.c_str());
		this->currentCollection->name = collection;
	}

	return this->currentCollection;

}

void  MongoDB::destroyCollection() {
	if (this->currentCollection != NULL)
		mongoc_collection_destroy(this->currentCollection->collection);
	delete this->currentCollection;
	this->currentCollection = NULL;
}

void MongoDB::_insert(_Command *command) {
	mongoc_collection_t * col = _getCollection(command->collection)->collection;
	const char * data = command->params.c_str();
	bson_t bson;
	bson_error_t error;
	bool result = bson_init_from_json(&bson, data, strlen(data), &error);
	if (!result) {
		std::cout << error.message << endl;
		return;
	}

	else {
		mongoc_bulk_operation_t *bulk = mongoc_collection_create_bulk_operation(col, true, NULL);
		mongoc_bulk_operation_insert(bulk, &bson);
		bool ret = mongoc_bulk_operation_execute(bulk, NULL, &error);

		if (!ret) {
			cout << error.message << endl;
		}else{
			cout << "insert ok" << endl;
		}
		bson_destroy(&bson);
		mongoc_bulk_operation_destroy(bulk);
	}
}

void MongoDB::_show(_Command *command) {
	try {
		if (command->params.find("dbs") != -1) {
			bson_error_t error;
			char **strv;
			unsigned i;
			if ((strv = mongoc_client_get_database_names(client, &error))) {
				for (i = 0; strv[i]; i++)
					cout << "  " << strv[i] << endl;
				bson_strfreev(strv);
			}
			else {
				cout << error.message << endl;
			}
		}
		else if (command->params.find("collections") != -1 && switchDataBase) {
			if (this->selectDatabase != NULL) {
				bson_error_t error;
				char **strv;
				unsigned i;

				if ((strv = mongoc_database_get_collection_names(this->selectDatabase, &error))) {
					for (i = 0; strv[i]; i++)
						cout << strv[i] << endl;
					bson_strfreev(strv);
				}
				else {
					cout << "Command failed: " << error.message << endl;
				}
			}
		}
		else {
			cout << "Command failed" << endl;
		}
	}
	catch (exception &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what() << endl;
	}
}

void MongoDB::_quit() {
	this->exit = true;
}

void MongoDB::_find(_Command *command) {
	bson_t filter;
	bson_t opts;
	bson_error_t error;
	vector<string> params;
	string err;
	if (this->analyseJsonArray(command->params, &params, err)) {
		int size = params.size();
		if (size > 2) {
			cout << "params count error" << endl;
			return;
		}
		const char * str = params[0].c_str();
		bool result = bson_init_from_json(&filter, str, strlen(str), &error);
		if (!result) {
			std::cout << error.message << endl;
			return;
		}
		string optsJson = "{}";
		if (size > 1) {
			optsJson = params[1];
		}
		bool opts_result = bson_init_from_json(&opts, optsJson.c_str(), strlen(optsJson.c_str()), &error);
		if (!opts_result) {
			std::cout << error.message << endl;
			return;
		}

		mongoc_collection_t * col = _getCollection(command->collection)->collection;

		mongoc_cursor_t  * cursor = mongoc_collection_find_with_opts(col, &filter, &opts, NULL);

		const bson_t *doc = NULL;
		//doc 会被自动清空
		while (mongoc_cursor_next(cursor, &doc)) {
			char * str = bson_as_json(doc, NULL);
			printf("%s\n", str);
			bson_free(str);
		}
		mongoc_cursor_destroy(cursor);
	}
	else {
		cout << err << endl;
	}



}

void MongoDB::_update(_Command *command) {
	//TODO
	mongoc_update_flags_t flags = MONGOC_UPDATE_MULTI_UPDATE;
	mongoc_collection_t * col = _getCollection(command->collection)->collection;
	bson_t selector;
	bson_t update;
	bson_error_t error;
	vector<string> params;
	string err;
	if (this->analyseJsonArray(command->params, &params, err)) {
		int size = params.size();
		if (size > 2) {
			cout << "params count error" << endl;
			return;
		}
		const char * str = params[0].c_str();
		bool result = bson_init_from_json(&selector, str, strlen(str), &error);
		if (!result) {
			std::cout << error.message << endl;
			return;
		}
		const char * ups = params[1].c_str();
		bool result_ups = bson_init_from_json(&update, ups, strlen(ups), &error);
		if (!result_ups) {
			std::cout << error.message << endl;
			return;
		}
		mongoc_write_concern_t * write_concern = mongoc_write_concern_new();
		bool res = mongoc_collection_update(col, flags, &selector, &update,
			write_concern, &error);
		mongoc_write_concern_destroy(write_concern);
		if (!res) {
			cout << error.message << endl;
			return;
		}
		cout << "update ok";
	}
}

void MongoDB::_remove(_Command *command) {
	//TODO
	mongoc_remove_flags_t flags = MONGOC_REMOVE_NONE;
	mongoc_collection_t * col = _getCollection(command->collection)->collection;
	bson_t selector;
	bson_error_t error;
	vector<string> params;
	string err;
	if (this->analyseJsonArray(command->params, &params, err)) {
		int size = params.size();
		if (size > 1) {
			cout << "params count error" << endl;
			return;
		}
		const char * str = params[0].c_str();
		bool result = bson_init_from_json(&selector, str, strlen(str), &error);
		if (!result) {
			std::cout << error.message << endl;
			return;
		}
		mongoc_write_concern_t * write_concern = mongoc_write_concern_new();
		bool res = mongoc_collection_remove(col, flags, &selector, write_concern, &error);
		mongoc_write_concern_destroy(write_concern);
		if (!res) {
			cout << error.message << endl;
			return;
		}
		cout << "remove ok" << endl;
		
	}
}

void MongoDB::_use(_Command *command) {
	if (this->selectDatabase != NULL) {
		mongoc_database_destroy(this->selectDatabase);
	}
	this->selectDatabase = mongoc_client_get_database(this->client, command->params.c_str());
	this->switchDataBase = true;
	cout << "switch " << command->params << endl;
}

void MongoDB::close() {
	this->destroyCollection();
	if (this->selectDatabase != NULL)
		mongoc_database_destroy(this->selectDatabase);
	mongoc_client_destroy(client);
	mongoc_cleanup();
	delete this;
}

void MongoDB::run() {
	this->exit = false;
	std::cout << "mongodb>";
	while (!exit) {
		string str;
		getline(cin, str);
		_Command * command = analyseCommand(str);
		if (command != NULL) {
			string comm = command->command;
			if (comm == "insert") {
				this->_insert(command);
			}
			else if (comm == "update") {
				this->_update(command);
			}
			else if (comm == "find") {
				this->_find(command);
			}
			else if (comm == "remove") {
				this->_remove(command);
			}
			else if (comm == "show") {
				this->_show(command);
			}
			else if (comm == "use") {
				this->_use(command);
			}
			else {
				std::cout << "command error" << endl;
			}
		}
		else {
			std::cout << "command error" << endl;
		}
		delete command;
		std::cout << "mongodb>";
	}
	this->close();
}

