
#include "logging.h"
#include "easylogging++.h"

#include <QApplication>
#include <QDir>

namespace udg {

void setLogDefault()
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    // Values are always std::string
    //defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s} %level %msg");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s} %level %msg");
    // default logger uses default configurations
    el::Loggers::reconfigureLogger("default", defaultConf);
    //LOG(INFO) << "Log using default file";
    // To set GLOBAL configurations you may use
    defaultConf.setGlobally( el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s} %level %msg");
    defaultConf.setGlobally(el::ConfigurationType::Filename, getLogFilePath().toStdString());
    el::Loggers::reconfigureLogger("default", defaultConf);
    INFO_LOG("--------setLogDefault()-----------");
}

void beginLogging()
{
    //QDir logDir = udg::UserLogsPath;
    QString logDir = QDir::currentPath()+"/log";
    QDir qlogDir =  logDir;// udg::UserCurrentAppPath;//use current work path
    if (!qlogDir.exists())
    {
        //logDir.mkpath(udg::UserLogsPath);
        qlogDir.mkpath(logDir);
    }
    setLogDefault();

}

QString getLogFilePath()
{
    return QDir::toNativeSeparators(QDir::currentPath()+"/log/MHealthReport.log");
}

void debugLog(const QString &msg, const QString &file, int line, const QString &function)
{
    LOG(DEBUG) << qPrintable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
}

void infoLog(const QString &msg, const QString&, int, const QString&)
{
    LOG(INFO) << qUtf8Printable(msg);
}

void warnLog(const QString &msg, const QString &file, int line, const QString &function)
{
    LOG(WARNING) << qUtf8Printable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
}

void errorLog(const QString &msg, const QString &file, int line, const QString &function)
{
    LOG(ERROR) << qUtf8Printable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
}

void fatalLog(const QString &msg, const QString &file, int line, const QString &function)
{
    LOG(FATAL) << qUtf8Printable(QString("%1 [ %2:%3 %4 ]").arg(msg).arg(file).arg(line).arg(function));
}

void verboseLog(int vLevel, const QString &msg, const QString&, int, const QString&)
{
    VLOG(vLevel) << qUtf8Printable(msg);
}

void traceLog(const QString &msg, const QString&, int, const QString&)
{
    LOG(TRACE) << qUtf8Printable(msg);
}

}
