<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ page isELIgnored="false"%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>登记患者</title>
<link href="${pageContext.request.contextPath }/css/common.css"
	rel="stylesheet" type="text/css" />
<script type="text/javascript">
  function check() {
		for (var i = 0; i < document.form1.elements.length - 1; i++) {
			if (document.form1.elements[i].value == "" && document.form1.elements[i].name <> "PatientID" ) {
				alert("当前表单不能有空项");
				document.form1.elements[i].focus();
				return false;
			}
		}
		return true;
		}
</script>
</head>
<body>
	<h2>新增患者信息</h2>
	<form
		action="${pageContext.request.contextPath }/servlet/addStudy?opr=addStu"
		name="form1" method="post" onsubmit="return check()">
		<table id="stuDetailList">
			<tr>
				<td>姓名：</td>
				<td><input type="text" name="PatientName"></td>
			</tr>
			<tr>
				<td>性别：</td>
				<td>
				<input type="radio" name="PatientSex" value="M"	checked="checked">男
				<input type="radio" name="PatientSex" value="F">女
				<input type="radio" name="PatientSex" value="O">其他
				</td>
			</tr>
			<tr>
				<td>出生日期：</td>
				<td><input type="text" name="PatientBirthday"></td>
			</tr>
			<tr>
				<td>检查日期：</td>
				<td><input type="text" name="ScheduledDateTime"></td>
			</tr>
						<tr>
				<td>身份證ID：</td>
				<td><input type="text" name="PatientID"></td>
			</tr>
			<tr>
				<td>检查类别：</td>
				<td><select name="StudyModality">
						<option value="CT">放射影像CT</option>
						<option value="MR">放射影像MR</option>
						<option value="DR">放射影像DR</option>
						<option value="CR">放射影像CR</option>
						<option value="US">超声US</option>
						<option value="ES">内窥镜ES</option>
						<option value="other" selected="selected">其它</option>
				</select></td>
			</tr>
			<tr>
				<td>检查内容：</td>
				<td><textarea name="StudyDescription" cols="40" rows="6"></textarea></td>
			</tr>
			<tr>
				<td colspan="2" style="text-align: center"><input type="submit"
					value="提交信息"></td>
			</tr>
		</table>
	</form>

</body>
</html>