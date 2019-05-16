package com.dao.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.dao.BaseDao;
import com.dao.StudyDao;
import com.entity.Study;

/*
 * 	private int PatientIdentity;
	private String PatientName;
	private String PatientID;
	private String PatientSex;
	private String PatientBirthday;
	private String PatientTelNumber;
 */
public class StudyDaoimp extends BaseDao implements StudyDao {
	@Override
	public List<Study> getAllStudy() {
		List<Study> list = new ArrayList<Study>();
		String sql = "select `PatientIdentity`,`PatientName`,`PatientID`,`PatientBirthday`,`PatientSex`,`patientTelNumber` from h_patient ";
		Object[] params = {};
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {
				Study stu = new Study();
				stu.setPatientIdentity(rs.getLong("PatientIdentity"));
				stu.setPatientName(rs.getString("PatientName"));
				stu.setPatientID(rs.getString("PatientID"));
				stu.setPatientSex(rs.getString("PatientSex"));
				stu.setPatientBirthday(rs.getString("PatientBirthday"));
				stu.setPatientTelNumber(rs.getString("PatientTelNumber"));
				list.add(stu);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return list;
	}

	@Override
	public Study getStudyMoreInfo(Study study) {
		Study stu = new Study();
		String sql = "select  `PatientName`,`PatientIdentity`,`PatientBirthday`,`PatientSex`,`patientTelNumber` from h_patient where `PatientID`=?";
		Object[] params = { study.getPatientID() };
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {
				stu.setPatientBirthday(rs.getString("PatientBirthday"));
				stu.setPatientIdentity(rs.getLong("PatientIdentity"));
				stu.setPatientSex(rs.getString("PatientSex"));
				stu.setPatientName(rs.getString("PatientName"));
				stu.setPatientTelNumber(rs.getString("PatientTelNumber"));
				stu.setPatientID(rs.getString("PatientID"));
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return stu;
	}

	@Override
	public int addStudy(Study study) {
		int row = 0;
		String sql = "insert into h_patient(`PatientIdentity`,`PatientName`,`PatientBirthday`,`PatientSex`,`PatientID`,`patientTelNumber`) values(?,?,?,?,?,?)";
		Object[] params = {study.creatPatientIdentity(),study.getPatientName(),study.getPatientBirthday(),study.getPatientSex(),study.getPatientID(),study.getPatientTelNumber()};
		row = this.executeUpdateSQL(sql, params);
		if(row>0){
			System.out.println("增加患者成功");
		}else{
			System.out.println("增加患者失败");
		}
		return row;
	}

	@Override
	public int delStudy(Study study) {
		int row = 0;
		String sql = "delete from h_patient where `PatientID`=?";
		Object[] params = {study.getPatientID()};
		row = this.executeUpdateSQL(sql, params);
		if(row>0){
			System.out.println("删除患者成功");
		}else{
			System.out.println("删除患者失败");
		}
		return row;
	}

	@Override
	public int modifyStudy(Study study) {
		int row = 0;
		String sql = "update h_patient set `PatientName`=?,`PatientSex`=?,`PatientBirthday`=?,`patientTelNumber`=?  where `PatientID`=?";
		Object[] params = {study.getPatientName(),study.getPatientSex(),study.getPatientBirthday(),study.getPatientTelNumber(),study.getPatientID()};
		row = this.executeUpdateSQL(sql, params);
		if(row>0){
			System.out.println("修改患者成功");
		}else{
			System.out.println("修改患者失败");
		}
		
		return row;
	}
	
	
	public boolean findStudy(Study study){
		boolean flag = true;
		int row = 0;
		String sql = "select count(1) from h_patient where  `PatientName`=? and `PatientBirthday`=? and `PatientSex`=? and `PatientID`=? and `patientTelNumber`=?";
		Object[] params = {study.getPatientName(),study.getPatientBirthday(),study.getPatientSex(),study.getPatientID(),study.getPatientTelNumber()};
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
