package com.dicomserver.health.dao;

import com.dicomserver.health.entity.LoginUser;

public interface UserService {
    //查找用户
    public boolean findUser(LoginUser user);
}
