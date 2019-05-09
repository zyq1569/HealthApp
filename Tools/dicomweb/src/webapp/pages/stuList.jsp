<%@page import="com.entity.Grade"%>
<%@page import="com.service.GradeService"%>
<%@page import="com.service.impl.GradeServiceimp"%>
<%@page import="com.entity.Student"%>
<%@page import="java.util.List"%>
<%@page import="com.service.StudentService"%>
<%@page import="com.service.impl.StudentServiceimp"%>
<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ page isELIgnored="false"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>检查列表信息</title>
<link href="${pageContext.request.contextPath }/css/common.css"
	rel="stylesheet" type="text/css" />
<script type="text/javascript">
  function check(){
    var mymessage=confirm("确定要删除吗？");  
    if(mymessage==true){  
       return true;
    }  
    else if(mymessage==false){  
       return false;
    }   
  }
</script>
</head>
<body>
	<div align="right">
		欢迎你，${user.username } &nbsp;&nbsp;&nbsp;<a href="logout.jsp">退出</a>
	</div>
	<h2>检查列表</h2>
	<p id="addStu">
		<a href="addStu.jsp"><input type="button" value="添加患者"></a>
	</p>

	<table>
		<tr>
			<td class="listTable">ID</td>
			<td class="listTable">姓名</td>
			<td class="listTable">性别</td>
			<td class="listTable">年龄</td>
			<td class="listTable">检查类别</td>
			<td id="profile">检查内容</td>
			<td id="operate">操作</td>
		</tr>
		<%--第一种写法，使用JSP原生标签 --%>
		<%
   session.removeAttribute("list");
   StudentService stuService = new StudentServiceimp();
   GradeService gradeService = new GradeServiceimp();
   Grade grade = new Grade();
   List<Student> list = stuService.getAllStudent();
   session.setAttribute("list", list);
   for(Student stu:list){
    grade.setGradeId(stu.getGradeId());
    grade = gradeService.getGrade(grade);
 %>

		<tr>
			<td><%=stu.getId() %></td>
			<td><%=stu.getName() %></td>
			<td><%=stu.getSex() %></td>
			<td><%=stu.getAge() %></td>
			<td><%=grade.getGradeName() %></td>
			<td><%=stu.getProfile() %></td>
			<td><a href="stuDetail.jsp?id=<%=stu.getId() %>">查看&nbsp;&nbsp;</a>
				<a href="modifyStu.jsp?id=<%=stu.getId() %>">修改&nbsp;&nbsp;</a> <a
				href="${pageContext.request.contextPath }/servlet/delStudent?id=<%=stu.getId() %>"
				onclick="return check()">删除&nbsp;&nbsp;</a></td>
		</tr>
		<%} %>


		<%--第二种写法，使用JSTL标签 --%>
		<%--  <%
   StudentService stuService = new StudentServiceimp();
   GradeService gradeService = new GradeServiceimp();
   Grade grade = new Grade();
   List<Student> list = stuService.getAllStudent();
   session.setAttribute("list", list);

 %>
<c:forEach items="${list }" var="stu">
  <tr> 
   <td>${stu.id } </td><td>${stu.name }</td>
   <td>${stu.sex }</td>
   <td>${stu.age }</td>
   <td>${stu.gradeId }</td>
   <td>${stu.profile }</td>
   <td><a href="stuDetail.jsp">查看&nbsp;&nbsp;</a>
   <a href="modifyStu.jsp">修改&nbsp;&nbsp;</a>
   <a href="#">删除&nbsp;&nbsp;</a></td>
  </tr>

</c:forEach> --%>



	</table>

</body>
</html>