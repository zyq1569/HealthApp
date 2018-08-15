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
	char* name;
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
	char* value;//记录返回值
	bool init;//记录返回值是否初始化
};

class ResultSet
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

class HSqlError : public std::exception
{
public:
	HSqlError(const std::string& what)
		: exception(what.c_str())
	{
	}
};


class HMariaDb
{
public:
	HMariaDb(void);
	HMariaDb(const std::string& server, const std::string& user, const std::string& password, const std::string& database);
	~HMariaDb(void);

	void connect(const std::string& server, const std::string& user, const std::string& password, const std::string& database);
	void execute(const std::string& sql);
	void query(const std::string& sql);
	void getresult(ResultSet& rs);
private:
	MYSQL* pConnectionHandlerPtr;
protected:
	void close(void)
	{
		if (NULL != pConnectionHandlerPtr)
		{
			mysql_close(pConnectionHandlerPtr);
			pConnectionHandlerPtr = NULL;
		}
	}
};
