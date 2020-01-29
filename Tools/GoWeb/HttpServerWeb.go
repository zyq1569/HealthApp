// HttpWeb
//go get -u github.com/labstack/echo/...
package main

import (
	"flag"

	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"
)

var name string

func init() {
	flag.StringVar(&name, "name", "default", "log in user")
}

func main() {
	flag.Parse() //暂停获取参数
	e := echo.New()
	e.Use(middleware.LoggerWithConfig(middleware.LoggerConfig{
		Format: "${time_rfc3339_nano} ${remote_ip} ${method} ${uri} ${status}\n",
	}))
	e.GET("/Login/login.html", func(c echo.Context) error {
		// return c.String(http.StatusOK, "------Hello, World!----------")
		return c.File("D:/code/C++/HealthApp/Tools/PageWeb/Login/login.html")
	})
	e.Logger.Fatal(e.Start(":9090"))
}
