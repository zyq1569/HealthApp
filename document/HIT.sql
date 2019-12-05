-- --------------------------------------------------------
-- 主机:                           127.0.0.1
-- Server version:               10.3.14-MariaDB - mariadb.org binary distribution
-- Server OS:                    Win64
-- HeidiSQL 版本:                  10.1.0.5464
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dumping database structure for hit
CREATE DATABASE IF NOT EXISTS `hit` /*!40100 DEFAULT CHARACTER SET gbk */;
USE `hit`;

-- Dumping structure for table hit.h_modality
CREATE TABLE IF NOT EXISTS `h_modality` (
  `ModalityIdentity` int(11) NOT NULL,
  `ModalityAET` char(50) DEFAULT NULL,
  `ModalityManufacturer` char(50) DEFAULT NULL,
  `ModalityIPAddr` char(50) DEFAULT NULL,
  `ModalityType` char(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='设备基本信息维护表';

-- Data exporting was unselected.
-- Dumping structure for table hit.h_order
CREATE TABLE IF NOT EXISTS `h_order` (
  `StudyOrderIdentity` bigint(64) NOT NULL COMMENT '预约检查主索引',
  `PatientIdentity` bigint(64) NOT NULL COMMENT '患者唯一ID',
  `StudyID` char(50) DEFAULT NULL COMMENT '检查号ID',
  `StudyUID` char(50) DEFAULT NULL COMMENT '检查UID',
  `StudyModality` char(50) DEFAULT NULL COMMENT '检查设备',
  `ScheduledDateTime` datetime DEFAULT NULL COMMENT '预约设备检查时间',
  `StudyType` int(11) DEFAULT 0 COMMENT '0影像设备检查',
  `StudyState` int(11) DEFAULT 0 COMMENT '检查状态：1.预约 2.等待检查 3.已检查 4.诊断 5.报告审核',
  `AETitle` char(50) DEFAULT NULL COMMENT '设备AE',
  `OrderDateTime` datetime DEFAULT NULL COMMENT '登记预约的时间',
  `StudyDescription` char(128) DEFAULT NULL,
  `StudyCode` char(128) DEFAULT '0000'
) ENGINE=InnoDB DEFAULT CHARSET=gbk COMMENT='设备检查预约表';

-- Data exporting was unselected.
-- Dumping structure for table hit.h_patient
CREATE TABLE IF NOT EXISTS `h_patient` (
  `PatientIdentity` bigint(64) unsigned NOT NULL,
  `PatientID` char(50) NOT NULL,
  `PatientName` char(50) DEFAULT NULL,
  `PatientNameEnglish` char(50) DEFAULT NULL,
  `PatientSex` char(10) DEFAULT NULL,
  `PatientBirthday` char(22) DEFAULT NULL,
  `PatientAddr` varchar(50) DEFAULT NULL,
  `PatientCarID` varchar(50) DEFAULT NULL,
  `PatientTelNumber` varchar(50) DEFAULT NULL,
  `PatientType` int(11) DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='患者基本信息';

-- Data exporting was unselected.
-- Dumping structure for table hit.h_report
CREATE TABLE IF NOT EXISTS `h_report` (
  `studyid` int(11) NOT NULL,
  `report` blob DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=gbk COMMENT='患者检查报告内容二进制';

-- Data exporting was unselected.
-- Dumping structure for table hit.h_study
CREATE TABLE IF NOT EXISTS `h_study` (
  `StudyIdentity` bigint(64) unsigned NOT NULL,
  `StudyDcmPatientName` char(50) DEFAULT NULL,
  `StudyDescription` char(50) DEFAULT NULL,
  `PatientIdentity` bigint(64) unsigned NOT NULL,
  `StudyID` char(50) DEFAULT NULL,
  `StudyUID` char(129) NOT NULL,
  `StudyDateTime` datetime(6) DEFAULT NULL,
  `InstitutionName` char(128) DEFAULT NULL,
  `ProcedureStepStartDate` datetime(6) DEFAULT current_timestamp(6),
  `StudyDept` char(50) DEFAULT NULL,
  `StudyModality` char(10) DEFAULT NULL,
  `StudyModalityIdentity` int(11) DEFAULT NULL,
  `StudyState` char(50) DEFAULT 'order',
  `StudyManufacturer` char(50) DEFAULT NULL,
  `AETitle` char(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='患者一次检查基本信息';

-- Data exporting was unselected.
-- Dumping structure for table hit.h_user
CREATE TABLE IF NOT EXISTS `h_user` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `username` varchar(20) NOT NULL,
  `password` varchar(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

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
