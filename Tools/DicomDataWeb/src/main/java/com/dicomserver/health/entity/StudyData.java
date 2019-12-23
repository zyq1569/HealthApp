package com.dicomserver.health.entity;

import java.text.SimpleDateFormat;
import java.util.Date;

public class StudyData {
    //patient table
    private String PatientIdentity;
    private String PatientName;//PatientNameEnglish
    private String PatientID;
    private String PatientSex;
    private String PatientBirthday;
    private String PatientTelNumber;
    private String PatientAddr, PatientCarID, PatientType;// to do creat history table

    //order table ->patient :PatientIdentity
    private String StudyOrderIdentity;
    private String StudyID;
    private String StudyUID;
    private String ScheduledDateTime;
    private String OrderDateTime;
    private String StudyDescription;
    private String StudyModality;
    private String AETitle;
    private String StudyType, StudyCode,StudyState,StudyCost;

    //study image table ->patient :PatientIdentity
    private String StudyDateTime,sStudyID,sStudyModality,sStudyUID;
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

    public String getStudyOrderIdentity() {
        return StudyOrderIdentity;
    }
    public void setStudyOrderIdentity(String StudyOrderIdentity) {
        this.StudyOrderIdentity = StudyOrderIdentity;
    }

    public String getScheduledDateTime() {
        return ScheduledDateTime;
    }
    public void setScheduledDateTime(String ScheduledDateTime) {
        this.ScheduledDateTime = ScheduledDateTime;
    }

    public String getOrderDateTime() {
    return OrderDateTime;
}
    public void setOrderDateTime(String StudyDateTime) {
        this.OrderDateTime = OrderDateTime;
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

    public String getStudyType() {
        return StudyType;
    }
    public void setStudyType(String StudyType) {
        this.StudyType = StudyType;
    }

    public String getStudyCode() {
        return StudyCode;
    }
    public void setStudyCode(String StudyCode) {
        this.StudyCode = StudyCode;
    }

    public String getStudyState() {
        return StudyState;
    }
    public void setStudyState(String StudyState) {
        this.StudyState = StudyState;
    }

    public String getStudyCost() {
        return StudyCost;
    }
    public void setStudyCost(String StudyCost) {
        this.StudyCost = StudyCost;
    }

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

    public String getPatientAddr() {
        return PatientAddr;
    }
    public void setPatientAddr(String PatientAddr) {
        this.PatientAddr = PatientAddr;
    }

    public String getPatientCarID() {
        return PatientCarID;
    }
    public void setPatientCarID(String PatientCarID) {
        this.PatientCarID = PatientCarID;
    }

    public String getPatientType() {
        return PatientType;
    }
    public void setPatientType(String PatientType) {
        this.PatientType = PatientType;
    }

    //----------study table
    public String getStudyDateTime() {
        return StudyDateTime;
    }
    public void setStudyDateTime(String StudyDateTime) {
        this.StudyDateTime = StudyDateTime;
    }

    public String getsStudyID() {
        return sStudyID;
    }
    public void setsStudyID(String sStudyID) {
        this.sStudyID = sStudyID;
    }

    public String getsStudyModality() {
        return sStudyModality;
    }
    public void setsStudyModality(String sStudyModality) {
        this.sStudyModality = sStudyModality;
    }

    public String getsStudyUID() {
        return sStudyUID;
    }
    public void setsStudyUID(String sStudyUID) {
        this.sStudyUID = sStudyUID;
    }
}
