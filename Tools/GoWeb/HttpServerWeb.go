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

	// "strings"

	"encoding/json"

	"./Data"
	"./Units"

	// "fmt"
	_ "github.com/go-sql-driver/mysql"
	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"
)

const (
	DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	IMAGE_Dir = "D:/code/C++/HealthApp/bin/win32/DCM_SAVE/Images"
	PAGE_Dir  = "D:/code/C++/HealthApp/Tools/PageWeb"
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
	println(PAGE_Dir + req.URL.Path)
	filepath := PAGE_Dir + req.URL.Path
	return c.File(filepath)
}

func LoadImageFile(c echo.Context) error {
	req := c.Request()
	println("req.URL.Path:" + req.URL.Path)
	println(PAGE_Dir + req.URL.Path)
	filepath := PAGE_Dir + req.URL.Path
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
	//分页查询https://blog.csdn.net/myth_g/article/details/89672722
	startTime := c.FormValue("start")
	endTime := c.FormValue("end")
	page := c.FormValue("page")
	limit := c.FormValue("limit")
	// var mess strings.Builder
	// mess.WriteString("start:")
	// mess.WriteString(startTime)
	// mess.WriteString("end:")
	// mess.WriteString(endTime)
	// mess.WriteString("page:")
	// mess.WriteString(page)
	// mess.WriteString("limit:")
	// mess.WriteString(limit)
	// // mess := "start:" + startTime + "end:" + end + "page:" + page + "limit:" + limit
	// println(mess.String())
	var studyjson Study.StudyDataJson
	if maridb_db != nil {
		var count int
		p, err := strconv.Atoi(page)
		checkErr(err)
		lim, err := strconv.Atoi(limit)
		checkErr(err)
		count = (p - 1) * lim
		var sql string
		sql = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday," +
			" p.PatientSex,s.StudyUID,s.StudyID,s.StudyIdentity,s.StudyDateTime," +
			" s.StudyDescription, s.StudyModality from " +
			" h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity and " +
			" s.StudyDateTime >= " + startTime + " and  s.StudyDateTime <= " + endTime +
			" order by s.PatientIdentity limit " + strconv.Itoa(count) + "," + limit
		println(sql)
		rows, err := maridb_db.Query(sql)
		if err != nil {
			println(err)
		} else {
			studyjson.Code = 0
			studyjson.Msg = ""
			studyjson.Count = 21
			for rows.Next() {
				var data Study.StudyData
				err = rows.Scan(&data.PatientIdentity, &data.PatientName,
					&data.PatientID, &data.PatientBirthday,
					&data.PatientSex, &data.StudyUID, &data.StudyID,
					&data.StudyOrderIdentity, &data.StudyDateTime,
					&data.StudyDescription, &data.StudyModality)
				studyjson.Data = append(studyjson.Data, data)
			}
			sql = "select count(*) count from " +
				"h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity and " +
				" s.StudyDateTime >= " + startTime + " and  s.StudyDateTime <= " + endTime
			rows, err := maridb_db.Query(sql)
			if err == nil {
				for rows.Next() {
					rows.Scan(&studyjson.Count)
				}
			}
		}
	}
	js, err := json.Marshal(studyjson)
	if err != nil {
		println(err)
		return c.String(http.StatusOK, "null")
	}
	println(string(js))
	return c.String(http.StatusOK, string(js))
}

