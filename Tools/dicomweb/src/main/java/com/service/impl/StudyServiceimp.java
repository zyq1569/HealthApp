package com.service.impl;

import java.util.List;

import com.dao.StudyDao;
import com.dao.impl.StudyDaoimp;
import com.entity.Study;
import com.service.StudyService;

/*
 *
 *
 */
public class StudyServiceimp implements StudyService{
	private StudyDao studyDao = null;
	
	public StudyServiceimp(){
		studyDao = new StudyDaoimp();
	}

	@Override
	public List<Study> getAllStudy() {
		return studyDao.getAllStudy();
	}

	@Override
	public Study getStudyMoreInfo(Study study) {
		return studyDao.getStudyMoreInfo(study);
	}

	@Override
	public int addStudy(Study study) {
		return studyDao.addStudy(study);
	}

	@Override
	public int delStudy(Study study) {
		return studyDao.delStudy(study);
	}

	@Override
	public int modifyStudy(Study study) {
		return studyDao.modifyStudy(study);
	}
	
	//查找有没有指定的检查
	public boolean findStudy(Study study){
		return studyDao.findStudy(study);
				
	}

}


