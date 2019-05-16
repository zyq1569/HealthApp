<%@page import="com.service.impl.GradeServiceimp"%>
<%@page import="com.service.GradeService"%>
<%@page import="com.entity.Grade"%>
<%@page import="com.entity.Study"%>
<%@page import="com.service.StudyService"%>
<%@page import="com.service.impl.StudyServiceimp"%>
<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
    <%@ page isELIgnored="false" %>
    <%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>患者详细信息</title>
<link href="${pageContext.request.contextPath }/css/common.css" rel="stylesheet" type="text/css" />
</head>
<body>
<h2>患者详细信息</h2>
<%
session.removeAttribute("study");
int id = Integer.parseInt(request.getParameter("id"));
StudyService stuService = new StudyServiceimp();
Study study = new Study();
study.setId(id);
study = stuService.getStudyMoreInfo(study);
Grade grade = new Grade();
GradeService gradeService = new GradeServiceimp();
grade.setGradeId(study.getGradeId());
grade = gradeService.getGrade(grade);
session.setAttribute("study", study);
 %>


<table id="stuDetailList">
  <tr>
    <td>姓名：</td>
    <td>${study.name }</td>
  </tr>
    <tr>
    <td>性别：</td>
    <td>${study.sex }</td>
  </tr>
    <tr>
    <td>年龄：</td>
    <td>${study.age }</td>
  </tr>
    <tr>
    <td>检查类型：</td>
    <td><%=grade.getGradeName() %></td>
  </tr>
    <tr>
    <td>检查内容：</td>
    <td>${study.profile }</td>
   </tr>
</table>

</body>
</html>