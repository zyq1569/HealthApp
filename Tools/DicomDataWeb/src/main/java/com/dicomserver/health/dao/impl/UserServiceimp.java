package com.dicomserver.health.dao.impl;

import com.dicomserver.health.dao.UserDao;
import com.dicomserver.health.dao.UserService;
import com.dicomserver.health.entity.LoginUser;

public class UserServiceimp implements UserService {
    UserDao userDao = null;

    public UserServiceimp(){
        userDao = new UserDaoimp();
    }

    @Override
    public boolean findUser(LoginUser user) {
        return userDao.findUser(user);
    }


}
