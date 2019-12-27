package com.dicomserver.health.dao;

import com.dicomserver.health.entity.ReportData;
import com.dicomserver.health.entity.StudyData;

import java.util.List;

public interface StudyDataDao {
    public List<StudyData> getAllStudy();
    public List<StudyData> getAllStudyImageData();
    public StudyData getStudyMoreInfo(StudyData study);
    public int addStudy(StudyData study);
    public int delStudy(StudyData study);
    public int modifyStudy(StudyData study);
    public boolean findStudy(StudyData study);

    //deal patient table
    public int addPatient(StudyData orderStudy);
    public int updatePatient(StudyData orderStudy);
    public int markPatient(StudyData orderStudy);

    //deal order tablel
    public int addOrderStudy(StudyData orderStudy);
    public int updateOrderStudy(StudyData orderStudy);
    public int markOrderStudy(StudyData orderStudy);

    //deal report table
    public int getStudyReport(ReportData reportData);
    public int addStudyReport(ReportData reportData);
    public int updateStudyReport(ReportData reportData);
    public int markStudyReport(ReportData reportData);
}
