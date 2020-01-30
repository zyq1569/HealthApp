// HttpWeb
//go get -u github.com/labstack/echo/...
/*DSN数据源名称
  [username[:password]@][protocol[(address)]]/dbname[?param1=value1¶mN=valueN]
  user@unix(/path/to/socket)/dbname
  user:password@tcp(localhost:5555)/dbname?charset=utf8&autocommit=true
  user:password@tcp([de:ad:be:ef::ca:fe]:80)/dbname?charset=utf8mb4,utf8
  user:password@/dbname
  无数据库: user:password@/
*/

package main

import (
	"database/sql"
	"flag"
	"net/http"
	"strconv"
	"strings"

	// "fmt"
	_ "github.com/go-sql-driver/mysql"
	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"
)

type StudyData struct {
	PatientIdentity, PatientName, PatientID, PatientSex, PatientBirthday                      string
	PatientTelNumber, PatientAddr, PatientCarID, PatientType, PatientEmail                    string
	StudyOrderIdentity, StudyID, StudyUID, ScheduledDateTime, OrderDateTime, StudyDescription string
	StudyModality, AETitle, StudyType, StudyCode, StudyState, StudyCost                       string
	StudyDateTime, sStudyID, StudyDepart, sStudyModality, sStudyUID, CostType                 string
}

const (
	DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
)

var name string
var maridb_db *sql.DB

func init() {
	flag.StringVar(&name, "name", "default", "log in user")
}

func Healthsystem(c echo.Context) error {
	req := c.Request()
	println("req.URL.Path:" + req.URL.Path)
	println("D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path)
	filepath := "D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path
	return c.File(filepath)
}

func Login(c echo.Context) error {
	req := c.Request()
	println("req.URL.Path:" + req.URL.Path)
	println("D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path)
	filepath := "D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path
	return c.File(filepath)
}

func CheckLogin(c echo.Context) error {
	username := c.FormValue("user")
	userpwd := c.FormValue("password")
	println("username:" + username + "/userpwd:" + userpwd)
	// c.Response(http.StatusText())
	//c.SetResponse()
	return c.String(http.StatusOK, "ok")
}

func GetDBStudyImage(c echo.Context) error {
	start := c.FormValue("start")
	end := c.FormValue("password")
	page := c.FormValue("page")
	limit := c.FormValue("limit")

	println("start:" + start + "/end:" + end + "/page:" + page + "/limit:" + limit)

	return c.String(http.StatusOK, "ok")
}

func GetDBStudyData(c echo.Context) error {
	startTime := c.FormValue("start")
	endTime := c.FormValue("password")
	page := c.FormValue("page")
	limit := c.FormValue("limit")

	var mess strings.Builder
	mess.WriteString("start:")
	mess.WriteString(startTime)
	mess.WriteString("end:")
	mess.WriteString(endTime)

	mess.WriteString("page:")
	mess.WriteString(page)
	mess.WriteString("limit:")
	mess.WriteString(limit)

	// mess := "start:" + startTime + "end:" + end + "page:" + page + "limit:" + limit
	println(mess.String())

	if maridb_db != nil {
		var count int
		p, err := strconv.Atoi(page)
		checkErr(err)
		lim, err := strconv.Atoi(limit)
		checkErr(err)
		count = (p - 1) * lim
		var sql string
		sql = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber," +
			" p.PatientAddr, p.PatientEmail, p.PatientCarID, s.StudyID ,s.StudyUID,s.StudyDepart,s.CostType," +
			" s.StudyOrderIdentity,s.ScheduledDateTime,s.StudyDescription, s.StudyModality, s.StudyCost, s.StudyState, s.ScheduledDateTime" +
			" from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and " +
			" s.ScheduledDateTime>=" + startTime + " and  s.ScheduledDateTime<=" + endTime + " order by s.StudyOrderIdentity " +
			" limit " + strconv.Itoa(count) + "," + limit
		rows, err := maridb_db.Query(sql)
		for rows.Next() {

		}
	}
	return c.String(http.StatusOK, "ok")
}

func checkErr(err error) {
	if err != nil {
		panic(err)
	}
}

func checkMariDB(db *sql.DB) {
	//db, err := OpenDB()//sql.Open("mysql", "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8")
	rows, err := db.Query("SELECT * FROM h_user")

	for rows.Next() {
		var id int
		var username string
		var password string
		err = rows.Scan(&id, &username, &password)
		checkErr(err)
		println(id)
		println(username)
		println(password)
	}
}

func OpenDB() (success bool, db *sql.DB) {
	var isOpen bool
	db, err := sql.Open("mysql", DB_Driver)
	if err != nil {
		isOpen = false
	} else {
		isOpen = true
	}
	checkErr(err)
	return isOpen, db
}

func main() {
	maridb_db = nil
	open, db := OpenDB()
	if open == true {
		maridb_db = db
		checkMariDB(db)
	}

	flag.Parse() //暂停获取参数
	e := echo.New()
	e.Use(middleware.LoggerWithConfig(middleware.LoggerConfig{
		Format: "${time_rfc3339_nano} ${remote_ip} ${method} ${uri} ${status}\n",
	}))
	e.POST("/login/checkuser", CheckLogin)
	e.GET("healthsystem/ris/studydata/", GetDBStudyData)
	e.GET("healthsystem/ris/studydata/", GetDBStudyImage)
	e.GET("/Login/*", Login)
	e.GET("/healthsystem/*", Healthsystem)
	e.GET("/favicon.ico", func(c echo.Context) error {
		// println("----------favicon.ico--------")
		return c.File("D:/code/C++/HealthApp/Tools/PageWeb/favicon.ico")
	})
	e.Logger.Fatal(e.Start(":9090"))
}

// stmt, err := maridb_db.Prepare(sql)
// rows, err := stmt.Exec()//https://www.cnblogs.com/jackylee92/p/6209596.html
