package com.dao.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.dao.BaseDao;
import com.dao.StudyDao;
import com.entity.Study;

public class StudyDaoimp extends BaseDao implements StudyDao {
	@Override
	public List<Study> getAllStudy() {
		List<Study> list = new ArrayList<Study>();
		/*
		 * String sql =
		 * "select `PatientIdentity`,`PatientName`,`PatientID`,`PatientBirthday`,`PatientSex`,`patientTelNumber` "
		 * + "from h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity";
		 */
		String sql = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber,s.StudyID "
				+ ",s.StudyIdentity,s.StudyDateTime,s.InstitutionName, s.StudyModality from h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity";
		Object[] params = {};
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {
				Study stu = new Study();
				stu.setPatientIdentity(rs.getString("p.PatientIdentity"));
				stu.setPatientName(rs.getString("p.PatientName"));
				stu.setPatientID(rs.getString("p.PatientID"));
				stu.setPatientSex(rs.getString("p.PatientSex"));
				stu.setPatientBirthday(rs.getString("p.PatientBirthday"));
				stu.setPatientTelNumber(rs.getString("p.PatientTelNumber"));
				
				
				stu.setStudyIdentity(rs.getString("s.StudyIdentity"));
				stu.setStudyID(rs.getString("s.StudyID"));
				stu.setStudyDateTime(rs.getString("s.StudyDateTime"));
				stu.setInstitutionName(rs.getString("s.InstitutionName"));
				stu.setStudyModality(rs.getString("s.StudyModality"));
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
		String StudyIdentity = study.getStudyIdentity();
		String PatientID = study.getPatientID();
		// String sql = "select
		// `PatientName`,`PatientIdentity`,`PatientBirthday`,`PatientSex`,`patientTelNumber`
		// from h_patient where `PatientID`=?";
		String sql = "select  * from h_patient where `PatientID`=?";
		Object[] params = { PatientID/* study.getStudyIdentity() */ };
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {
				stu.setPatientBirthday(rs.getString("PatientBirthday"));
				stu.setPatientIdentity(rs.getString("PatientIdentity"));
				stu.setPatientSex(rs.getString("PatientSex"));
				stu.setPatientName(rs.getString("PatientName"));
				stu.setPatientTelNumber(rs.getString("PatientTelNumber"));
				stu.setPatientID(rs.getString("PatientID"));
				stu.setPatientIdentity(rs.getString("PatientIdentity"));
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		sql = "select * from h_study where `StudyIdentity`=?";
		Object[] Identity = { StudyIdentity/* stu.getStudyIdentity() */ };
		rs = this.executeQuerySQL(sql, Identity);
		try {
			while (rs.next()) {
				stu.setStudyID(rs.getString("StudyID"));
				stu.setStudyDateTime(rs.getString("StudyDateTime"));
				stu.setInstitutionName(rs.getString("InstitutionName"));
				stu.setStudyModality(rs.getString("StudyModality"));
				stu.setStudyIdentity(StudyIdentity);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return stu;
	}

	@Override
	public int addStudy(Study study) {
		int row = 0;
		boolean find = false;
		String PatientIdentity = "";
		// String s = study.creatStudyUID();
		if (study.getPatientID() == null || study.getPatientID() == "") {
			study.setPatientID(Long.toUnsignedString(System.currentTimeMillis()));
			if (study.getPatientTelNumber() == null) {
				study.setPatientTelNumber(Long.toUnsignedString(System.currentTimeMillis()));
			}

		} else {
			// 查找是否存在指定的ID
			String findsql = "select  * from h_patient where `PatientID`=?";
			Object[] findparams = { study.getPatientID() };
			ResultSet findrs = this.executeQuerySQL(findsql, findparams);
			try {
				while (findrs.next()) {
					PatientIdentity = findrs.getString("PatientIdentity");
					study.setPatientIdentity(PatientIdentity);
					find = true;
				}
			} catch (SQLException e) {
				e.printStackTrace();
			}
			//
		}
		if (!find) {
			if (PatientIdentity == "") {
				PatientIdentity = study.creatPatientIdentity();
			}
			String sql = "insert into h_patient(`PatientIdentity`,`PatientName`,`PatientBirthday`,`PatientSex`,`PatientID`,`patientTelNumber`) values(?,?,?,?,?,?)";
			Object[] params = { PatientIdentity, study.getPatientName(), study.getPatientBirthday(),
					study.getPatientSex(), study.getPatientID(), study.getPatientTelNumber() };
			row = this.executeUpdateSQL(sql, params);
			if (row > 0) {
				System.out.println("增加患者成功");
			} else {
				System.out.println("增加患者失败");
			}
		}
		String SUID = study.creatStudyUID();
		study.setStudyUID(SUID);
		String ID = study.creatStudyID();
		study.setStudyID(ID);
		String sql = "insert into h_study(`StudyIdentity`,`StudyID`, `StudyUID`,`PatientIdentity`,`StudyDateTime`, `InstitutionName`,`StudyModality`)"
				+ " value (?,?,?,?,?,?,?)";
		Object[] params = { study.creatStudyIdentity(), study.getStudyID(), study.getStudyUID(), PatientIdentity,
				study.getStudyDateTime(), study.getInstitutionName(), study.getStudyModality() };
		int studyrow = this.executeUpdateSQL(sql, params);
		if (studyrow > 0) {
			System.out.println("增加检查成功");
		} else {
			System.out.println("增加检查失败");
		}

		return row;
	}

	@Override
	public int delStudy(Study study) {
		int row = 0;
		String sql = "delete from h_patient where `PatientID`=?";
		Object[] params = { study.getPatientID() };
		row = this.executeUpdateSQL(sql, params);
		if (row > 0) {
			System.out.println("删除患者成功");
		} else {
			System.out.println("删除患者失败");
		}
		return row;
	}

	@Override
	public int modifyStudy(Study study) {
		int row = 0;
		String sql = "update h_patient set `PatientName`=?,`PatientSex`=?,`PatientBirthday`=?,`patientTelNumber`=?  where `PatientID`=?";
		Object[] params = { study.getPatientName(), study.getPatientSex(), study.getPatientBirthday(),
				study.getPatientTelNumber(), study.getPatientID() };
		row = this.executeUpdateSQL(sql, params);
		if (row > 0) {
			System.out.println("修改患者成功");
		} else {
			System.out.println("修改患者失败");
		}

		return row;
	}

	public boolean findStudy(Study study) {
		boolean flag = true;
		int row = 0;
		String sql = "select count(1) from h_patient where  `PatientName`=? and `PatientBirthday`=? and `PatientSex`=? and `PatientID`=? and `patientTelNumber`=?";
		Object[] params = { study.getPatientName(), study.getPatientBirthday(), study.getPatientSex(),
				study.getPatientID(), study.getPatientTelNumber() };
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while (rs.next()) {
				row = rs.getInt(1);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}

		if (row > 0) {
			System.out.println("此患者已经存在");
			flag = true;
		} else {
			System.out.println("没有这个患者");
			flag = false;
		}

		return flag;

	}
}
