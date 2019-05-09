package com.dao.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.dao.BaseDao;
import com.dao.StudentDao;
import com.entity.Student;

/*
 */
public class StudentDaoimp extends BaseDao implements StudentDao {

	@Override
	public List<Student> getAllStudent() {
		List<Student> list = new ArrayList<Student>();
		String sql = "select `id`,`name`,`gradeId`,`age`,`sex`,`profile` from student ";
		Object[] params = {};
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {
				Student stu = new Student();
				stu.setId(rs.getInt("id"));
				stu.setAge(rs.getInt("age"));
				stu.setGradeId(rs.getInt("gradeId"));
				stu.setSex(rs.getString("sex"));
				stu.setName(rs.getString("name"));
				stu.setProfile(rs.getString("profile"));
				list.add(stu);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return list;
	}

	@Override
	public Student getStudentMoreInfo(Student student) {
		Student stu = new Student();
		String sql = "select `name`,`gradeId`,`age`,`sex`,`profile` from student where `id`=?";
		Object[] params = { student.getId() };
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {

				stu.setAge(rs.getInt("age"));
				stu.setGradeId(rs.getInt("gradeId"));
				stu.setSex(rs.getString("sex"));
				stu.setName(rs.getString("name"));
				stu.setProfile(rs.getString("profile"));

			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return stu;
	}

	@Override
	public int addStudent(Student student) {
		int row = 0;
		String sql = "insert into student(`id`,`name`,`age`,`sex`,`gradeId`,`profile`) values(?,?,?,?,?,?)";
		Object[] params = {student.getId(),student.getName(),student.getAge(),student.getSex(),student.getGradeId(),student.getProfile()};
		row = this.executeUpdateSQL(sql, params);
		if(row>0){
			System.out.println("增加患者成功");
		}else{
			System.out.println("增加患者失败");
		}
		return row;
	}

	@Override
	public int delStudent(Student student) {
		int row = 0;
		String sql = "delete from student where `id`=?";
		Object[] params = {student.getId()};
		row = this.executeUpdateSQL(sql, params);
		if(row>0){
			System.out.println("删除患者成功");
		}else{
			System.out.println("删除患者失败");
		}
		return row;
	}

	@Override
	public int modifyStudent(Student student) {
		int row = 0;
		String sql = "update student set `name`=?,`sex`=?,`gradeId`=?,`age`=?,`profile`=?  where `id`=?";
		Object[] params = {student.getName(),student.getSex(),student.getGradeId(),student.getAge(),student.getProfile(),student.getId()};
		row = this.executeUpdateSQL(sql, params);
		if(row>0){
			System.out.println("修改患者成功");
		}else{
			System.out.println("修改患者失败");
		}
		
		return row;
	}
	
	
	public boolean findStudent(Student student){
		boolean flag = true;
		int row = 0;
		String sql = "select count(1) from student where  `name`=? and `age`=? and `sex`=? and `gradeId`=? and `profile`=?";
		Object[] params = {student.getName(),student.getAge(),student.getSex(),student.getGradeId(),student.getProfile()};
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while(rs.next()){
				row = rs.getInt(1);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		
		if(row>0){
			System.out.println("此患者已经存在");
			flag = true;
		}else{
			System.out.println("没有这个患者");
			flag = false;
		}
		
		return flag;
		
	}
}
