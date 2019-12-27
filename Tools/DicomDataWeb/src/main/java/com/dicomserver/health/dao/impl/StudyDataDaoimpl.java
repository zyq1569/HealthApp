package com.dicomserver.health.dao.impl;

import com.dicomserver.health.dao.BaseDao;
import com.dicomserver.health.dao.StudyDataDao;
import com.dicomserver.health.entity.ReportData;
import com.dicomserver.health.entity.StudyData;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

public class StudyDataDaoimpl extends BaseDao implements StudyDataDao {
    @Override //check all dcm image info
    public List<StudyData> getAllStudyImageData() {
        List<StudyData> list = new ArrayList<StudyData>();
        /*
         * String sql =
         * "select `PatientIdentity`,`PatientName`,`PatientID`,`PatientBirthday`,`PatientSex`,`patientTelNumber` "
         * + "from h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity";
         */
        String sql = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber,s.StudyID,s.StudyUID "
                + ",s.StudyIdentity,s.StudyDateTime,s.StudyDescription, s.StudyModality from "
                + "h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity";
        Object[] params = {};
        String str;
        ResultSet rs = this.executeQuerySQL(sql, params);
        try {
            while (rs.next()) {
                StudyData stu = new StudyData();
                stu.setPatientIdentity(rs.getString("p.PatientIdentity"));
                str = rs.getString("p.PatientName");
                if (str.length() < 1)
                    str = "unknow";
                stu.setPatientName(str);
                stu.setPatientID(rs.getString("p.PatientID"));
                stu.setPatientSex(rs.getString("p.PatientSex"));
                stu.setPatientBirthday(rs.getString("p.PatientBirthday"));
                stu.setPatientTelNumber(rs.getString("p.PatientTelNumber"));
                stu.setStudyUID(rs.getString("s.StudyUID"));
                //stu.setStudyIdentity(rs.getString("s.StudyOrderIdentity"));
                stu.setStudyID(rs.getString("s.StudyID"));
                // stu.setScheduledDateTime(rs.getString("s.ScheduledDateTime"));
                stu.setStudyDateTime(rs.getString("s.StudyDateTime"));
                str = rs.getString("s.StudyDescription");
                if (str.length() < 1)
                    str = "unknow";
                stu.setStudyDescription(str);
                str = rs.getString("s.StudyModality");
                if (str.length() < 1)
                    str = "unknow";
                stu.setStudyModality(str);
                list.add(stu);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return list;
    }

    @Override
    public List<StudyData> getAllStudy() {
        List<StudyData> list = new ArrayList<StudyData>();
        String sql = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber," +
                "p.PatientAddr, p.PatientEmail, p.PatientCarID, s.StudyID ,s.StudyUID,s.StudyDepart,s.CostType," +
                "s.StudyOrderIdentity,s.ScheduledDateTime,s.StudyDescription, s.StudyModality, s.StudyCost, s.StudyState, s.ScheduledDateTime" +
                " from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0";
        Object[] params = {};
        ResultSet rs = this.executeQuerySQL(sql, params);
        try {
            while (rs.next()) {
                StudyData stu = new StudyData();
                stu.setPatientIdentity(rs.getString("p.PatientIdentity"));
                stu.setPatientName(rs.getString("p.PatientName"));
                stu.setPatientID(rs.getString("p.PatientID"));
                stu.setPatientSex(rs.getString("p.PatientSex"));
                stu.setPatientBirthday(rs.getString("p.PatientBirthday"));
                stu.setPatientTelNumber(rs.getString("p.PatientTelNumber"));
                stu.setPatientAddr(rs.getString("p.PatientAddr"));
                stu.setPatientEmail(rs.getString("p.PatientEmail"));
                stu.setPatientCarID(rs.getString("p.PatientCarID"));

                stu.setStudyOrderIdentity(rs.getString("s.StudyOrderIdentity"));
                stu.setStudyID(rs.getString("s.StudyID"));
                stu.setScheduledDateTime(rs.getString("s.ScheduledDateTime"));
                stu.setStudyDescription(rs.getString("s.StudyDescription"));
                stu.setStudyModality(rs.getString("s.StudyModality"));
                stu.setStudyDepart(rs.getString("s.StudyDepart"));
                stu.setStudyCost(rs.getString("s.StudyCost"));
                stu.setCostType(rs.getString("s.CostType"));
                stu.setStudyDateTime(rs.getString("s.ScheduledDateTime"));
                stu.setStudyUID(rs.getString("s.StudyUID"));
                stu.setStudyState(rs.getString("s.StudyState"));
                list.add(stu);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return list;
    }

    @Override
    public StudyData getStudyMoreInfo(StudyData study) {
        StudyData stu = new StudyData();
        String StudyIdentity = study.getStudyOrderIdentity();
        String PatientID = study.getPatientID();
        // String sql = "select
        // `PatientName`,`PatientIdentity`,`PatientBirthday`,`PatientSex`,`patientTelNumber`
        // from h_patient where `PatientID`=?";
        String sql = "select  * from h_patient where `PatientID`=?";
        Object[] params = {PatientID/* study.getStudyIdentity() */};
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
        sql = "select * from h_order where `StudyOrderIdentity`=?";
        Object[] Identity = {StudyIdentity/* stu.getStudyIdentity() */};
        rs = this.executeQuerySQL(sql, Identity);
        try {
            while (rs.next()) {
                stu.setStudyID(rs.getString("StudyID"));
                stu.setScheduledDateTime(rs.getString("ScheduledDateTime"));
                stu.setStudyDescription(rs.getString("StudyDescription"));
                stu.setStudyModality(rs.getString("StudyModality"));
                stu.setStudyOrderIdentity(StudyIdentity);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return stu;
    }

    @Override
    public int addStudy(StudyData study) {
        int row = 0;
        boolean find = false;
        String PatientIdentity = "";
        // String s = study.creatStudyUID();
        if (study.getPatientID() == null || study.getPatientID().equals("")) {
            study.setPatientID(study.creatPatientID());
        } else {
            // 查找是否存在指定的ID
            String findsql = "select  * from h_patient where `PatientID`=?";
            Object[] findparams = {study.getPatientID()};
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
            Object[] params = {PatientIdentity, study.getPatientName(), study.getPatientBirthday(),
                    study.getPatientSex(), study.getPatientID(), study.getPatientTelNumber()};
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
        String sql = "insert into h_order(`StudyOrderIdentity`,`StudyID`, `StudyUID`,`PatientIdentity`,`ScheduledDateTime`, `StudyDescription`,`StudyModality`)"
                + " value (?,?,?,?,?,?,?)";
        Object[] params = {study.creatStudyIdentity(), study.getStudyID(), study.getStudyUID(), PatientIdentity,
                study.getScheduledDateTime(), study.getStudyDescription(), study.getStudyModality()};
        int studyrow = this.executeUpdateSQL(sql, params);
        if (studyrow > 0) {
            System.out.println("增加预约检查成功");
        } else {
            System.out.println("增加预约检查失败");
        }

        return row;
    }

    @Override
    public int delStudy(StudyData study) {
        int row = 0;
        String sql = "delete from h_patient where `PatientID`=?";
        Object[] params = {study.getPatientID()};
        row = this.executeUpdateSQL(sql, params);
        if (row > 0) {
            System.out.println("删除预约检查成功");
        } else {
            System.out.println("删除预约检查失败");
        }
        return row;
    }

    @Override
    public int modifyStudy(StudyData study) {
        int row = 0;
        String sql = "update h_patient set `PatientName`=?,`PatientSex`=?,`PatientBirthday`=?,`patientTelNumber`=?  where `PatientID`=?";
        Object[] params = {study.getPatientName(), study.getPatientSex(), study.getPatientBirthday(),
                study.getPatientTelNumber(), study.getPatientID()};
        row = this.executeUpdateSQL(sql, params);
        if (row > 0) {
            System.out.println("修改患者成功");
        } else {
            System.out.println("修改患者失败");
        }

        return row;
    }

    @Override
    public boolean findStudy(StudyData study) {
        boolean flag = true;
        int row = 0;
        String sql = "select count(1) from h_patient where  `PatientName`=? and `PatientBirthday`=? and `PatientSex`=? and `PatientID`=? and `patientTelNumber`=?";
        Object[] params = {study.getPatientName(), study.getPatientBirthday(), study.getPatientSex(),
                study.getPatientID(), study.getPatientTelNumber()};
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

    //patient table
    @Override
    public int addPatient(StudyData orderStudy) {
        int row = 0;
        boolean find = false;
        String PatientIdentity = "";
        if (orderStudy.getPatientID() == null || orderStudy.getPatientID().equals("")) {
            orderStudy.setPatientID(orderStudy.creatPatientID());
        } else {
            // 查找是否存在指定的ID
            String findsql = "select  * from h_patient where `PatientID`=?";
            Object[] findparams = {orderStudy.getPatientID()};
            ResultSet findrs = this.executeQuerySQL(findsql, findparams);
            try {
                while (findrs.next()) {
                    PatientIdentity = findrs.getString("PatientIdentity");
                    orderStudy.setPatientIdentity(PatientIdentity);
                    find = true;
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        if (find) {
            return updatePatient(orderStudy);
        } else {
            if (PatientIdentity.equals("")) {
                PatientIdentity = orderStudy.creatPatientIdentity();
                orderStudy.setPatientIdentity(PatientIdentity);
            }
            String sql = "insert into h_patient(`PatientIdentity`,`PatientName`,`PatientBirthday`,`PatientSex`,`PatientID`," +
                    "`patientTelNumber`,`PatientAddr`,`PatientCarID`,`PatientType`,`PatientEmail`) values(?,?,?,?,?,?,?,?,?,?)";
            Object[] params = {PatientIdentity, orderStudy.getPatientName(), orderStudy.getPatientBirthday(),
                    orderStudy.getPatientSex(), orderStudy.getPatientID(), orderStudy.getPatientTelNumber(),
                    orderStudy.getPatientAddr(), orderStudy.getPatientCarID(), orderStudy.getPatientType(), orderStudy.getPatientEmail()};
            row = this.executeUpdateSQL(sql, params);
            if (row > 0) {
                System.out.println("增加患者成功");
            } else {
                System.out.println("增加患者失败");
            }
            return row;
        }
    }

    @Override
    public int updatePatient(StudyData orderStudy) {
        int row = 0;
        String sql = "update  h_patient set `PatientAddr`=?,`PatientName`=?,`PatientBirthday`=?,`PatientSex`=?,`patientTelNumber`=?,`PatientCarID`=?,`PatientType`=? ,`PatientEmail`=? where `PatientID`=?";
        Object[] params = {orderStudy.getPatientAddr(), orderStudy.getPatientName(), orderStudy.getPatientBirthday(),
                orderStudy.getPatientSex(), orderStudy.getPatientTelNumber(), orderStudy.getPatientCarID(),
                orderStudy.getPatientType(), orderStudy.getPatientEmail(), orderStudy.getPatientID()};
        row = this.executeUpdateSQL(sql, params);
        if (row > 0) {
            System.out.println("update患者成功");
        } else {
            System.out.println("update患者失败");
        }
        return row;
    }

    @Override
    public int markPatient(StudyData orderStudy) {
        int row = 0;
        String sql = "update  h_patient set 'PatientState'=?  where `PatientID`= ? ";
        Object[] params = {"-1", orderStudy.getPatientID()};
        row = this.executeUpdateSQL(sql, params);
        if (row > 0) {
            System.out.println("mark delete患者成功");
        } else {
            System.out.println("mark delete患者失败");
        }
        return row;
    }

    //order table
    @Override
    public int addOrderStudy(StudyData orderStudy) {
        String StudyOrderIdentity = orderStudy.getStudyOrderIdentity();
        if (!StudyOrderIdentity.equals("")) {
            return updateOrderStudy(orderStudy);
        }
        String PatientIdentity = orderStudy.getPatientIdentity();

        if (orderStudy.getsStudyUID() == null || orderStudy.getsStudyUID().equals("")) {
            String SUID = orderStudy.creatStudyUID();
            orderStudy.setStudyUID(SUID);
        }
        if (orderStudy.getsStudyID() == null || orderStudy.getsStudyID().equals("")) {
            String ID = orderStudy.creatStudyID();
            orderStudy.setStudyID(ID);
        }
        StudyOrderIdentity = orderStudy.creatStudyIdentity();
        orderStudy.setStudyOrderIdentity(StudyOrderIdentity);
        String sql = "insert into h_order (`StudyOrderIdentity`,`StudyID`, `StudyUID`,`PatientIdentity`,`ScheduledDateTime`, `StudyDescription`,`StudyModality`, "
                + " `StudyCost`,`StudyCode`,`StudyDepart`,`CostType`) value(?,?,?,?,?,?,?,?,?,?,?)";
        Object[] params = {orderStudy.getStudyOrderIdentity(), orderStudy.getStudyID(), orderStudy.getStudyUID(), PatientIdentity,
                orderStudy.getScheduledDateTime(), orderStudy.getStudyDescription(), orderStudy.getStudyModality(), orderStudy.getStudyCost(),
                orderStudy.getStudyCode(), orderStudy.getStudyDepart(), orderStudy.getCostType()};
        int studyrow = this.executeUpdateSQL(sql, params);
        if (studyrow > 0) {
            System.out.println("增加预约检查成功");
        } else {
            System.out.println("增加预约检查失败");
        }
        return studyrow;
    }

    @Override
    public int updateOrderStudy(StudyData orderStudy) {
        String StudyOrderIdentity = orderStudy.getStudyOrderIdentity();
        String sql = "update h_order set `ScheduledDateTime`=?, `StudyDescription`=?,`StudyModality`=?,`StudyCost`=?,`StudyCode`=? "
                + " ,`StudyDepart`=?,`CostType`=? where StudyOrderIdentity=?";
        Object[] params = {orderStudy.getScheduledDateTime(), orderStudy.getStudyDescription(), orderStudy.getStudyModality(), orderStudy.getStudyCost(),
                orderStudy.getStudyCode(), orderStudy.getStudyDepart(), orderStudy.getCostType(), StudyOrderIdentity};
        int updaterow = this.executeUpdateSQL(sql, params);
        if (updaterow > 0) {
            System.out.println("更新预约检查成功");
        } else {
            System.out.println("更新预约检查失败");
        }
        return updaterow;
    }

    @Override
    public int markOrderStudy(StudyData orderStudy) {
        int row = 0;
        String StudyOrderIdentity = orderStudy.getStudyOrderIdentity();
        String sql = "update h_order set `StudyState`=? where StudyOrderIdentity=?";
        Object[] params = {"-1", StudyOrderIdentity};
        int studyrow = this.executeUpdateSQL(sql, params);
        if (studyrow > 0) {
            System.out.println("增加预约检查成功");
        } else {
            System.out.println("增加预约检查失败");
        }
        return row;
    }

    //deal report table
    public int getStudyReport(ReportData reportData) {
        String reportIdentity = reportData.getReportIdentity();
        if (!reportIdentity.equals("")) {
            // 查找是否存在指定的reportIdentity
            String findsql = "select  * from h_report where `ReportIdentity`=?";
            Object[] findparams = {reportIdentity};
            ResultSet rs = this.executeQuerySQL(findsql, findparams);
            try {
                while (rs.next()) {
                    //`StudyOrderIdentity`, `ReportIdentity`,`ReportState`,`ReportTemplate`,`ReportCreatDate`, `ReportWriterID`,`ReportCheckID`, `ReportCheckDate`,`ReportContent`,`ReportOther`
//                    reportData.setReportIdentity(rs.getString("ReportIdentity"));
                    reportData.setStudyOrderIdentity(rs.getString("StudyOrderIdentity"));
                    reportData.setReportState(rs.getString("ReportState"));
                    reportData.setReportTemplate(rs.getString("ReportTemplate"));
                    reportData.setReportCreatDate(rs.getString("ReportCreatDate"));
                    reportData.setReportWriterID(rs.getString("ReportWriterID"));
                    reportData.setReportCheckID(rs.getString("ReportCheckID"));
                    reportData.setReportCheckDate(rs.getString("ReportCheckDate"));
                    reportData.setReportContent(rs.getString("ReportContent"));
                    reportData.setReportOther(rs.getString("ReportOther"));
                    return 1;
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        return 0;
    }

    public int addStudyReport(ReportData reportData) {
        String studyOrderIdentity = reportData.getStudyOrderIdentity();
        String reportIdentity = reportData.getReportIdentity();
        if (!reportIdentity.equals("")) {
            // 查找是否存在指定的reportIdentity
            String findsql = "select  * from h_report where `ReportIdentity`=?";
            Object[] findparams = {reportIdentity};
            ResultSet findrs = this.executeQuerySQL(findsql, findparams);
            try {
                while (findrs.next()) {
                    return updateStudyReport(reportData);
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
            return 0;
        }
        if (!studyOrderIdentity.equals("")) {
            if (reportIdentity.equals("")) {
                reportData.setReportIdentity(studyOrderIdentity);
            }
            String sql = "insert into h_report (`StudyOrderIdentity`, `ReportIdentity`,`ReportState`,`ReportTemplate`,`ReportCreatDate`, `ReportWriterID`,`ReportCheckID`, "
                    + " `ReportCheckDate`,`ReportContent`,`ReportOther`) value(?,?,?,?,?,?,?,?,?,?)";
            Object[] params = {reportData.getStudyOrderIdentity(), reportData.getReportIdentity(), reportData.getReportState(),
                    reportData.getReportTemplate(), reportData.getReportCreatDate(), reportData.getReportWriterID(), reportData.getReportCheckID(),
                    reportData.getReportCheckDate(), reportData.getReportContent(), reportData.getReportOther()};
            int reportrow = this.executeUpdateSQL(sql, params);
            if (reportrow > 0) {
                System.out.println("add 报告成功");
                return reportrow;
            }
        }
        System.out.println("add 报告失败");
        return 0;
    }

    public int updateStudyReport(ReportData reportData) {
        int row = 0;
        String sql = "update  h_report set `ReportState`=?,`ReportTemplate`=?,`ReportCheckID`=?,`ReportCheckDate`=?,`ReportContent`=?,`ReportOther`=?," +
                "  where `ReportIdentity`=?";
        Object[] params = {reportData.getReportState(), reportData.getReportTemplate(), reportData.getReportCheckID(),
                reportData.getReportCheckDate(), reportData.getReportContent(), reportData.getReportOther(), reportData.getReportIdentity()};
        row = this.executeUpdateSQL(sql, params);
        if (row > 0) {
            System.out.println("update报告成功");
        } else {
            System.out.println("update报告失败");
        }
        return row;
    }

    public int markStudyReport(ReportData reportData) {
        return 0;
    }
}
