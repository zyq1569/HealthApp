#ifndef UTIL_H
#define UTIL_H
#include<QStringList>

QStringList DirFilename(QString path, QStringList filters);

bool isDirExist(QString fullPath);

//参数说明：
//QString fullFileName;//文件全路径(包含文件名)
bool isFileExist(QString fullFileName);

bool CreatDir(QString fullPath);




























#endif // UTIL_H
