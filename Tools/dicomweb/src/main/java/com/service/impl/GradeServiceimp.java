package com.service.impl;

import com.dao.GradeDao;
import com.dao.impl.GradeDaoimp;
import com.entity.Grade;
import com.service.GradeService;

/*
 *
 *
 */
public class GradeServiceimp implements GradeService {
	private GradeDao gradeDao= null;
	
	public GradeServiceimp(){
		gradeDao = new GradeDaoimp();
	}

	@Override
	public Grade getGrade(Grade grade) {
		return gradeDao.getGrade(grade);
	}
	
	public Grade getGrade(String gname){
		return gradeDao.getGrade(gname);
	}

}


