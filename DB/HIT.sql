#/////////////////////////////////////////////////////////////////////////////////////
#mysql  table：
CREATE TABLE `h_modality` (
	`ModalityIdentity` INT(11) NOT NULL,
	`ModalityAET` CHAR(50) NULL DEFAULT NULL,
	`ModalityManufacturer` CHAR(50) NULL DEFAULT NULL,
	`ModalityIPAddr` CHAR(50) NULL DEFAULT NULL,
	`ModalityType` CHAR(50) NULL DEFAULT NULL
)
COMMENT='设备基本信息维护表'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
#///////////////////////////////////////////////////////////////////////////////////
CREATE TABLE `h_order` (
	`StudyOrderIdentity` BIGINT(64) NOT NULL COMMENT '预约检查主索引',
	`PatientIdentity` BIGINT(64) NOT NULL COMMENT '患者唯一ID',
	`StudyID` CHAR(50) NULL DEFAULT NULL COMMENT '检查号ID',
	`StudyUID` CHAR(50) NULL DEFAULT NULL COMMENT '检查UID',
	`StudyModality` CHAR(50) NULL DEFAULT NULL COMMENT '检查设备',
	`ScheduledDateTime` DATETIME NULL DEFAULT NULL COMMENT '预约设备检查时间',
	`StudyType` INT(11) NULL DEFAULT '0' COMMENT '0影像设备检查',
	`StudyState` INT(11) NULL DEFAULT '0' COMMENT '检查状态：1.预约 2.等待检查 3.已检查 4.诊断 5.报告审核',
	`AETitle` CHAR(50) NULL DEFAULT NULL COMMENT '设备AE',
	`OrderDateTime` DATETIME NULL DEFAULT NULL COMMENT '登记预约的时间',
	`StudyDescription` CHAR(128) NULL DEFAULT NULL,
	`StudyCode` CHAR(128) NULL DEFAULT '0000'
)
COMMENT='设备检查预约表'
COLLATE='gbk_chinese_ci'
ENGINE=InnoDB
;
#//////////////////////////////////////////////////////////////////////////////
CREATE TABLE `h_patient` (
	`PatientIdentity` BIGINT(64) UNSIGNED NOT NULL,
	`PatientID` CHAR(50) NOT NULL,
	`PatientName` CHAR(50) NULL DEFAULT NULL,
	`PatientNameEnglish` CHAR(50) NULL DEFAULT NULL,
	`PatientSex` CHAR(10) NULL DEFAULT NULL,
	`PatientBirthday` CHAR(22) NULL DEFAULT NULL,
	`PatientAddr` VARCHAR(50) NULL DEFAULT NULL,
	`PatientCarID` VARCHAR(50) NULL DEFAULT NULL,
	`PatientTelNumber` VARCHAR(50) NULL DEFAULT NULL,
	`PatientType` INT(11) NULL DEFAULT '0'
)
COMMENT='患者基本信息'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
#//////////////////////////////////////////////////////////////////////////////////
CREATE TABLE `h_report` (
	`studyid` INT(11) NOT NULL,
	`report` BLOB NULL DEFAULT NULL
)
COMMENT='患者检查报告内容二进制'
COLLATE='gbk_chinese_ci'
ENGINE=InnoDB
;
#/////////////////////////////////////////////////////////////////////////////////////
CREATE TABLE `h_study` (
	`StudyIdentity` BIGINT(64) UNSIGNED NOT NULL,
	`PatientIdentity` BIGINT(64) UNSIGNED NOT NULL,
	`StudyID` CHAR(50) NULL DEFAULT NULL,
	`StudyUID` CHAR(129) NOT NULL,
	`StudyDateTime` DATETIME(6) NULL DEFAULT NULL,
	`InstitutionName` CHAR(128) NULL DEFAULT NULL,
	`ProcedureStepStartDate` DATETIME(6) NULL DEFAULT CURRENT_TIMESTAMP(6),
	`StudyDept` CHAR(50) NULL DEFAULT NULL,
	`StudyModality` CHAR(10) NULL DEFAULT NULL,
	`StudyModalityIdentity` INT(11) NULL DEFAULT NULL,
	`StudyState` CHAR(50) NULL DEFAULT 'order',
	`StudyManufacturer` CHAR(50) NULL DEFAULT NULL,
	`AETitle` CHAR(50) NULL DEFAULT NULL
)
COMMENT='患者一次检查基本信息'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;
#///////////////////////////////////////////////////////////////////////////////////////
CREATE TABLE `h_user` (
	`id` INT(4) NOT NULL AUTO_INCREMENT,
	`username` VARCHAR(20) NOT NULL,
	`password` VARCHAR(20) NOT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
AUTO_INCREMENT=4
;
#////////////////////////////////////////////////////////////////////////////////////////
CREATE TABLE `user` (
	`id` INT(4) NOT NULL AUTO_INCREMENT,
	`username` VARCHAR(20) NOT NULL,
	`password` VARCHAR(20) NOT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
AUTO_INCREMENT=4
;
