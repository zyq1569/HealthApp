//
// MySql.h
#pragma once

#include "mysql.h" 

#include <iostream>
#include <string>
#include <vector>
#include <map>

struct MySQLData
{
	enum_field_types type;
	int NumericScale;//精度
	int Precision;
	std::string name;
	short int16_data;
	short tiny_data;
	int int32_data;
	bool bit_data;
	__int64 int64_data;
	float float_data;
	double double_data;
	MYSQL_TIME time_data;
	char* str_data;
	char* *blob_data;
	unsigned long length;
	my_bool is_null;
	std::string value;//记录返回值
	bool init;//记录返回值是否初始化
};

class ResultSet
	/// ResultSet imp
{
public:

	ResultSet()
		: _current(0)
	{
	}

	void addRow(const std::vector<std::string>& row)
	{
		_resultSet.push_back(row);
	}

	bool fetch(size_t field, std::string& fieldValue)
	{
		size_t sz = _resultSet.size();

		if(sz)
		{
			if(sz > _current)
			{
				fieldValue = _resultSet[_current++][field];

				return true;
			}
		}

		_current = 0;

		return false;
	}

	bool fetch(std::vector<std::string>& rowValue)
	{
		size_t sz = _resultSet.size();

		if(sz)
		{
			if(sz > _current)
			{
				rowValue = _resultSet[_current++];

				return true;
			}
		}

		_current = 0;

		return false;
	}

	std::string get(size_t row, size_t field)
	{
		return _resultSet[row][field];
	}

	std::vector<std::string> get(size_t row)
	{
		return _resultSet[row];
	}

	size_t countRows(void)
	{
		if (_resultSet.empty()) return 0;

		return _resultSet.size();
	}

	size_t countFields(void)
	{
		if (_resultSet[0].empty()) return 0;

		return _resultSet[0].size();
	}

private:

	std::vector<std::vector<std::string> > _resultSet;

	size_t _current;

}; // ResultSet

class DataBaseError 
	: public std::exception
	/// DataBaseError class
{
public:
	DataBaseError(const std::string& what)
		: exception(what.c_str())
	{
	}

};

class MySqlDB
{
public:

	MySqlDB()
	{
	}
	MySqlDB(const std::string& server, const std::string& user, const std::string& password, const std::string& database)
	{
		_connectionHandlerPtr = mysql_init(NULL);
		if (NULL == mysql_real_connect(_connectionHandlerPtr, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0))
		{
			throw DataBaseError("Failed to connect to database: Error: " + std::string(mysql_error(_connectionHandlerPtr)));
		}
	}


	virtual ~MySqlDB()
	{
	}
	
	void connect(const std::string& server, const std::string& user, const std::string& password, const std::string& database)
	{		
		_connectionHandlerPtr = mysql_init(NULL); 
				
		if (NULL == mysql_real_connect(_connectionHandlerPtr, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0)) 
		{ 
			throw DataBaseError("Failed to connect to database: Error: " + std::string(mysql_error(_connectionHandlerPtr)));
		} 
	}

	void execute(const std::string& sql)
	{
		std::cout << sql << std::endl;

		if(!(mysql_query(_connectionHandlerPtr, sql.c_str()) == 0) )
		{ 
			throw DataBaseError("Failed to execute sql: Error: " + std::string(mysql_error(_connectionHandlerPtr)));
		}
	}

	void query(const std::string& sql)
	{
		if (!(mysql_query(_connectionHandlerPtr, sql.c_str()) == 0))
		{
			throw DataBaseError("Failed to execute sql: Error: " + std::string(mysql_error(_connectionHandlerPtr)));
		}
	}

	void populate(ResultSet& rs)
	{
		MYSQL_RES *result = NULL; // result of querying for all rows in table 
		MYSQL_FIELD *fields = NULL;

		// You must call mysql_store_result() or mysql_use_result() 
		// for every query that successfully retrieves data (SELECT, SHOW, DESCRIBE, EXPLAIN). 

		result = mysql_use_result(_connectionHandlerPtr); 

		MYSQL_ROW row;
		unsigned int num_fields;
		unsigned int i;



		num_fields = mysql_num_fields(result);
		fields = mysql_fetch_fields(result);
		std::string sdata;
		for (i = 0; i < num_fields; i++)
		{
			printf("Field %u is %s/n", i, fields[i].name);
			sdata += fields[i].name;
		}
		// get rows
		while ((row = mysql_fetch_row(result)))
		{
			//unsigned long *lengths;
			//lengths = mysql_fetch_lengths(result);

			 std::vector<std::string> myRow;

			// get fields od row
			for(i = 0; i < num_fields; i++)
			{
				if (row[i] == NULL) 
				{ 
					myRow.push_back("NULL");
				} 
				else 
				{ 
					myRow.push_back(row[i]);
				}
			}

			rs.addRow(myRow);
		}

		mysql_free_result(result); 
	}

protected:

	void close(void)
	{
		mysql_close(_connectionHandlerPtr); 
	}
	
private:
	
	MYSQL* _connectionHandlerPtr;
};
