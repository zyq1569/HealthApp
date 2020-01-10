package com.dicomserver.health.dao.impl;

import com.dicomserver.health.HealthApplication;
import com.dicomserver.health.dao.BaseDao;
import com.dicomserver.health.dao.UserDao;
import com.dicomserver.health.entity.LoginUser;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.sql.ResultSet;
import java.sql.SQLException;

public class UserDaoimp extends BaseDao implements UserDao {
    private static Logger log = LogManager.getLogger(UserDaoimp.class);
    @Override
    public boolean findUser(LoginUser user) {
        boolean flag = true;
        String sql = "select count(1) from h_user where `username`=? and `password`=?";
        Object[] params = {user.getUsername(),user.getPassword()};
        ResultSet rs = this.executeQuerySQL(sql, params);
        try {
            while(rs.next()){
                int num = rs.getInt(1);
                if(num>0){
                    log.info("存在这个用户");
                    flag = true;
                }else{
                    log.error("不存在这个用户",user.getUsername());
                    flag = false;
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return flag;
    }

//    public static void main(String[] args) {
//        UserDao udao = new	 UserDaoimp();
//        LoginUser user = new LoginUser();
//        user.setUsername("admin");
//        user.setPassword("admin");
//        udao.findUser(user);
//    }
}