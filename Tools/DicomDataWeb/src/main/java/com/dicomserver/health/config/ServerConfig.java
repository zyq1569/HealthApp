package com.dicomserver.health.config;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public final class ServerConfig {
    private Properties properties;
    private static ServerConfig serverConfig;
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
        return properties.getProperty(key);
    }


//    static final boolean SSL = System.getProperty("ssl") != null;
//    static final int PORT = Integer.parseInt(System.getProperty("port", SSL ? "8443" : "8080"));
//    public  String PROPERTIES_NAME = System.getProperty("user.dir")+"/src/main/resources/application.properties";
//    public  FileInputStream in = null;
//    public String config_port = "";
//
//    public static String file_path = "D:\\";
//    public static String image_path = "D:\\";
//    public static int server_port = 0;
//
//    public  void Init(){
//        if (server_port > 0)
//            return;
//        try {
//            Properties properties = new Properties();
//            in = new FileInputStream(PROPERTIES_NAME);
//            properties.load(in);
//            config_port = properties.getProperty("netty.dicom.port");
//            server_port = Integer.parseInt(config_port);
//            System.out.println("读取配置信息成功！config_port:" + config_port);
//            file_path =   properties.getProperty("netty.dicom.filePath");
//            image_path =  properties.getProperty("netty.dicom.imagePath");
//            System.out.println("读取配置信息成功！file_path:" + file_path);
//        } catch (FileNotFoundException e) {
//                e.printStackTrace();
//            } catch (IOException e) {
//                e.printStackTrace();
//        } finally {
//            if (in != null) {
//                try {
//                    in.close();
//                } catch (Exception e) {
//                    e.printStackTrace();
//                }
//            }
//        }
//    }
//    public int GetPort(){
//        Init();
//        if (server_port > 0)
//            return server_port;
//        return PORT;
//    }
//    public  String StrPort(){
//        Init();
//        if (server_port >0)
//            return config_port;
//        return Integer.toString(PORT);
//    }
//    public String GetFilePath(){
//        Init();
//        return file_path;
//    }
//    public String GetImagePath(){
//        Init();
//        return image_path;
//    }
//    public static boolean GetSSL(){
//        return SSL;
//    }
}
