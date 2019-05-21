package com.dao.impl;

import java.sql.ResultSet;
import java.sql.SQLException;

import com.dao.BaseDao;
import com.dao.UserDao;
import com.entity.User;

/*
 *
 */
public class UserDaoimp extends BaseDao implements UserDao{

	@Override
	public boolean findUser(User user) {
		boolean flag = true;
		String sql = "select count(1) from h_user where `username`=? and `password`=?";
		Object[] params = {user.getUsername(),user.getPassword()};
		ResultSet rs = this.executeQuerySQL(sql, params);
		try {
			while(rs.next()){
				int num = rs.getInt(1);
				if(num>0){
					System.out.println("存在这个用户");
					flag = true;
				}else{
					System.out.println("不存在这个用户");
					flag = false;
				}
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		
		return flag;
	}
	
	 public static void main(String[] args) { 
     UserDao udao = new	 UserDaoimp(); 
	 User user = new User();
	 user.setUsername("admin");
	 user.setPassword("admin"); 
	 udao.findUser(user); 
	  }
	 
}


