package com.servlet;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.entity.Student;
import com.service.StudentService;
import com.service.impl.StudentServiceimp;

/*
 *
 *
 */
public class delStudent extends HttpServlet {

	private static final long serialVersionUID = 8988759302676261138L;

	public delStudent() {
		super();
	}

	public void destroy() {
		super.destroy(); 
	}
	
	public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        this.doPost(request, response);
	}

	public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		StudentService studentService = new StudentServiceimp();
		
		int id = Integer.parseInt(request.getParameter("id"));
		Student stu = new Student();
		stu.setId(id);

		studentService.delStudent(stu);
		response.sendRedirect("/dicomweb/pages/stuList.jsp");
		
	}

	public void init() throws ServletException {
	}

}