func GetDBStudyData(c echo.Context) error {
	//分页查询https://blog.csdn.net/myth_g/article/details/89672722
	startTime := c.FormValue("start")
	endTime := c.FormValue("end")
	page := c.FormValue("page")
	limit := c.FormValue("limit")
	// var mess strings.Builder
	// mess.WriteString("start:")
	// mess.WriteString(startTime)
	// mess.WriteString("end:")
	// mess.WriteString(endTime)
	// mess.WriteString("page:")
	// mess.WriteString(page)
	// mess.WriteString("limit:")
	// mess.WriteString(limit)
	// // mess := "start:" + startTime + "end:" + end + "page:" + page + "limit:" + limit
	// println(mess.String())
	var studyjson Study.StudyDataJson
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
			" s.StudyOrderIdentity,s.ScheduledDateTime,s.ScheduledDateTime,s.StudyDescription, s.StudyModality, s.StudyCost, s.StudyState " +
			" from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and " +
			" s.ScheduledDateTime>=" + startTime + " and  s.ScheduledDateTime<=" + endTime + " order by s.StudyOrderIdentity " +
			" limit " + strconv.Itoa(count) + "," + limit
		println(sql)
		rows, err := maridb_db.Query(sql)
		if err != nil {
			println(err)
		} else {
			studyjson.Code = 0
			studyjson.Msg = ""
			studyjson.Count = 21
			for rows.Next() {
				var data Study.StudyData
				err = rows.Scan(&data.PatientIdentity, &data.PatientName,
					&data.PatientID, &data.PatientBirthday,
					&data.PatientSex, &data.PatientTelNumber,
					&data.PatientAddr, &data.PatientEmail,
					&data.PatientCarID, &data.StudyID,
					&data.StudyUID, &data.StudyDepart,
					&data.CostType, &data.StudyOrderIdentity,
					&data.ScheduledDateTime, &data.StudyDateTime, &data.StudyDescription,
					&data.StudyModality, &data.StudyCost,
					&data.StudyState)
				studyjson.Data = append(studyjson.Data, data)
			}
			sql = "select count(*) count from " +
				" h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and" +
				"  s.ScheduledDateTime>= " + startTime + " and  s.ScheduledDateTime <= " + endTime
			rows, err := maridb_db.Query(sql)
			if err == nil {
				for rows.Next() {
					rows.Scan(&studyjson.Count)
				}
			}
		}
	}
	js, err := json.Marshal(studyjson)
	if err != nil {
		println(err)
		return c.String(http.StatusOK, "null")
	}
	println(string(js))
	return c.String(http.StatusOK, string(js))
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
		var idd int
		var username string
		var password string
		err = rows.Scan(&idd, &username, &password)
		checkErr(err)
		println(idd)
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
	var hash string = Units.GetStudyHashDir("1.2.826.0.1.3680043.9.7606.20091101104529.177289.200911020017")
	println(hash)
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
	e.GET("healthsystem/ris/stduyimage/", GetDBStudyImage)
	e.GET("/Login/*", Login)
	e.GET("/view/view.html", LoadImageFile)
	e.GET("/healthsystem/*", Healthsystem)
	e.GET("/favicon.ico", func(c echo.Context) error {
		// println("----------favicon.ico--------")
		return c.File("D:/code/C++/HealthApp/Tools/PageWeb/favicon.ico")
	})
	e.Logger.Fatal(e.Start(":9090"))
}

//参考
func queryMySal() {
	var err error
	connect := "f:f@tcp(192.168.1.0:3306)/lr1"
	db, err := sql.Open("mysql", connect)
	if err != nil {
		println("connect mysql failed, address = "+connect, err)
	} else {
		sqlContent := "select real_nm, sex, birth, ext from member where tenant=? and mem_id=?"
		rows, err := db.Query(sqlContent, "ST", "2017")
		if err != nil {
			println(err)
		} else {
			for rows.Next() {
				nm := sql.NullString{String: "", Valid: false}
				sex := sql.NullString{String: "", Valid: false}
				birth := sql.NullString{String: "", Valid: false}
				ext := sql.NullString{String: "", Valid: false}
				err := rows.Scan(&nm, &sex, &birth, &ext) // birth字段在数据库中是空字段
				if err != nil {
					println("d12 error. ", err)
				}
				println(nm.String)
				println(sex.String)
				println(birth.String)
				println(ext.String)
			}
		}
	}
}

// sqlContent := "select real_nm, sex, birth, ext from member where tenant=? and mem_id=?"
// rows, err := db.Query(sqlContent, "ST", "2017")
// stmt, err := maridb_db.Prepare(sql)
// rows, err := stmt.Exec()//https://www.cnblogs.com/jackylee92/p/6209596.html
