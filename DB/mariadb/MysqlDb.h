#pragma once

//--------------------
#include "../../include/mysql/win32/mysql.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>


//--------------------
//***************$$$$$$$$$*************************
//#ifdef HAVE_WINDOWS_H
//#include <direct.h>      /* for _mkdir() */
//#endif
//#include "dcmtk/oflog/fileap.h"

class SqlError : public std::exception
{
public:
    SqlError(const std::string& what)
        //: exception(what.c_str())
    {
    }
};

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
    #ifdef _WIN32
    __int64 int64_data;
    #else
    int64_t int64_data;
    #endif
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
    void clearField()
    {
        _field.clear();
    }
    void addField(std::string field)
    {
        _field.push_back(field);
    }

    std::string getFieldName(int index)
    {
        if (index < _field.size())
        {
            return _field[index];
        }
        return "";
    }
    bool getIndex(std::string FieldName, size_t &index)
    {
        size_t count = _field.size();
        for (size_t i = 0; i < count; i++)
        {
            if (getFieldName(i) == FieldName)
            {
                index = i;
                return true;
            }
        }
        return false;
    }

    bool getValue(size_t row, std::string FieldName, std::string &value)
    {
        size_t index = -1;
        if (getIndex(FieldName,index))
        {
            value = get(row, index);
            return true;
        }
        else
        {
            return false;
        }
        return false;
    }

    bool fetch(size_t field, std::string& fieldValue)
    {
        size_t sz = _resultSet.size();
        if (sz)
        {
            if (sz > _current)
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
        if (sz)
        {
            if (sz > _current)
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
        return _field.size();
        //if (_resultSet[0].empty())
        //          return 0;
        //return _resultSet[0].size();
    }
    void clear()
    {
        if (_resultSet.empty())
        {
            return;
        }
        else
        {
            size_t s = _resultSet.size();
            for (size_t i = 0; i < s; i++)
            {
                _resultSet[i].clear();
            }
            _resultSet.clear();
        }
    }
private:
    std::vector<std::vector<std::string> > _resultSet;
    size_t _current;
    std::vector<std::string > _field;
}; // ResultSet

class MysqlDb
{
public:
    MysqlDb(void);
    MysqlDb(const std::string& server, const std::string& user, const std::string& password, const std::string& database);
    ~MysqlDb(void);

    void connect(const std::string& server, const std::string& user, const std::string& password, const std::string& database);
    bool open(const std::string& server, const std::string& user, const std::string& password, const std::string& database, std::string &error);
    void execute(const std::string& sql);
    void query(const std::string& sql);
    // if return NULL :no result!
    ResultSet * QueryResult()
    {
        if (resultset.countRows() < 1)
        {
            return NULL;
        }
        return &resultset;
    }
private:
    MYSQL* pConnectionHandlerPtr;
    ResultSet resultset;
    std::string m_server, m_user, m_password, m_database;
protected:
    void close(void)
    {
        if (NULL != pConnectionHandlerPtr)
        {
            mysql_close(pConnectionHandlerPtr);
            pConnectionHandlerPtr = NULL;
        }
    }
    void getresult(ResultSet& rs);
    int reconnection();
};
