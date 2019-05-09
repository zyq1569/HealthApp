package com.dao;

import java.util.List;

import com.entity.Student;

/*
 *
 *
 */
public interface StudentDao {
	public List<Student> getAllStudent();
	public Student getStudentMoreInfo(Student student);
	public int addStudent(Student student);
	public int delStudent(Student student);
	public int modifyStudent(Student student);
	public boolean findStudent(Student student);

}


