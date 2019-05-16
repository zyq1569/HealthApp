package com.servlet;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.entity.Grade;
import com.entity.Study;
import com.service.GradeService;
import com.service.StudyService;
import com.service.impl.GradeServiceimp;
import com.service.impl.StudyServiceimp;

/*
 *
 *	private long PatientIdentity;
	private String PatientName;
	private String PatientID;
	private String PatientSex;
	private String PatientBirthday;
	private String PatientTelNumber;
 *
 */
public class addStudy extends HttpServlet {

	private static final long serialVersionUID = 5804433309240831094L;

	public addStudy() {
		super();
	}

	public void destroy() {
		super.destroy(); 
	}
	
	public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        this.doPost(request, response);
	}

	public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		String PatientName = request.getParameter("PatientName");
		String PatientSex = request.getParameter("PatientSex");
		String PatientBirthday = request.getParameter("PatientBirthday");//Integer.parseInt(
		String PatientID = request.getParameter("PatientID");
		String PatientTelNumber = request.getParameter("PatientTelNumber");
		
		
		//GradeService gService = new GradeServiceimp();
		//Grade grade = gService.getGrade(gradeName);
		
		Study stu = new Study();
		stu.setPatientName(PatientName);
		stu.setPatientBirthday(PatientBirthday);
		stu.setPatientSex(PatientSex);
		stu.setPatientTelNumber(PatientTelNumber);
		stu.setPatientID(PatientID);
		
		StudyService studyService = new StudyServiceimp();
		
		if(studyService.findStudy(stu)){//已经存在，就不添加
			response.sendRedirect("/dicomweb/pages/stuList.jsp");
			
		}else{//如果不存在，就添加
			
			String opr = request.getParameter("opr");//根据传的opr参数决定是添加还是修改信息
			int n = 0;
			if(opr.equals("addStu")){
				n = studyService.addStudy(stu);
			}else if(opr.equals("modifyStu")){
				String id = request.getParameter("PatientID");//Integer.parseInt
				stu.setPatientID(id);
				n = studyService.modifyStudy(stu);
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


