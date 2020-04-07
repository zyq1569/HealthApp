#ifndef _PATIENTDATA_
#define _PATIENTDATA_

#include "sqlite3.h"
#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

class Study
{
public:
	Study() {}
	Study(std::string studyuid, std::string patid, std::string patname, std::string studydesc, std::string studydate, boost::filesystem::path path, bool checked)
		: studyuid(studyuid), patid(patid), patname(patname), studydesc(studydesc), studydate(studydate), path(path), checked(checked) {}
	std::string studyuid;
	std::string patid;
	std::string patname;
	std::string studydesc;
	std::string studydate;
	boost::filesystem::path path;
	bool checked;
};

class PatientData
{
public:

	PatientData();
	~PatientData();
	void createdb();
	void Clear();
	bool Load(boost::filesystem::path filename);
	bool Save(boost::filesystem::path filename);
	int AddStudy(std::string studyuid, std::string patid, std::string patname, std::string studydesc, std::string studydate, boost::filesystem::path path);
	void GetStudies(boost::function< int(Study &) > action);
	void GetCheckedStudies(boost::function< int(Study &) > action);
	void GetStudies(std::string patientid, std::string patientname, boost::function< int(Study &) > action);
	void SetStudyCheck(std::string studyuid, bool checked);
	void GetStudy(int id, Study& study);
	int GetCheckedStudyCount();
	bool PathExists(boost::filesystem::path path);
	std::string TextSearch(int start, std::string text);
protected:
	sqlite3 *db;

};


#endif