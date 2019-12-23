package com.dicomserver.health.dao.impl;

import com.dicomserver.health.dao.BaseDao;
import com.dicomserver.health.dao.StudyDataDao;
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
        /*
         * String sql =
         * "select `PatientIdentity`,`PatientName`,`PatientID`,`PatientBirthday`,`PatientSex`,`patientTelNumber` "
         * + "from h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity";
         */
        String sql = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber,s.StudyID "
                + ",s.StudyOrderIdentity,s.ScheduledDateTime,s.StudyDescription, s.StudyModality from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity";
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

                stu.setStudyOrderIdentity(rs.getString("s.StudyOrderIdentity"));
                stu.setStudyID(rs.getString("s.StudyID"));
                stu.setScheduledDateTime(rs.getString("s.ScheduledDateTime"));
                stu.setStudyDescription(rs.getString("s.StudyDescription"));
                stu.setStudyModality(rs.getString("s.StudyModality"));
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
            study.setPatientID(Long.toUnsignedString(System.currentTimeMillis()));
            if (study.getPatientTelNumber() == null) {
                study.setPatientTelNumber(Long.toUnsignedString(System.currentTimeMillis()));
            }

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
    public int addPatient(StudyData orderStudy){
        int row = 0;
        boolean find = false;
        String PatientIdentity = "";
        if (orderStudy.getPatientID() == null || orderStudy.getPatientID().equals("")) {
            orderStudy.setPatientID(Long.toUnsignedString(System.currentTimeMillis()));
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
        if (find){
            return updatePatient(orderStudy);
        }else {
            if (PatientIdentity == "") {
                PatientIdentity = orderStudy.creatPatientIdentity();
                orderStudy.setPatientIdentity(PatientIdentity);
            }
            String sql = "insert into h_patient(`PatientIdentity`,`PatientName`,`PatientBirthday`,`PatientSex`,`PatientID`,`patientTelNumber`) values(?,?,?,?,?,?)";
            Object[] params = {PatientIdentity, orderStudy.getPatientName(), orderStudy.getPatientBirthday(),
                    orderStudy.getPatientSex(), orderStudy.getPatientID(), orderStudy.getPatientTelNumber()};
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
    public int updatePatient(StudyData orderStudy){
        int row = 0;
        String sql = "update  h_patient set `PatientAddr`=?,`PatientName`=?,`PatientBirthday`=?,`PatientSex`=?,`patientTelNumber`=?,'PatientCarID'=?,'PatientType'=? where `PatientID`=?";
        Object[] params = {orderStudy.getPatientAddr(), orderStudy.getPatientName(), orderStudy.getPatientBirthday(),
                           orderStudy.getPatientSex(), orderStudy.getPatientTelNumber(), orderStudy.getPatientCarID(),
                           orderStudy.getPatientType(), orderStudy.getPatientID()};
        row = this.executeUpdateSQL(sql, params);
        if (row > 0) {
            System.out.println("update患者成功");
        } else {
            System.out.println("update患者失败");
        }
        return row;
    }
    @Override
    public int markPatient(StudyData orderStudy){
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
    public int addOrderStudy(StudyData orderStudy){
        int row = 0;
        String PatientIdentity = orderStudy.getPatientIdentity();

        if (orderStudy.getsStudyUID() == null || orderStudy.getsStudyUID().equals("")){
            String SUID = orderStudy.creatStudyUID();
            orderStudy.setStudyUID(SUID);
        }
        if (orderStudy.getsStudyID() == null || orderStudy.getsStudyID().equals("")){
            String ID = orderStudy.creatStudyID();
            orderStudy.setStudyID(ID);
        }
        String StudyOrderIdentity = orderStudy.creatStudyIdentity();
        orderStudy.setStudyOrderIdentity(StudyOrderIdentity);
        String sql = "insert into h_order (`StudyOrderIdentity`,`StudyID`, `StudyUID`,`PatientIdentity`,`ScheduledDateTime`, `StudyDescription`,`StudyModality`,`StudyCost`,`StudyCode`) "
                + " value(?,?,?,?,?,?,?,?,?)";
        Object[] params = {orderStudy.getStudyOrderIdentity(), orderStudy.getStudyID(), orderStudy.getStudyUID(), PatientIdentity,
                orderStudy.getScheduledDateTime(), orderStudy.getStudyDescription(), orderStudy.getStudyModality(), orderStudy.getStudyCost(),
                orderStudy.getStudyCode()};
        int studyrow = this.executeUpdateSQL(sql, params);
        if (studyrow > 0) {
            System.out.println("增加预约检查成功");
        } else {
            System.out.println("增加预约检查失败");
        }
        return row;
    }
    @Override
    public int updateOrderStudy(StudyData orderStudy){
        int row = 0;
        String StudyOrderIdentity = orderStudy.getStudyOrderIdentity();
        String sql = "update h_order set `ScheduledDateTime`=?, `StudyDescription`=?,`StudyModality`=?,`StudyCost`=?,`StudyCode`=? "
                + " where StudyOrderIdentity=?";
        Object[] params = {orderStudy.getScheduledDateTime(), orderStudy.getStudyDescription(), orderStudy.getStudyModality(), orderStudy.getStudyCost(),
                orderStudy.getStudyCode(),StudyOrderIdentity};
        int studyrow = this.executeUpdateSQL(sql, params);
        if (studyrow > 0) {
            System.out.println("增加预约检查成功");
        } else {
            System.out.println("增加预约检查失败");
        }
        return row;
    }
    @Override
    public int markOrderStudy(StudyData orderStudy){
        int row = 0;
        String StudyOrderIdentity = orderStudy.getStudyOrderIdentity();
        String sql = "update h_order set `StudyState`=? where StudyOrderIdentity=?";
        Object[] params = {"-1",StudyOrderIdentity};
        int studyrow = this.executeUpdateSQL(sql, params);
        if (studyrow > 0) {
            System.out.println("增加预约检查成功");
        } else {
            System.out.println("增加预约检查失败");
        }
        return row;
    }
}
