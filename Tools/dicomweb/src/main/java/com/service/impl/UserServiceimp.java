package com.service.impl;

import com.dao.UserDao;
import com.dao.impl.UserDaoimp;
import com.entity.User;
import com.service.UserService;

/*
 *
 *
 */
public class UserServiceimp implements UserService{
	UserDao userDao = null;
	
	public UserServiceimp(){
		userDao = new UserDaoimp();
	}

	@Override
	public boolean findUser(User user) {
		return userDao.findUser(user);
	}
	

}


