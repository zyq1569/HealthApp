package com.service.impl;

import java.util.List;

import com.dao.StudentDao;
import com.dao.impl.StudentDaoimp;
import com.entity.Student;
import com.service.StudentService;

/*
 *
 *
 */
public class StudentServiceimp implements StudentService{
	private StudentDao studentDao = null;
	
	public StudentServiceimp(){
		studentDao = new StudentDaoimp();
	}

	@Override
	public List<Student> getAllStudent() {
		return studentDao.getAllStudent();
	}

	@Override
	public Student getStudentMoreInfo(Student student) {
		return studentDao.getStudentMoreInfo(student);
	}

	@Override
	public int addStudent(Student student) {
		return studentDao.addStudent(student);
	}

	@Override
	public int delStudent(Student student) {
		return studentDao.delStudent(student);
	}

	@Override
	public int modifyStudent(Student student) {
		return studentDao.modifyStudent(student);
	}
	
	//查找有没有指定的学生
	public boolean findStudent(Student student){
		return studentDao.findStudent(student);
				
	}

}


