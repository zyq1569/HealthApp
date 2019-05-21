package com.servlet;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.entity.Study;
import com.service.StudyService;
import com.service.impl.StudyServiceimp;

/*
 *
 *
 */
public class delStudy extends HttpServlet {

	private static final long serialVersionUID = 8988759302676261138L;

	public delStudy() {
		super();
	}

	public void destroy() {
		super.destroy(); 
	}
	
	public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        this.doPost(request, response);
	}

	public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		StudyService studyService = new StudyServiceimp();
		
		//int id = Integer.parseInt(request.getParameter("id"));
		String id = request.getParameter("PatientID");
		Study stu = new Study();
		stu.setPatientID(id);

		studyService.delStudy(stu);
		response.sendRedirect("/dicomweb/pages/stuList.jsp");
		
	}

	public void init() throws ServletException {
	}

}


