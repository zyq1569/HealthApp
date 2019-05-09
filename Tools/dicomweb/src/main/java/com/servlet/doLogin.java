package com.servlet;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import com.entity.User;
import com.service.UserService;
import com.service.impl.UserServiceimp;

/*
 *
 *
 */
public class doLogin extends HttpServlet {
	private static final long serialVersionUID = -1975709666978641311L;

	public doLogin() {
		super();
	}

	public void destroy() {
		super.destroy(); // Just puts "destroy" string in log
		// Put your code here
	}

	public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		this.doPost(request, response);
 
	}

	public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		String userName = request.getParameter("username");
		String password = request.getParameter("password");
		if(userName!=null && !userName.equals("")){
			UserService uerService = new UserServiceimp();
			User user = new User();
			user.setUsername(userName);
			user.setPassword(password);
			if(uerService.findUser(user)){//存在这个用户，可以正常访问信息
				request.getSession().setAttribute("user", user);
				response.sendRedirect("/dicomweb/pages/stuList.jsp");
				
			}else{//不存在这个用户，给出提示，转回登录页面
				String message = "用户名或密码错误";
				request.getSession().setAttribute("msg", message);
				response.sendRedirect("/dicomweb/login.jsp");
			}
		}
		
	}

	public void init() throws ServletException {
		// Put your code here
	}

}


