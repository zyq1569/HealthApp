#include "HMariaDb.h"

HMariaDb::HMariaDb(void)
{
    pConnectionHandlerPtr = NULL;
}

HMariaDb::HMariaDb(const std::string& server, const std::string& user, const std::string& password, const std::string& database)
{
    pConnectionHandlerPtr = mysql_init(NULL);
    if (NULL == mysql_real_connect(pConnectionHandlerPtr, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0))
    {
        throw HSqlError("Failed to connect to database: Error: " + std::string(mysql_error(pConnectionHandlerPtr)));
    }
    if (!mysql_set_character_set(pConnectionHandlerPtr, "GBK"))
    {
        // const char *c = mysql_character_set_name(pconnectionhandlerptr);
        //printf("new client character set: %s\n",
        //     mysql_character_set_name(pconnectionhandlerptr));
    }
    else
    {
        throw HSqlError("Failed to mysql_set_character_set GBK" + std::string(mysql_error(pConnectionHandlerPtr)));
    }
    m_server = server;
    m_user = user;
    m_password = password;
    m_database = database;
}

HMariaDb::~HMariaDb(void)
{
    close();
}

void HMariaDb::connect(const std::string& server, const std::string& user, const std::string& password, const std::string& database)
{
    pConnectionHandlerPtr = mysql_init(NULL);

    if (NULL == mysql_real_connect(pConnectionHandlerPtr, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, NULL, 0))
    {
        throw HSqlError("Failed to connect to database: Error: " + std::string(mysql_error(pConnectionHandlerPtr)));
    }
    if (!mysql_set_character_set(pConnectionHandlerPtr, "GBK"))
    {
        // const char *c = mysql_character_set_name(pconnectionhandlerptr);
        //printf("new client character set: %s\n",
        //     mysql_character_set_name(pconnectionhandlerptr));
    }
    else
    {
        throw HSqlError("Failed to mysql_set_character_set GBK" + std::string(mysql_error(pConnectionHandlerPtr)));
    }
    m_server = server;
    m_user = user;
    m_password = password;
    m_database = database;
}

int HMariaDb::reconnection()
{
    close();
    int try_times = 0;
    while (try_times < 3)
    {
        try
        {
            pConnectionHandlerPtr = mysql_init(NULL);
            if (NULL != mysql_real_connect(pConnectionHandlerPtr, m_server.c_str(), m_user.c_str(), m_password.c_str(), m_database.c_str(), 0, NULL, 0))
            {
                if (!mysql_set_character_set(pConnectionHandlerPtr, "GBK"))
                {
                    return mysql_ping(pConnectionHandlerPtr);
                }
                else
                {
                    try_times++;
                }
            }
            else
            {
                try_times++;
            }
        }
        catch (...)
        {
            try_times++;
        }
#ifdef _win32
        Sleep(10);
#else
        sleep(10);
#endif
    }
    return -1;
}

void HMariaDb::query(const std::string& sql)
{
    if (NULL == pConnectionHandlerPtr)
        return;

    if (NULL != mysql_ping(pConnectionHandlerPtr))
    {
        if (NULL != reconnection())
        {
            throw HSqlError("Failed to reconnection to database: Error: " + std::string(mysql_error(pConnectionHandlerPtr)));
        }
    }
    if (!(mysql_query(pConnectionHandlerPtr, sql.c_str()) == 0))
    {
        throw HSqlError("Failed to execute sql: Error: " + std::string(mysql_error(pConnectionHandlerPtr)));
    }
    else
    {
        resultset.clear();
        getresult(resultset);
    }
}

void HMariaDb::execute(const std::string& sql)
{
    if (NULL == pConnectionHandlerPtr)
        return;
    if (NULL != mysql_ping(pConnectionHandlerPtr))
    {
        if (NULL != reconnection())
        {
            throw HSqlError("Failed to reconnection to database: Error: " + std::string(mysql_error(pConnectionHandlerPtr)));
        }
    }
    if (!(mysql_query(pConnectionHandlerPtr, sql.c_str()) == 0))
    {
        throw HSqlError("Failed to execute sql: Error: " + std::string(mysql_error(pConnectionHandlerPtr)));
    }
}

void HMariaDb::getresult(ResultSet& rs)
{
    if (NULL == pConnectionHandlerPtr)
        return;
    MYSQL_RES *result = NULL;
    MYSQL_FIELD *fields = NULL;

    // You must call mysql_store_result() or mysql_use_result()
    // for every query that successfully retrieves data (SELECT, SHOW, DESCRIBE, EXPLAIN).

    result = mysql_use_result(pConnectionHandlerPtr);
    if (result == NULL)
    {
        return;
    }

    MYSQL_ROW row;
    unsigned int num_fields;
    unsigned int i;

    num_fields = mysql_num_fields(result);
    fields = mysql_fetch_fields(result);
    rs.clearField();
    std::string sdata;
    for (i = 0; i < num_fields; i++)
    {
        printf("Field %u is %s/n", i, fields[i].name);
        sdata += fields[i].name;
        rs.addField(fields[i].name);
    }
    // get rows
    while ((row = mysql_fetch_row(result)))
    {
        //unsigned long *lengths;
        //lengths = mysql_fetch_lengths(result);

        std::vector<std::string> myRow;

        // get fields od row
        for (i = 0; i < num_fields; i++)
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
