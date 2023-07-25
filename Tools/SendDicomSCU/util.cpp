#include "util.h"
#include <QDir>
#include<QDirIterator>
bool isDirExist(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
        return true;
    }
    return false;

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

}

bool CreatDir(QString fullPath)
{
    QDir dir(fullPath); // 注意
    if(dir.exists())
    {
        return true;
    }else{
        dir.setPath("");
        bool ok = dir.mkpath(fullPath);
        return ok;
    }
}

QStringList DirFilename(QString path, QStringList filters)
{
    QStringList filelist;
    QDir dir(path);
    if (!dir.exists())
    {
        return filelist;
    }

    if (filelist.isEmpty())
    {
        filelist<<QString("*.dcm")<<QString("*.DCM");
    }
    QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while (dir_iterator.hasNext())
    {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        filelist.append(absolute_file_path);
    }
    return filelist;
}
