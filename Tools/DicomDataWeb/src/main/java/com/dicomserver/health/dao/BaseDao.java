package com.dicomserver.health.dao;

import com.dicomserver.health.config.ServerConfig;

import java.sql.*;

/*
 */
public class BaseDao {
	Connection connection = null;
	PreparedStatement pstm = null;
	ResultSet rs = null;
	
	
	//获取数据库连接
	public boolean getConnection(){
		try {
			Class.forName(ServerConfig.getInstance().getString("driver"));
			String url = ServerConfig.getInstance().getString("url");
			String username = ServerConfig.getInstance().getString("username");
			String password = ServerConfig.getInstance().getString("password");
			
			connection = DriverManager.getConnection(url, username, password);
			
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
			return false;
		} catch (SQLException e) {
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	//对数据库的增加、删除、修改，返回SQL语句影响的行数
	public int executeUpdateSQL(String sql,Object[] params){
		int row = 0;
		if(getConnection()){
			try {
				pstm = connection.prepareStatement(sql);
				for(int i = 0; i < params.length; i++){
					pstm.setObject(i+1, params[i]);
				}
				row = pstm.executeUpdate();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
		return row;
	}
	
	//对数据库的查询，返回查询结果集合
	public ResultSet executeQuerySQL(String sql, Object[] params){
		if(getConnection()){
			try {
				pstm = connection.prepareStatement(sql);
				for(int i = 0; i < params.length; i++){
					pstm.setObject(i+1, params[i]);
				}
				rs = pstm.executeQuery();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
		return rs;
	}
	
	
	//关闭资源
	public void closeAllResource(){
		if(connection != null){
			try {
				connection.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
		if(pstm != null){
			try {
				pstm.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
			
		}
		if(rs != null){
			try {
				rs.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
		
	}
	

}


