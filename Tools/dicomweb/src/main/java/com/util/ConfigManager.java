package com.util;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/*
 *
 *
 */
public class ConfigManager {
	private Properties properties;
	
	private static ConfigManager configManager;
	
	public ConfigManager(){
		String filePath="database.properties";
		InputStream is = ConfigManager.class.getClassLoader().getResourceAsStream(filePath);
		properties = new Properties();
		try {
			properties.load(is);
			is.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}
	
	//创建唯一一个实例对象使用了单例模式
	public static synchronized ConfigManager getInstance(){
		if(configManager == null){
			configManager = new ConfigManager();
		}
		return configManager;
	}
	
	public String getString(String key){
		return properties.getProperty(key);
	}

}


