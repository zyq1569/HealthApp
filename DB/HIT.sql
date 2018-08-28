-- --------------------------------------------------------
-- 主机:                           127.0.0.1
-- 服务器版本:                        10.3.8-MariaDB - mariadb.org binary distribution
-- 服务器操作系统:                      Win64
-- HeidiSQL 版本:                  9.4.0.5125
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- 导出 HIT 的数据库结构
CREATE DATABASE IF NOT EXISTS `hit` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `HIT`;

-- 导出  表 HIT.h_modality 结构
CREATE TABLE IF NOT EXISTS `h_modality` (
  `ModalityIdentity` int(11) NOT NULL,
  `ModalityAET` char(50) DEFAULT NULL,
  `ModalityManufacturer` char(50) DEFAULT NULL,
  `ModalityIPAddr` char(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='设备基本信息维护表';

-- 正在导出表  HIT.h_modality 的数据：~0 rows (大约)
/*!40000 ALTER TABLE `h_modality` DISABLE KEYS */;
REPLACE INTO `h_modality` (`ModalityIdentity`, `ModalityAET`, `ModalityManufacturer`, `ModalityIPAddr`) VALUES
	(123456, 'GECT1', 'GE', '192.168.22.155');
/*!40000 ALTER TABLE `h_modality` ENABLE KEYS */;

-- 导出  表 HIT.h_patient 结构
CREATE TABLE IF NOT EXISTS `h_patient` (
  `PatientIdentity` int(11) NOT NULL,
  `PatientID` char(50) NOT NULL,
  `PatientName` char(50) NOT NULL,
  `PatientNameEnglish` char(50) NOT NULL,
  `PatientSex` char(10) NOT NULL,
  `PatientBirthday` char(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='患者基本信息';

-- 正在导出表  HIT.h_patient 的数据：~0 rows (大约)
/*!40000 ALTER TABLE `h_patient` DISABLE KEYS */;
REPLACE INTO `h_patient` (`PatientIdentity`, `PatientID`, `PatientName`, `PatientNameEnglish`, `PatientSex`, `PatientBirthday`) VALUES
	(111111, '0827', '测试工作列表', 'TestWorkList', 'M', '19570827');
/*!40000 ALTER TABLE `h_patient` ENABLE KEYS */;

-- 导出  表 HIT.h_study 结构
CREATE TABLE IF NOT EXISTS `h_study` (
  `StudyIdentity` int(11) NOT NULL,
  `StudyID` char(50) DEFAULT NULL,
  `StudyUID` char(255) DEFAULT NULL,
  `PatientIdentity` char(11) DEFAULT NULL,
  `ProcedureStepStartDate` datetime(6) DEFAULT NULL,
  `StudyDept` char(50) DEFAULT NULL,
  `StudyModality` char(50) DEFAULT NULL,
  `StudyModalityIdentity` int(11) DEFAULT NULL,
  `StudyState` char(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='患者一次检查基本信息';

-- 正在导出表  HIT.h_study 的数据：~0 rows (大约)
/*!40000 ALTER TABLE `h_study` DISABLE KEYS */;
REPLACE INTO `h_study` (`StudyIdentity`, `StudyID`, `StudyUID`, `PatientIdentity`, `ProcedureStepStartDate`, `StudyDept`, `StudyModality`, `StudyModalityIdentity`, `StudyState`) VALUES
	(222222, '001', '1.2.225.645345.988434.23546.1', '111111', '2018-08-27 10:20:23.000000', '放射', 'CT', 123456, '0'),
	(222221, '002', '1.2.225.645345.988434.235499.5', '111112', '2018-08-28 16:04:51.000000', '放射', 'CT', 123456, NULL);
/*!40000 ALTER TABLE `h_study` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
