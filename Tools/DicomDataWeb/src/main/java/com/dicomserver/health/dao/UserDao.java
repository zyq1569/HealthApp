package com.dicomserver.health.dao;

import com.dicomserver.health.entity.LoginUser;

public interface UserDao {

    public boolean findUser(LoginUser user);

}

