package com.dicomserver.health.config;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

//driver=com.mysql.cj.jdbc.Driver
//        url=jdbc:mysql://127.0.0.1:3306/hit?useUnicode=true&characterEncoding=UTF-8&serverTimezone=UTC
//        username=root
//        password=root
//        port=8080
//        filePath=D:/code/C++/HealthApp/bin/win32/DCM_SAVE/Images
//        webdir=D:/code/C++/HealthApp/Tools/PageWeb
public final class ServerConfig {
    private Properties properties;
    private static ServerConfig serverConfig;
    public String url = ""; public  String driver = ""; public String username = "";
    public String password = ""; public String port = ""; public String filePath = "";
    public String webdir = "";
    public boolean bparm = false;
    public ServerConfig(){
        String filePath = "application.properties";
        InputStream is = ServerConfig.class.getClassLoader().getResourceAsStream(filePath);
        properties = new Properties();
        try {
            properties.load(is);
            is.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    //创建实例对象单例模式
    public static synchronized ServerConfig getInstance(){
        if(serverConfig == null){
            serverConfig = new ServerConfig();
        }
        return serverConfig;
    }

    public String getString(String key)    {
        if (bparm){
            if (key.equals("url")){
                return  url;
            }
            if (key.equals("driver")){
                return  driver;
            }
            if (key.equals("username")){
                return  username;
            }
            if (key.equals("password")){
                return  password;
            }
            if (key.equals("port")){
                return  port;
            }
            if (key.equals("filePath")){
                return  filePath;
            }
            if (key.equals("webdir")){
                return  webdir;
            }
        }
        return properties.getProperty(key);
    }
}
