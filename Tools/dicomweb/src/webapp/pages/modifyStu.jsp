<%@page import="com.service.impl.StudyServiceimp"%>
<%@page import="com.service.StudyService"%>
<%@page import="com.entity.Study"%>
<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
    <%@ page isELIgnored="false" %>
    <%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>修改患者信息</title>
<link href="${pageContext.request.contextPath }/css/common.css" rel="stylesheet" type="text/css" />
</head>
<body>
<h2>修改患者信息</h2>
<form action="${pageContext.request.contextPath }/servlet/addStudy?opr=modifyStu" method="post">
  <table id="stuDetailList">
  <%
   int id = Integer.parseInt(request.getParameter("id"));
   StudyService studyService = new StudyServiceimp();
   Study stu = new Study();
   stu.setId(id);
   stu = studyService.getStudyMoreInfo(stu);
   %>
  
  <tr>
     <td>姓名：</td>
    <td><input type="text" name="stuName" value="<%=stu.getName() %>"></td>
  </tr>
    <tr>
    <td>性别：</td>
    <td><input type="radio" name="sex" value="男" checked="checked">男<input type="radio" name="sex" value="女">女<input type="radio" name="sex" value="其他">其他</td>
  </tr>
    <tr>
    <td>年龄：</td>
    <td><input type="text" name="stuAge" value="<%=stu.getAge() %>"></td>
  </tr>
    <tr>
    <td>检查类型：</td>
    <td><select name="stuGrade">
         <option value="RIS" >G1</option>
         <option value="US">G2</option>
         <option value="ES">G3</option>
         <option value="other" selected="selected">other</option>
       </select>
    </td>
  </tr>
    <tr>
    <td>检查内容：</td>
    
    <td><textarea name="showProfile" cols="40" rows="6" ><%=stu.getProfile() %></textarea></td>
   </tr>
       <tr>
       <td><input type="hidden" name="stuId" value="<%=id %>"></td>
    <td colspan="2" style="text-align: center"><input type="submit" value="提交信息"></td>
   </tr>
  </table>
</form>

</body>
</html>