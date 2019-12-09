package com.dicomserver.health.dao;

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
}
