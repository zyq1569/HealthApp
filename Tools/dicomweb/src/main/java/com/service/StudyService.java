package com.service;

import java.util.List;

import com.entity.Study;

/*
 *
 *
 */
public interface StudyService {
	public List<Study> getAllStudy();
	public Study getStudyMoreInfo(Study study);
	public int addStudy(Study study);
	public int delStudy(Study study);
	public int modifyStudy(Study study);
	public boolean findStudy(Study study);
}


