<%@page import="com.entity.Study"%>
<%@page import="com.service.StudyService"%>
<%@page import="com.service.impl.StudyServiceimp"%>
<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ page isELIgnored="false"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>患者详细信息</title>
<link href="${pageContext.request.contextPath }/css/common.css"
	rel="stylesheet" type="text/css" />
</head>
<body>
	<h2>患者详细信息</h2>
	<%
		session.removeAttribute("study");
		String StudyIdentity = request.getParameter("StudyIdentity");
		String PatientID = request.getParameter("PatientID");
		StudyService stuService = new StudyServiceimp();
		Study study = new Study();
		study.setStudyIdentity(StudyIdentity);
		study.setPatientID(PatientID);
		study = stuService.getStudyMoreInfo(study);
		session.setAttribute("study", study);
	%>

	<table id="stuDetailList">
		<tr>
			<td>姓名：</td>
			<td>${study.getPatientName() }</td>
		</tr>
		<tr>
			<td>性别：</td>
			<td>${study.getPatientSex() }</td>
		</tr>
		<tr>
			<td>出生日：</td>
			<td>${study.getPatientBirthday() }</td>
		</tr>
		<tr>
			<td>患者ID：</td>
			<td>${study.getPatientID() }</td>
		</tr>
		<tr>
			<td>检查类型：</td>
			<td>${study.getStudyModality()}</td>
		</tr>
		<tr>
			<td>检查时间：</td>
			<td>${study.getStudyDateTime()}</td>
		</tr>		
		<tr>
			<td>检查内容：</td>
			<td>${study.getInstitutionName() }</td>
		</tr>

	</table>
	<div align="center">
		<a href="stuList.jsp">返回</a>
	</div>
	<div align="center">
		${user.username }:&nbsp;&nbsp;&nbsp;您好，可以返回上頁.
	</div>
</body>

</html>