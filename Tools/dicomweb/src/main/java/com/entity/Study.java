package com.entity;

import java.text.SimpleDateFormat;
import java.util.Date;

/*
 *
 *
 */
public class Study
{
	private String PatientIdentity;
	private String PatientName;
	private String PatientID;
	private String PatientSex;
	private String PatientBirthday;
	private String PatientTelNumber;
	
	private String StudyIdentity;
	private String StudyID;
	private String StudyUID;
	private String StudyDateTime;
	private String InstitutionName;
	private String StudyModality;
	static String UID = "1.2.826.0.1.3680043.9.7604.";
	
	public String creatPatientIdentity()	{
		return Long.toUnsignedString(System.currentTimeMillis()) ;
	}
	public String creatStudyIdentity()	{
		return Long.toUnsignedString(System.currentTimeMillis()) ;
	}
	public String creatStudyID()	{
		String id = "" ;
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
		//得到long类型当前时间
		long l = System.currentTimeMillis();
		//new日期对象
		Date date = new Date(l);
		id = dateFormat.format(date);
		return id;
	}
	public String creatStudyUID()	{
		String uid = "" ;
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmss.");
		//得到long类型当前时间
		long l = System.currentTimeMillis();
		//new日期对象
		Date date = new Date(l);
		uid = dateFormat.format(date) + Long.toUnsignedString(System.currentTimeMillis());
		return  UID + uid;
	}
	
	public String getStudyIdentity() {
		return StudyIdentity;
	}
	public void setStudyIdentity(String StudyIdentity) {
		this.StudyIdentity = StudyIdentity;
	}
	
	public String getStudyDateTime() {
		return StudyDateTime;
	}
	public void setStudyDateTime(String StudyDateTime) {
		this.StudyDateTime = StudyDateTime;
	}
	
	public String getStudyUID() {
		return StudyUID;
	}
	public void setStudyUID(String StudyUID) {
		this.StudyUID = StudyUID;
	}
	//
	public String getInstitutionName() {
		return InstitutionName;
	}
	public void setInstitutionName(String InstitutionName) {
		this.InstitutionName = InstitutionName;
	}
	
	public String getStudyModality() {
		return StudyModality;
	}
	public void setStudyModality(String StudyModality) {
		this.StudyModality = StudyModality;
	}
	
	//
	public String getStudyID() {
		return StudyID;
	}
	public void setStudyID(String StudyID) {
		this.StudyID = StudyID;
	}
	
	public String getPatientIdentity() {
		return PatientIdentity;
	}
	public void setPatientIdentity(String PatientIdentity) {
		this.PatientIdentity = PatientIdentity;
	}
	
	public String getPatientName() {
		return PatientName;
	}
	public void setPatientName(String name) {
		this.PatientName = name;
	}
	
	public String getPatientID() {
		return PatientID;
	}
	public void setPatientID(String PatientID) {
		this.PatientID =PatientID;
	}
	
	public String getPatientSex() {
		return PatientSex;
	}
	public void setPatientSex(String sex) {
		this.PatientSex = sex;
	}
	
	public String getPatientBirthday() {
		return PatientBirthday;
	}
	public void setPatientBirthday(String birthday) {
		this.PatientBirthday = birthday;
	}
	
	public String getPatientTelNumber() {
		return PatientTelNumber;
	}
	public void setPatientTelNumber(String patientTelNumber) {
		this.PatientTelNumber = patientTelNumber;
	}
	

}


