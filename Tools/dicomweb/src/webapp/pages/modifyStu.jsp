<%@page import="com.service.impl.StudyServiceimp"%>
<%@page import="com.service.StudyService"%>
<%@page import="com.entity.Study"%>
<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ page isELIgnored="false"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>修改患者信息</title>
<link href="${pageContext.request.contextPath }/css/common.css"
	rel="stylesheet" type="text/css" />
</head>
<body>
	<h2>修改患者信息</h2>
	<form
		action="${pageContext.request.contextPath }/servlet/addStudy?opr=modifyStu"
		method="post">
		<table id="stuDetailList">
			<%
				String StudyIdentity = request.getParameter("StudyIdentity");
				String PatientID = request.getParameter("PatientID");
				StudyService studyService = new StudyServiceimp();
				Study stu = new Study();
				stu.setPatientID(PatientID);
				stu.setStudyIdentity(StudyIdentity);
				stu = studyService.getStudyMoreInfo(stu);
			%>

			<tr>
				<td>姓名：</td>
				<td><input type="text" name="PatientName" value="<%=stu.getPatientName()%>"></td>
			</tr>
			<tr>
				<td>性别：</td>
				<td><input type="radio"
					name="PatientSex" value="M" checked="checked" id="M">男(M) <input
					type="radio" name="PatientSex" value="F" id="F">女(F) <input
					type="radio" name="PatientSex" value="O">其他(O)
<%-- <script
type="text/javascript">
					/* if (stu.getPatientSex() == "F") */
						$("#F").attr("checked","checked");
						$("#M").removeAttr("checked");
					$("input[name=PatientSex][value="<%=stu.getPatientSex()%>"]").attr("checked",'true');
					$("input[name=PatientSex]:eq(F)").attr("checked",'true');
</script> --%>
				</td>
			</tr>
			<tr>
				<td>出生日：</td>
				<td><input type="text" name="PatientBirthday"
					value="<%=stu.getPatientBirthday()%>"></td>
			</tr>
			<tr>
				<td>检查类型：</td>
				<td><select name="StudyModality">
						<option value="CT">放射影像CT</option>
						<option value="MR">放射影像MR</option>
						<option value="CR">放射影像CR</option>
						<option value="DR">放射影像DR</option>
						<option value="US">超聲US</option>
						<option value="ES">内窺鏡ES</option>
						<option value="other">other</option>
						<option value="<%=stu.getStudyModality()%>" selected="selected"><%=stu.getStudyModality()%>
						</option>
				</select></td>
			</tr>
			<tr>
				<td>检查内容：</td>
				<td><textarea name="InstitutionName" cols="40" rows="6"><%=stu.getStudyDescription()%></textarea></td>
			</tr>
			<tr>
			<td>
<input type="hidden" name="StudyIdentity" value="<%=stu.getStudyIdentity()%>">
<input type="hidden" name="PatientID"  value="<%=stu.getPatientID()%>"> 
		  </td>
				<td colspan="2" style="text-align: center">
				<input type="submit"
					value="提交信息"></td>
			</tr>
		</table>
		<div align="center">
			<a href="stuList.jsp">返回</a>
		</div>
	</form>
</body>
</html>