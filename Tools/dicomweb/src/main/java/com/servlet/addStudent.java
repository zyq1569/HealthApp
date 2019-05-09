package com.servlet;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.entity.Grade;
import com.entity.Student;
import com.service.GradeService;
import com.service.StudentService;
import com.service.impl.GradeServiceimp;
import com.service.impl.StudentServiceimp;

/*
 *
 *
 */
public class addStudent extends HttpServlet {

	private static final long serialVersionUID = 5804433309240831094L;

	public addStudent() {
		super();
	}

	public void destroy() {
		super.destroy(); 
	}
	
	public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        this.doPost(request, response);
	}

	public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		String name = request.getParameter("stuName");
		String sex = request.getParameter("sex");
		int age = Integer.parseInt(request.getParameter("stuAge"));
		String gradeName = request.getParameter("stuGrade");
		String profile = request.getParameter("showProfile");
		
		
		GradeService gService = new GradeServiceimp();
		Grade grade = gService.getGrade(gradeName);
		
		Student stu = new Student();
		stu.setName(name);
		stu.setAge(age);
		stu.setSex(sex);
		stu.setProfile(profile);
		stu.setGradeId(grade.getGradeId());
		
		StudentService studentService = new StudentServiceimp();
		
		if(studentService.findStudent(stu)){//已经存在，就不添加
			response.sendRedirect("/dicomweb/pages/stuList.jsp");
			
		}else{//如果不存在，就添加
			
			String opr = request.getParameter("opr");//根据传的opr参数决定是添加还是学生信息
			int n = 0;
			if(opr.equals("addStu")){
				n = studentService.addStudent(stu);
			}else if(opr.equals("modifyStu")){
				int id = Integer.parseInt(request.getParameter("stuId"));
				stu.setId(id);
				n = studentService.modifyStudent(stu);
			}
			
			
			if(n>0){
				response.sendRedirect("/dicomweb/pages/stuList.jsp");
			}else{
				response.sendRedirect("/dicomweb/pages/addStu.jsp");
			}
			
		}
		
		

	}

	public void init() throws ServletException {
	}

}


