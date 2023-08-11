ROOTDIR         = $$PWD

win32 {

msvc: DEFINES      += MSVC

}

# EasyLogging++ is set to be thread safe, to not have a default log file, and to not handle crashes
#DEFINES         += ELPP_THREAD_SAFE ELPP_NO_DEFAULT_LOG_FILE ELPP_DISABLE_DEFAULT_CRASH_HANDLING


#build_pass:message($$PWD) # 当前pri文件所在目录
#build_pass:message($$OUT_PWD) # 生成makefile所在目录
#build_pass:message($$_PRO_FILE_) # 包含当前pri的pro所在路径
#build_pass:message($$_PRO_FILE_PWD_) # 包含当前pri的pro所在目录


#define K_PLUGIN_FACTORY_WITH_JSON(name, jsonFile, pluginRegistrations)
#class name : public KPluginFactory
#{
#    Q_OBJECT
#    Q_INTERFACES(KPluginFactory)
#    Q_PLUGIN_METADATA(IID KPluginFactory_iid FILE jsonFile)
#public:
#    explicit name();
#    ~name();
#};

#name::name()
#{
#    pluginRegistrations
#}
#name::~name()
#{}

#build_pass:message($$PWD) # 当前pri文件所在目录
#build_pass:message($$OUT_PWD) # 生成makefile所在目录
#build_pass:message($$_PRO_FILE_) # 包含当前pri的pro所在路径
#build_pass:message($$_PRO_FILE_PWD_) # 包含当前pri的pro所在目录
#https://www.cnblogs.com/codeForFamily/p/qt-creator-ide-source-learn-3-2.html#replace-functions
