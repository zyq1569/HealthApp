#include "units.h"
#include <QDir>
#include <QFileInfo>
//#include <QDebug>
//#include <QString>
//#include <QFile>
//#include <QMessageBox>

bool isDirExist(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
      return true;
    }
    return false;
//    QFileInfo fileInfo(fullPath);
//    if(fileInfo.isDir())
//    {
//      return true;
//    }
//    return false;
}

//参数说明：
//QString fullFileName;//文件全路径(包含文件名)
bool isFileExist(QString fullFileName)
{
    QFileInfo fileInfo(fullFileName);
    if(fileInfo.exists())
    {
        return true;
    }
    return false;
//        QFile file(fullFileName);
//        if(file.exists())
//        {
//            return true;
//        }
//        return false;
}
