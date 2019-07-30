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
	
	private String StudyOrderIdentity;
	private String StudyID;
	private String StudyUID;
	private String ScheduledDateTime;
	private String StudyDescription;
	private String StudyModality;
	private String AETitle;
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
		uid = dateFormat.format(date) + Long.toUnsignedString(System.currentTimeMillis()).substring(0, 6);
		return  UID + uid;
	}
	
	public String getStudyIdentity() {
		return StudyOrderIdentity;
	}
	public void setStudyIdentity(String StudyOrderIdentity) {
		this.StudyOrderIdentity = StudyOrderIdentity;
	}
	
	public String getScheduledDateTime() {
		return ScheduledDateTime;
	}
	public void setScheduledDateTime(String ScheduledDateTime) {
		this.ScheduledDateTime = ScheduledDateTime;
	}
	
	public String getStudyUID() {
		return StudyUID;
	}
	public void setStudyUID(String StudyUID) {
		this.StudyUID = StudyUID;
	}
	//
	public String getStudyDescription() {
		return StudyDescription;
	}
	public void setStudyDescription(String StudyDescription) {
		this.StudyDescription = StudyDescription;
	}
	
	public String getStudyModality() {
		return StudyModality;
	}
	public void setStudyModality(String StudyModality) {
		this.StudyModality = StudyModality;
	}
	
	public String getAETitle() {
		return AETitle;
	}
	public void setAETitle(String AETitle) {
		this.AETitle = AETitle;
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


