-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               10.5.5-MariaDB - mariadb.org binary distribution
-- Server OS:                    Win64
-- HeidiSQL Version:             10.3.0.5771
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dumping database structure for hit
CREATE DATABASE IF NOT EXISTS `hit` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `hit`;

-- Dumping structure for table hit.h_modality
CREATE TABLE IF NOT EXISTS `h_modality` (
  `ModalityIdentity` int(11) NOT NULL,
  `ModalityAET` char(50) DEFAULT '',
  `ModalityManufacturer` char(50) DEFAULT '',
  `ModalityIPAddr` char(50) DEFAULT '',
  `ModalityType` char(50) DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='设备基本信息维护表';

-- Data exporting was unselected.

-- Dumping structure for table hit.h_order
CREATE TABLE IF NOT EXISTS `h_order` (
  `StudyOrderIdentity` bigint(66) NOT NULL COMMENT '预约检查主索引',
  `PatientIdentity` bigint(66) NOT NULL COMMENT '患者唯一ID',
  `StudyID` char(50) DEFAULT '0' COMMENT '检查号ID',
  `StudyUID` char(129) NOT NULL DEFAULT '0' COMMENT '检查UID',
  `StudyModality` char(50) DEFAULT '' COMMENT '检查设备',
  `StudyAge` int(11) DEFAULT -1 COMMENT '检查年龄',
  `ScheduledDateTime` datetime DEFAULT '1900-01-01 00:00:00' COMMENT '预约设备检查时间',
  `AETitle` char(50) DEFAULT '' COMMENT '设备 AE( 设备的唯一名称)',
  `OrderDateTime` datetime DEFAULT current_timestamp() COMMENT '登记预约的时间',
  `StudyDescription` longtext DEFAULT '' COMMENT '检查描述',
  `StudyDepart` char(50) DEFAULT '' COMMENT '检查的科室',
  `StudyCode` char(30) DEFAULT '0000' COMMENT '用于对应设备的检查部位编码使用',
  `StudyCost` varchar(50) DEFAULT '0' COMMENT '检查费用',
  `CostType` char(50) DEFAULT '' COMMENT '检查的费用类型(eg. 公费)',
  `StudyType` int(11) DEFAULT 0 COMMENT '0影像设备检查',
  `StudyState` int(11) DEFAULT 1 COMMENT '检查状态：-1.标记删除 1.预约 2.等待检查 3.已检查 4.诊断 5.报告审核',
  `StudyDateTime` datetime DEFAULT '1900-01-01 00:00:00',
  `InstitutionName` char(128) DEFAULT '' COMMENT '医疗机构信息(eg.XXX医院)',
  `ProcedureStepStartDate` datetime DEFAULT '1900-01-01 00:00:00',
  `StudyModalityIdentity` int(11) DEFAULT -1,
  `StudyManufacturer` char(50) DEFAULT '' COMMENT '设备厂商(eg. GE)',
  `RegisterID` int(11) DEFAULT -1 COMMENT '信息登记员ID:-1: dcm 文件自动登记信息 0: 其它系统登记信息 1：前台登记',
  PRIMARY KEY (`StudyOrderIdentity`),
  KEY `PatientIdentity` (`PatientIdentity`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COMMENT='设备检查预约表';

-- Data exporting was unselected.

-- Dumping structure for table hit.h_patient
CREATE TABLE IF NOT EXISTS `h_patient` (
  `PatientIdentity` bigint(66) unsigned NOT NULL,
  `PatientID` char(50) NOT NULL,
  `PatientName` char(50) DEFAULT 'unknow',
  `PatientNameEnglish` char(50) DEFAULT '""' COMMENT '姓名拼音(提供给设备-设备几乎都不支持汉字)',
  `PatientSex` char(10) DEFAULT 'O' COMMENT 'O :为待补充',
  `PatientBirthday` char(22) DEFAULT '19000101',
  `PatientAddr` varchar(120) DEFAULT '""',
  `PatientEmail` varchar(50) DEFAULT '""',
  `PatientCarID` varchar(50) DEFAULT '""',
  `PatientTelNumber` varchar(50) DEFAULT '""',
  `PatientType` int(11) DEFAULT 0,
  `PatientState` int(11) DEFAULT 0 COMMENT '定义患者状态 小于0为标记删除',
  PRIMARY KEY (`PatientIdentity`),
  KEY `PatientID` (`PatientID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='患者基本信息(对应多个 h_order，定义患者信息唯一信息，检查为1-n个，to do .....增加患者信息变动过程历史)';

-- Data exporting was unselected.

-- Dumping structure for table hit.h_report
CREATE TABLE IF NOT EXISTS `h_report` (
  `ReportIdentity` bigint(20) NOT NULL,
  `StudyOrderIdentity` bigint(20) NOT NULL,
  `ReportState` int(11) DEFAULT -1 COMMENT '报告状态????? 0.空文档 1.草稿 2.加载 3.审核',
  `ReportTemplate` longtext DEFAULT '' COMMENT '报告模板名称',
  `ReportCreatDate` datetime DEFAULT current_timestamp() COMMENT '报告创建时间',
  `ReportWriterID` char(50) DEFAULT '' COMMENT '报告医生ID',
  `ReportCheckID` char(50) DEFAULT '' COMMENT '审核医生ID',
  `ReportCheckDate` datetime DEFAULT current_timestamp() COMMENT '审核时间',
  `ReportContent` longtext DEFAULT '' COMMENT '报告内容',
  `ReportOther` longtext DEFAULT '' COMMENT '报告扩展字符',
  PRIMARY KEY (`ReportIdentity`),
  KEY `StudyOrderIdentity` (`StudyOrderIdentity`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COMMENT='患者检查报告内容二进制';

-- Data exporting was unselected.

-- Dumping structure for table hit.h_study
CREATE TABLE IF NOT EXISTS `h_study` (
  `StudyIdentity` bigint(64) unsigned NOT NULL COMMENT '检查影像主索引',
  `PatientIdentity` bigint(64) unsigned NOT NULL COMMENT '患者ID',
  `StudyDcmPatientName` char(50) DEFAULT '' COMMENT 'dcm患者姓名',
  `StudyDescription` char(50) DEFAULT '""' COMMENT 'dcm检查描述',
  `StudyID` char(50) DEFAULT '""',
  `StudyUID` char(129) NOT NULL COMMENT '检查UID 唯一性',
  `StudyDateTime` datetime(6) DEFAULT '1900-01-01 00:00:00.000000',
  `InstitutionName` char(128) DEFAULT '""' COMMENT '医疗机构信息(eg.XXX医院)',
  `ProcedureStepStartDate` datetime(6) DEFAULT current_timestamp(6),
  `StudyDept` char(50) DEFAULT '""',
  `StudyModality` char(10) DEFAULT '""' COMMENT 'eg. CT',
  `StudyModalityIdentity` int(11) DEFAULT -1,
  `StudyState` char(50) DEFAULT 'order',
  `StudyManufacturer` char(50) DEFAULT '""' COMMENT '设备厂商(eg. GE)',
  `AETitle` char(50) DEFAULT '""' COMMENT '设备 AE( 设备的唯一名称)',
  PRIMARY KEY (`StudyIdentity`),
  KEY `PatientIdentity` (`PatientIdentity`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='患者一次图像检查基本信息(dcm文件中信息，正常情况这些信息都是来至ris登记的信息-worklist，特殊情况下是技师在影像设备上如输入甚至设备默认值)';

-- Data exporting was unselected.

-- Dumping structure for table hit.h_user
CREATE TABLE IF NOT EXISTS `h_user` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `username` varchar(20) NOT NULL,
  `password` varchar(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COMMENT='用户登录账号信息';

-- Data exporting was unselected.

-- Dumping structure for table hit.user
CREATE TABLE IF NOT EXISTS `user` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `username` varchar(20) NOT NULL,
  `password` varchar(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
