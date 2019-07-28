#pragma once
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/fileap.h"

DCMTK_OFSTD_EXPORT struct OFHashValue;
//!�����ַ���������Hash��ֵ
DCMTK_OFSTD_EXPORT OFHashValue CreateHashValue(const char * buffer, unsigned int length);
DCMTK_OFSTD_EXPORT unsigned long studyuid_hash(const char *str);
DCMTK_OFSTD_EXPORT static OFString longToString(unsigned long i);

//��֤Ŀ¼�Ƿ����
DCMTK_OFSTD_EXPORT bool DirectoryExists(const OFString Dir);
//��ȫ·���ļ�������ȡ�ļ�·��
DCMTK_OFSTD_EXPORT OFString ExtractFileDir(const OFString FileName);
//����һ����Ŀ¼
DCMTK_OFSTD_EXPORT bool ForceDirectories(const OFString Dir);
DCMTK_OFSTD_EXPORT OFBool CreatDir(OFString dir);
DCMTK_OFSTD_EXPORT OFString GetCurrWorkingDir();