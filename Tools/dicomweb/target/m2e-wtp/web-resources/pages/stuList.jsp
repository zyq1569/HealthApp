<%@page import="com.entity.Study"%>
<%@page import="java.util.List"%>
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
<title>检查列表信息</title>
<link href="${pageContext.request.contextPath }/css/common.css"
	rel="stylesheet" type="text/css" />
<script type="text/javascript">
	function check() {
		var mymessage = confirm("确定要删除吗？");
		if (mymessage == true) {
			return true;
		} else if (mymessage == false) {
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
			<td class="listTable">患者ID</td>
			<td class="listTable">姓名</td>
			<td class="listTable">性别</td>
			<td class="listTable">生日</td>
			<td class="listTable">检查类别</td>
			<td id="profile">检查内容</td>
			<td id="operate">操作</td>
		</tr>
		<%-- 第一种写法，使用JSP原生标签 --%>
		<%
			session.removeAttribute("list");
			StudyService stuService = new StudyServiceimp();
			List<Study> list = stuService.getAllStudy();
			session.setAttribute("list", list);
			for (Study stu : list) {
		%>
		<tr>
			<td><%=stu.getPatientID()%></td>
			<td><%=stu.getPatientName()%></td>
			<td><%=stu.getPatientSex()%></td>
			<td><%=stu.getPatientBirthday()%></td>
			<td><%=stu.getStudyModality()%></td>
			<td><%=stu.getStudyDescription()%></td>
			<td><a
				href="stuDetail.jsp?StudyIdentity=<%=stu.getStudyIdentity()%>&PatientID=<%=stu.getPatientID()%>">查看&nbsp;&nbsp;</a>
				<a
				href="modifyStu.jsp?StudyIdentity=<%=stu.getStudyIdentity()%>&PatientID=<%=stu.getPatientID()%>">修改&nbsp;&nbsp;</a>
				<a
				href="${pageContext.request.contextPath }/servlet/delStudy?PatientID=<%=stu.getPatientID() %>"
				onclick="return check()">删除&nbsp;&nbsp; </a></td>
		</tr>
		<%
			}
		%>
	</table>

</body>
</html>