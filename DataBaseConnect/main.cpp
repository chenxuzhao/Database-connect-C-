#include <iostream>
#include "DB.h"

string trim(string str) {
	int begin = str.find_first_not_of(" ");
	int end = str.find_last_not_of(" ");
	if (end>begin)
		return str.substr(begin, end - begin + 1);
	return "";
}

int main() {

	struct DBConfig config;
	config.driver = "mongodb";
	config.hostname = "localhost";
	config.name = "test";
	config.password = "root";
	config.port = "3306";
	config.username = "root";



	DB * db = DB::getDB();

	while (true) {
		cout << "select database or exit" << endl;
		cout << "      mysql" << endl;
		cout << "      mongodb" << endl;
		cout << "      redis" << endl;
		string str;
		getline(cin, str);
		if (str.find("exit") >= 0)
			break;
		config.driver = trim(str);
		if (db->connect(config))
			db->run();
	}
}