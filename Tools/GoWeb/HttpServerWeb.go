// HttpWeb
//go get -u github.com/labstack/echo/...
package main

import (
	"database/sql"
	"flag"
	"fmt"

	_ "github.com/go-sql-driver/mysql"
	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"
)

var name string

func init() {
	flag.StringVar(&name, "name", "default", "log in user")
}

func Login(c echo.Context) error {
	req := c.Request()
	println("req.URL.Path:" + req.URL.Path)
	println("D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path)

	filepath := "D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path
	return c.File(filepath)
}
func checkErr(err error) {
	if err != nil {
		panic(err)
	}
}

func checkMariDB() {
	db, err := sql.Open("mysql", "root:@tcp(127.0.0.1:3306)/hit?charset=utf8")
	rows, err := db.Query("SELECT * FROM h_user")
	for rows.Next() {
		var uid int
		var username string

		err = rows.Scan(&uid, &username)
		checkErr(err)
		fmt.Println(uid)
		fmt.Println(username)
	}
}
func main() {
	flag.Parse() //暂停获取参数
	e := echo.New()
	e.Use(middleware.LoggerWithConfig(middleware.LoggerConfig{
		Format: "${time_rfc3339_nano} ${remote_ip} ${method} ${uri} ${status}\n",
	}))
	e.GET("/Login/*", Login)
	// e.GET("/Login/*", func(c echo.Context) error {
	// 	req := c.Request()
	// 	println("req.URL.Path:" + req.URL.Path)
	// 	println("D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path)

	// 	filepath := "D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path
	// 	return c.File(filepath)

	// })
	e.Logger.Fatal(e.Start(":9090"))
}
