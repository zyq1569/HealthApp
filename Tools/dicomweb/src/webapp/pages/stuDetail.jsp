<%@page import="com.service.impl.GradeServiceimp"%>
<%@page import="com.service.GradeService"%>
<%@page import="com.entity.Grade"%>
<%@page import="com.entity.Student"%>
<%@page import="com.service.StudentService"%>
<%@page import="com.service.impl.StudentServiceimp"%>
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
session.removeAttribute("student");
int id = Integer.parseInt(request.getParameter("id"));
StudentService stuService = new StudentServiceimp();
Student student = new Student();
student.setId(id);
student = stuService.getStudentMoreInfo(student);
Grade grade = new Grade();
GradeService gradeService = new GradeServiceimp();
grade.setGradeId(student.getGradeId());
grade = gradeService.getGrade(grade);
session.setAttribute("student", student);
 %>


<table id="stuDetailList">
  <tr>
    <td>姓名：</td>
    <td>${student.name }</td>
  </tr>
    <tr>
    <td>性别：</td>
    <td>${student.sex }</td>
  </tr>
    <tr>
    <td>年龄：</td>
    <td>${student.age }</td>
  </tr>
    <tr>
    <td>检查类型：</td>
    <td><%=grade.getGradeName() %></td>
  </tr>
    <tr>
    <td>检查内容：</td>
    <td>${student.profile }</td>
   </tr>
</table>

</body>
</html>