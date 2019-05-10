package com.entity;
/*
 *
 *
 */
public class Study
{
	private long PatientIdentity;
	private String PatientName;
	private String PatientID;
	private String PatientSex;
	private String PatientBirthday;
	private String PatientTelNumber;
	
	public long creatPatientIdentity()
	{
		return System.currentTimeMillis();
	}
	
	public long getPatientIdentity() {
		return PatientIdentity;
	}
	public void setPatientIdentity(long PatientIdentity) {
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


