#pragma once
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/fileap.h"

DCMTK_OFSTD_EXPORT struct OFHashValue;
//!根据字符计算两个Hash数值
DCMTK_OFSTD_EXPORT OFHashValue CreateHashValue(const char * buffer, unsigned int length);
DCMTK_OFSTD_EXPORT unsigned long studyuid_hash(const char *str);
DCMTK_OFSTD_EXPORT static OFString longToString(unsigned long i);

//验证目录是否存在
DCMTK_OFSTD_EXPORT bool DirectoryExists(const OFString Dir);
//在全路径文件名中提取文件路径
DCMTK_OFSTD_EXPORT OFString ExtractFileDir(const OFString FileName);
//创建一个新目录
DCMTK_OFSTD_EXPORT bool ForceDirectories(const OFString Dir);
DCMTK_OFSTD_EXPORT OFBool CreatDir(OFString dir);
DCMTK_OFSTD_EXPORT OFString GetCurrWorkingDir();