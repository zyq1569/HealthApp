
#include "patientdata.h"
#include "sqlite3_exec_stmt.h"
#include <boost/locale.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

PatientData::PatientData()
{
	db = NULL;
}

void PatientData::createdb()
{	
	if (db)
		sqlite3_close(db);

	if (sqlite3_open_v2("tonoka.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) != SQLITE_OK)
	{
		std::ostringstream msg;
		msg << "Can't create database: " << sqlite3_errmsg(db);
		throw std::runtime_error(msg.str().c_str());				
	}	
		
	sqlite3_exec(db, "CREATE TABLE studies (studyuid TEXT UNIQUE, patid TEXT, patname TEXT, studydesc TEXT, studydate TEXT, path TEXT, checked TEXT)", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TEMP VIEW listctrl AS SELECT * FROM studies ORDER BY studyuid ASC", NULL, NULL, NULL);
}

PatientData::~PatientData()
{
	if(db)
		sqlite3_close(db);
}

void PatientData::Clear()
{
	if(db)
		sqlite3_close(db);

	boost::filesystem::remove("tonoka.db");

	createdb();
}

bool PatientData::Load(boost::filesystem::path filename)
{
	sqlite3 *backup = NULL;
	std::string p = boost::locale::conv::utf_to_utf<char>(filename.c_str());
	sqlite3_open_v2(p.c_str(), &backup, SQLITE_OPEN_READONLY, NULL);
	sqlite3_backup *bk = sqlite3_backup_init(db, "main", backup, "main");
	int ret = sqlite3_backup_step(bk, -1);
	sqlite3_backup_finish(bk);
	sqlite3_close(backup);
	
	return ret == SQLITE_DONE;
}

bool PatientData::Save(boost::filesystem::path filename)
{
	sqlite3 *backup = NULL;
	std::string p = boost::locale::conv::utf_to_utf<char>(filename.c_str());
	sqlite3_open_v2(p.c_str(), &backup, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	sqlite3_backup *bk = sqlite3_backup_init(backup, "main", db, "main");
	int ret = sqlite3_backup_step(bk, -1);
	sqlite3_backup_finish(bk);	
	sqlite3_close(backup);

	return ret == SQLITE_DONE;
}

int PatientData::AddStudy(std::string studyuid, std::string patid, std::string patname, std::string studydesc, std::string studydate, boost::filesystem::path path)
{
	sqlite3_stmt *insert;
	std::string sql = "INSERT INTO studies VALUES(?, ?, ?, ?, ?, ?, '1')";
	sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &insert, NULL);
	sqlite3_bind_text(insert, 1, studyuid.c_str(), studyuid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 2, patid.c_str(), patid.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 3, patname.c_str(), patname.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 4, studydesc.c_str(), studydesc.length(), SQLITE_STATIC);
	sqlite3_bind_text(insert, 5, studydate.c_str(), studydate.length(), SQLITE_STATIC);

	std::string p = boost::locale::conv::utf_to_utf<char>(path.c_str());
	sqlite3_bind_text(insert, 6, p.c_str(), p.length(), SQLITE_STATIC);
	int res = sqlite3_exec_stmt(insert, NULL, NULL, NULL);
	sqlite3_finalize(insert);
	if(res == SQLITE_DONE || res == SQLITE_ABORT)
		return 1;
	else
		return 0;
}

int getstudiescallback(void *param,int columns,char** values, char**names)
{
	boost::function<int(Study &)> pfn = * static_cast<boost::function<int(Study &)> *>(param);

	Study result(values[0], values[1], values[2], values[3], values[4], boost::locale::conv::utf_to_utf<boost::filesystem::path::value_type>(values[5]), values[6][0] == '1');

	return pfn(result);
}

void PatientData::GetStudies(boost::function< int(Study &) > action)
{
	std::string selectsql = "SELECT studyuid, patid, patname, studydesc, studydate, path, checked FROM studies ORDER BY studyuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);	
	sqlite3_exec_stmt(select, getstudiescallback, &action, NULL);		
	sqlite3_finalize(select);
}

void PatientData::GetCheckedStudies(boost::function< int(Study &) > action)
{
	std::string selectsql = "SELECT studyuid, patid, patname, studydesc, studydate, path, checked FROM studies WHERE (checked = '1') ORDER BY studyuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_exec_stmt(select, getstudiescallback, &action, NULL);
	sqlite3_finalize(select);
}

void PatientData::GetStudies(std::string patientid, std::string patientname, boost::function< int(Study &) > action)
{
	std::string selectsql = "SELECT studyuid, patid, patname, studydesc, studydate, path, checked FROM studies WHERE (patid = ? AND patname = ?) ORDER BY studyuid ASC";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, patientid.c_str(), patientid.length(), SQLITE_STATIC);
	sqlite3_bind_text(select, 2, patientname.c_str(), patientname.length(), SQLITE_STATIC);
	sqlite3_exec_stmt(select, getstudiescallback, &action, NULL);		
	sqlite3_finalize(select);
}

void PatientData::SetStudyCheck(std::string studyuid, bool checked)
{
	std::string selectsql = "UPDATE studies SET checked = ? WHERE (studyuid = ?)";
	sqlite3_stmt *select;
	std::string checkstr = "1";
	if (!checked)
		checkstr = "0";
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_text(select, 1, checkstr.c_str(), checkstr.length(), SQLITE_STATIC);
	sqlite3_bind_text(select, 2, studyuid.c_str(), studyuid.length(), SQLITE_STATIC);	
	sqlite3_exec_stmt(select, getstudiescallback, NULL, NULL);
	sqlite3_finalize(select);
}

int setstudy(Study &study, Study &setthis)
{
	setthis = study;
	return 0;
}

void PatientData::GetStudy(int id, Study& study)
{
	std::string selectsql = "SELECT studyuid, patid, patname, studydesc, studydate, path, checked FROM studies WHERE (rowid = ?)";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);
	sqlite3_bind_int(select, 1, id);
	boost::function< int(Study &) > action = boost::bind(&setstudy, _1, study);
	sqlite3_exec_stmt(select, getstudiescallback, &action, NULL);
	sqlite3_finalize(select);
}

int setint(void *param, int columns, char** values, char**names)
{
	int *a = (int *)param;

	*a = boost::lexical_cast<int>(values[0]);

	return 1;
}

int setstring(void *param, int columns, char** values, char**names)
{
	std::string *a = (std::string *)param;

	*a = values[0];

	return 1;
}

int PatientData::GetCheckedStudyCount()
{
	std::string selectsql = "SELECT COUNT(*) FROM studies WHERE (checked = '1')";
	int s = 0;
	sqlite3_exec(db, selectsql.c_str(), setint, &s, NULL);
	return s;
}

bool PatientData::PathExists(boost::filesystem::path path)
{
	std::string selectsql = "SELECT COUNT(*) FROM studies WHERE (path = ?)";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);

	std::string p = boost::locale::conv::utf_to_utf<char>(path.c_str());
	sqlite3_bind_text(select, 1, p.c_str(), p.length(), SQLITE_STATIC);

	int s = 0;
	sqlite3_exec_stmt(select, setint, &s, NULL);
	sqlite3_finalize(select);

	return s > 0;
}

std::string PatientData::TextSearch(int start, std::string text)
{
	std::string selectsql = "SELECT studyuid FROM listctrl WHERE studyuid LIKE ? OR patid LIKE ? OR patname LIKE ? LIMIT 1 OFFSET ?";
	sqlite3_stmt *select;
	sqlite3_prepare_v2(db, selectsql.c_str(), selectsql.length(), &select, NULL);

	text = "%" + text + "%";
	sqlite3_bind_text(select, 1, text.c_str(), text.length(), SQLITE_STATIC);
	sqlite3_bind_text(select, 2, text.c_str(), text.length(), SQLITE_STATIC);
	sqlite3_bind_text(select, 3, text.c_str(), text.length(), SQLITE_STATIC);
	sqlite3_bind_int(select, 4, start);
	
	std::string studyuid;
	sqlite3_exec_stmt(select, setstring, &studyuid, NULL);
	sqlite3_finalize(select);

	return studyuid;
}