#include "MysqlDatabase.h"
#include <sstream>


MysqlDatabase::MysqlDatabase() :driver(NULL),
con(NULL),
exit(true),
switchDataBase(false)
{

}


MysqlDatabase::~MysqlDatabase()
{
}


bool MysqlDatabase::connect(DBConfig config) {
	try {
		driver = get_driver_instance();
		con = driver->connect("tcp://" + config.hostname + ":" + config.port, config.username, config.password);
		return true;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}


	return false;

}
bool  MysqlDatabase::strAvailable(string &str) {
	if (str.empty())
		return false;
	int begin = str.find_first_not_of(" ");
	int end = str.find_last_not_of(" ");
	if (end >= begin) {
		str = str.substr(begin, end - begin + 1);
		return true;
	}
	return false;
}

void MysqlDatabase::run() {
	this->exit = false;
	stringstream ss;
	cout << "mysql>";
	while (!exit) {
		try {
			string str;
			getline(cin, str);
			if (this->strAvailable(str)) {
				char ch = str[str.length() - 1];
				if (ch == ';') {
					ss << str;
					cout << "check :" << ss.str() << endl;
					int cursor = ss.str().find_first_of(" ");
					string command = ss.str().substr(0, cursor);
					if (command == "select")
						this->_query(ss.str());
					else if (command == "insert")
						this->_insert(ss.str());
					else if (command == "update")
						this->_update(ss.str());
					else if (command == "delete")
						this->_delete(ss.str());
					else if (command == "show")
						this->show(ss.str());
					else if (command == "use")
						this->use(ss.str());
					else if (command == "quit")
						this->quit();
					else
						this->exec(ss.str());

					ss.str("");
					cout << "mysql>";
				}
				else if (ch == '\\') {
					ss << str.substr(0, str.length() - 1) << " ";
					cout << "-->";
				}
				else {
					ss << str << " ";
					cout << "-->";
				}
			}
		}
		catch (sql::SQLException &e) {
			ss.str("");
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << ", SQLState: " << e.getSQLState() << " )" << endl;
			cout << "mysql>";
		}
	}

	this->close();

}

void  MysqlDatabase::show(string sql) {
	try {
		if (sql.find("databases") != -1) {

		}
		else if (sql.find("tables") != -1 && switchDataBase) {

		}
		else
			return;

		sql::Statement * stm = con->createStatement();
		sql::ResultSet *result = stm->executeQuery(sql);
		while (result->next())
			cout << "  " << result->getString(1) << endl;

	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}


}
void  MysqlDatabase::use(string sql) {
	try {
		int begin = sql.find_first_of(" ");
		int end = sql.find_first_of(";");
		sql = sql.substr(begin, end - begin);

		begin = sql.find_first_not_of(" ");
		end = sql.find_last_not_of(" ");
		string schema = sql.substr(begin, end + 1 - begin);
		con->setSchema(schema);
		this->switchDataBase = true;
		cout << " switch " << schema << endl;
	}
	catch (sql::SQLException &e) {
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

void  MysqlDatabase::quit() {
	this->exit = true;
	this->switchDataBase = false;
}

void MysqlDatabase::_query(string sql) {
	sql::Statement * stmt = con->createStatement();
	sql::ResultSet * result = stmt->executeQuery(sql);
	sql::ResultSetMetaData * rows = result->getMetaData();
	int columns = rows->getColumnCount();
	for (int i = 1; i <= columns; i++) {
		cout << rows->getColumnName(i) << "\t";
	}
	cout << endl;
	while (result && result->next()) {
		for (int i = 1; i <= columns; i++) {
			cout << result->getString(i) << "\t";
		}
		cout << endl;
	}
	result->close();
	delete stmt;
	delete result;


}
bool MysqlDatabase::_update(string sql) {
	cout << sql << endl;
	bool result = this->exec(sql);
	if (result)
		cout << "   update ok" << endl;
	else
		cout << "   update failed" << endl;
	return result;

}
bool MysqlDatabase::_insert(string sql) {
	bool result = this->exec(sql);
	if (result)
		cout << "   insert ok" << endl;
	else
		cout << "   insert failed" << endl;
	return result;
}

bool  MysqlDatabase::_delete(string sql) {
	bool result = this->exec(sql);
	if (result)
		cout << "   delete ok" << endl;
	else
		cout << "   delete failed" << endl;
	return result;
}

bool MysqlDatabase::exec(string sql) {
	try {
		sql::Statement * stmt = con->createStatement();
		stmt->execute(sql);
		delete stmt;
		return true;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		return false;
	}
}
void MysqlDatabase::close() {
	delete con;
	delete this;
}