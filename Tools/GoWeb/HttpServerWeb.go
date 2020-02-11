// HttpWeb
//go get -u github.com/labstack/echo/...
/*
DSN数据源名称
  [username[:password]@][protocol[(address)]]/dbname[?param1=value1¶mN=valueN]
  user@unix(/path/to/socket)/dbname
  user:password@tcp(localhost:5555)/dbname?charset=utf8&autocommit=true
  user:password@tcp([de:ad:be:ef::ca:fe]:80)/dbname?charset=utf8mb4,utf8
  user:password@/dbname
  无数据库: user:password@/

echo web 参考例子
https://blog.csdn.net/Noob_coder_JZ/article/details/83410095
*/

// HTTP状态码的分类

// 分类	分类描述
// 1** 	信息，服务器收到请求，需要请求者继续执行操作
// 2** 	成功，操作被成功接收并处理
// 3** 	重定向，需要进一步的操作以完成请求
// 4**	客户端错误，请求包含语法错误或无法完成请求
// 5** 	服务器错误，服务器在处理请求的过程中发生了错误

package main

import (
	"errors"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"

	//"./jsonparser"
	// "bytes"
	"database/sql"
	"encoding/json"
	"flag"
	"net/http"

	// "os"
	"strconv"
	"strings"

	// "encoding/json"
	"io/ioutil"

	"./Data"
	"./Units"

	// "fmt"
	// "time"

	_ "github.com/go-sql-driver/mysql"
	"github.com/labstack/echo"

	// "github.com/labstack/echo/middleware"
	log4go "github.com/jeanphorn/log4go"
)

type CustomContext struct {
	echo.Context
}

const (
	// DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	PAGE_TEST = "F:/temp/HealthApp/PageWeb"
	// PAGE_Dir    = "D:/code/C++/HealthApp/Tools/PageWeb"
	TIME_LAYOUT = "2000-01-02 15:04:05"
	PRE_UID     = "1.2.826.0.1.3680043.9.7604."
)

var PAGE_Dir, Web_Port, IMAGE_Dir, MySQL_IP, MySQL_User, MySQL_PWD, MySQL_DBName, DB_Driver string
var name string
var maridb_db *sql.DB

func init() {
	flag.StringVar(&name, "name", "default", "log in user")
}

func main() {
	DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	IMAGE_Dir = "D:/code/C++/HealthApp/bin/win32/DCM_SAVE/Images"
	PAGE_Dir = "D:/code/C++/HealthApp/Tools/PageWeb"
	Web_Port = "9090"
	log4go.LoadConfiguration("./logConfig.json")
	log4go.LOGGER("Test").Info("log4go Test ...")
	for idx, args := range os.Args {
		println("参数"+strconv.Itoa(idx)+":", args)
	}
	//1 mysql: 1 ip 2 name 3 user  4pwd
	//5 page web / 6 port
	//7 studyimage dir
	//8 level
	//var PAGE_Dir, Web_Port, IMAGE_Dir, MySQL_IP, MySQL_User, MySQL_PWD, MySQL_Name string
	if len(os.Args) > 8 {
		// for idx, args := range os.Args {
		// 	//println("参数"+strconv.Itoa(idx)+":", args)
		// }
		// MySQL_IP, MySQL_User, MySQL_PWD, MySQL_Name
		for i := 1; i < 9; i++ {
			println(os.Args[i])
			log4go.Info(os.Args[i])
		}
		MySQL_IP = os.Args[1]
		MySQL_DBName = os.Args[2]
		MySQL_User = os.Args[3]
		MySQL_PWD = os.Args[4]
		//DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
		DB_Driver = MySQL_User + ":" + MySQL_PWD + "@tcp(" + MySQL_IP + ":3306)/" + MySQL_DBName + "?charset=utf8"
		PAGE_Dir = os.Args[5]
		Web_Port = os.Args[6]
		IMAGE_Dir = os.Args[7]
	}
	// var hash string = Units.GetStudyHashDir("1.2.840.113619.2.55.3.604688119.868.1249343483.504")
	// rand.Seed(int64(time.Now().UnixNano()))
	// id := rand.Int()
	// println(strconv.Itoa(id))
	// println(strconv.Itoa(rand.Int()))
	// println(Units.GetCurrentTime())
	exepath, err := GetCurrentPath()
	if err != nil {
		log.Fatal(err)
	} else {
		PAGE_Dir = exepath + "/PageWeb"
		exist, err := PathExists(PAGE_Dir)
		if err != nil {
			println("get dir error![%v]\n", err)
			PAGE_Dir = "D:/code/C++/HealthApp/Tools/PageWeb"
			//log.Fatal(err)
		}
		if exist {
			println("PAGE_Dir:" + PAGE_Dir)
		} else {
			PAGE_Dir = "D:/code/C++/HealthApp/Tools/PageWeb"
			println("use coe page:" + PAGE_Dir)
			//log.Fatal(err)
		}
		println(exepath)
	}
	//

	// println(hash)
	maridb_db = nil
	open, db := OpenDB()
	if open == true {
		maridb_db = db
		// test
		//checkMariDB(db)
	}

	// flag.Parse() //暂停获取参数
	e := echo.New()
	//定义日志属性
	// e.Use(middleware.LoggerWithConfig(middleware.LoggerConfig{
	// 	Format: "${time_rfc3339_nano} ${remote_ip} ${method} ${uri} ${status}\n",
	// }))
	// e.Use(middleware.Logger())
	//login
	e.POST("/login/checkuser", CheckLogin)
	e.GET("/Login/*", Login)
	e.GET("/login/*", Login)

	//ris
	e.GET("/healthsystem/ris/studydata/", GetDBStudyData)
	e.GET("/healthsystem/ris/stduyimage/", GetDBStudyImage)

	//ris//update ->studydata
	e.POST("/healthsystem/ris/updata/", UpdateDBStudyData)
	e.POST("/healthsystem/ris/update/", UpdateDBStudyData)

	//ris/report
	e.POST("/healthsystem/ris/getreportdata", GetReportdata)
	e.POST("/healthsystem/ris/savereportdata/", SaveReportdata)

	e.GET("/healthsystem/*", Healthsystem)

	// view dicom
	e.GET("/view/*", LoadViewPage)

	// other
	e.GET("/favicon.ico", func(c echo.Context) error {
		// println("----------favicon.ico--------")
		return c.File("D:/code/C++/HealthApp/Tools/PageWeb/favicon.ico")
	})

	// load image file
	//需要定义/WADO?*路由??
	e.GET("/*", LoadImageFile) //WADO?*
	e.Logger.Fatal(e.Start(":" + Web_Port))
	// e.Logger.Fatal(e.Start(":9090"))
}

func PathExists(path string) (bool, error) {
	_, err := os.Stat(path)
	if err == nil {
		return true, nil
	}
	if os.IsNotExist(err) {
		return false, nil
	}
	return false, err
}

func GetCurrentPath() (string, error) {
	file, err := exec.LookPath(os.Args[0])
	if err != nil {
		return "", err
	}
	path, err := filepath.Abs(file)
	if err != nil {
		return "", err
	}
	//fmt.Println("path111:", path)
	if runtime.GOOS == "windows" {
		path = strings.Replace(path, "\\", "/", -1)
	}
	//fmt.Println("path222:", path)
	i := strings.LastIndex(path, "/")
	if i < 0 {
		return "", errors.New(`Can't find "/" or "\".`)
	}
	//fmt.Println("path333:", path)
	return string(path[0 : i+1]), nil
}

func Healthsystem(c echo.Context) error {
	req := c.Request()
	// println("req.URL.Path:" + req.URL.Path)
	// println("D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path)
	filepath := "D:/code/C++/HealthApp/Tools/PageWeb" + req.URL.Path
	return c.File(filepath)
}

func Login(c echo.Context) error {
	req := c.Request()
	// println("req.URL.Path:" + req.URL.Path)
	// println(PAGE_Dir + req.URL.Path)
	filepath := PAGE_Dir + req.URL.Path
	return c.File(filepath)
}

func LoadImageFile(c echo.Context) error {
	// println("---LoadImageFile---------")
	studyuid := c.FormValue("studyuid")
	image_hash_dir := Units.GetStudyHashDir(studyuid)
	filepath := IMAGE_Dir + image_hash_dir
	filepath += "/"
	filepath += studyuid
	filetype := c.FormValue("type")
	// println(filetype)
	if filetype == "json" {
		filepath += "/"
		filepath += studyuid
		filepath += ".json"
		return c.File(filepath)
	} else {
		seriesuid := c.FormValue("seriesuid")
		sopinstanceuid := c.FormValue("sopinstanceuid")
		filepath += "/"
		filepath += seriesuid
		filepath += "/"
		filepath += sopinstanceuid
		filepath += ".dcm"
		return c.File(filepath)
	}
}

func LoadViewPage(c echo.Context) error {
	req := c.Request()
	filepath := PAGE_Dir + req.URL.Path
	index := strings.Index(req.URL.Path, ".html?")
	if index > -1 {
		filename := req.URL.Path[0 : index+5]
		filepath = PAGE_Dir + filename
		return c.File(filepath)
	} else {
		return c.File(PAGE_Dir + req.URL.Path)
	}
}

func CheckLogin(c echo.Context) error {
	username := c.FormValue("user")
	userpwd := c.FormValue("password")
	println("username:" + username + "/userpwd:" + userpwd)
	return c.String(http.StatusOK, "ok")
}

func SaveReportdata(c echo.Context) error {
	var reportdata Study.ReportData
	reportdata.ReportIdentity = ""
	var bodyBytes []byte
	if c.Request().Body != nil {
		bodyBytes, _ = ioutil.ReadAll(c.Request().Body)
		/*err := */
		json.Unmarshal(bodyBytes, &reportdata)
		// if reportdata.ReportIdentity == "" {
		// 	println(err)
		// }
	}
	if maridb_db != nil {
		var sqlstr string
		reportIdentity := reportdata.ReportIdentity
		if reportIdentity != "" {
			sqlstr = "update  h_report set `ReportState`=?,`ReportTemplate`=?,`ReportCheckID`=?,`ReportCheckDate`=?,`ReportContent`=?,`ReportOther`=?" +
				" where `ReportIdentity`=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println(sqlstr)
				log.Fatal(perr)
			}
			affect_count, err := stmt.Exec(reportdata.ReportState, reportdata.ReportTemplate, reportdata.ReportCheckID, reportdata.ReportCheckDate, reportdata.ReportContent, reportdata.ReportOther, reportdata.ReportIdentity)
			if err != nil {
				println("affect_count")
				println(affect_count)
				log.Fatal(err)
			} /*else {
				println("affect_count")
				println(affect_count)
			}*/
		} else {
			reportdata.ReportCheckDate = Units.GetCurrentFormatTime()
			reportdata.ReportCreatDate = Units.GetCurrentFormatTime()
			reportdata.ReportTemplate = "0"
			reportdata.ReportWriterID = "0"
			reportdata.ReportCheckID = "0"
			reportdata.ReportOther = "0"
			sqlstr = "insert into h_report (`StudyOrderIdentity`, `ReportIdentity`,`ReportState`,`ReportTemplate`,`ReportCreatDate`," +
				"`ReportWriterID`,`ReportCheckID`, `ReportCheckDate`,`ReportContent`,`ReportOther`) value(?,?,?,?,?,?,?,?,?,?)"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println(sqlstr)
				log.Fatal(perr)
			} /* else {
				println(sqlstr)
			}*/
			reportdata.ReportIdentity = reportdata.StudyOrderIdentity
			affect_count, err := stmt.Exec(reportdata.StudyOrderIdentity, reportdata.ReportIdentity, reportdata.ReportState,
				reportdata.ReportTemplate, reportdata.ReportCreatDate, reportdata.ReportWriterID, reportdata.ReportCheckID, reportdata.ReportCheckDate, reportdata.ReportContent, reportdata.ReportOther)
			if err != nil {
				println(err)
				log.Fatal(err)
			} /*else {
				println("affect_count:")
				println(affect_count)
			}*/
			lastInsertId, err := affect_count.RowsAffected()
			if err != nil {
				println("lastInsertId:")
				println(lastInsertId)
				println(err)
				log.Fatal(err)
			} /*else {
				println("lastInsertId:")
				println(lastInsertId)
			}*/
		}
	}
	return c.String(http.StatusOK, "ok")
}

func GetReportdata(c echo.Context) error {
	var reportdata Study.ReportData
	var bodyBytes []byte
	if c.Request().Body != nil {
		bodyBytes, _ = ioutil.ReadAll(c.Request().Body)
		err := json.Unmarshal(bodyBytes, &reportdata)
		if err != nil {
			println(err)
		}
	}
	reportdata.ReportSave = "false"
	reportdata.ReportChange = "false"
	if maridb_db != nil {
		var sqlstr string
		sqlstr = "select  * from h_report where `ReportIdentity`=" + reportdata.ReportIdentity
		// println(sqlstr)
		rows, err := maridb_db.Query(sqlstr)
		if err != nil {
			println(err)
		} else {
			for rows.Next() {
				// reportdata.ReportWriterID = sql.NullString{String: "", Valid: false}
				// reportdata.ReportCheckID = sql.NullString{String: "", Valid: false}
				// reportdata.ReportOther = sql.NullString{String: "", Valid: false}
				err = rows.Scan(&reportdata.ReportIdentity, &reportdata.StudyOrderIdentity,
					&reportdata.ReportState, &reportdata.ReportTemplate,
					&reportdata.ReportCreatDate, &reportdata.ReportWriterID,
					&reportdata.ReportCheckID, &reportdata.ReportCheckDate,
					&reportdata.ReportContent, &reportdata.ReportOther)
			}
		}
	}
	// // 2-------------------------
	// println("----reportDBdata.ReportContent--------")
	js, err := json.Marshal(reportdata)
	// js, err := json.MarshalIndent(reportdata, "", "  ")
	jsdata := strings.ReplaceAll(string(js), "=", "\\u003d")
	// println(jsdata)
	if err != nil {
		println(err)
	}
	return c.String(http.StatusOK, jsdata)
}

func GetDBStudyImage(c echo.Context) error {
	//分页查询https://blog.csdn.net/myth_g/article/details/89672722
	startTime := c.FormValue("start")
	endTime := c.FormValue("end")
	page := c.FormValue("page")
	limit := c.FormValue("limit")
	var studyjson Study.StudyDataJson
	if maridb_db != nil {
		var count int
		p, err := strconv.Atoi(page)
		checkErr(err)
		lim, err := strconv.Atoi(limit)
		checkErr(err)
		count = (p - 1) * lim
		var sqlstr string
		sqlstr = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday," +
			" p.PatientSex,s.StudyUID,s.StudyID,s.StudyIdentity,s.StudyDateTime," +
			" s.StudyDescription, s.StudyModality from " +
			" h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity and " +
			" s.StudyDateTime >= " + startTime + " and  s.StudyDateTime <= " + endTime +
			" order by s.PatientIdentity limit " + strconv.Itoa(count) + "," + limit
		// println(sqlstr)
		rows, err := maridb_db.Query(sqlstr)
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
			sqlstr = "select count(*) count from " +
				"h_patient p, h_study s where p.PatientIdentity = s.PatientIdentity and " +
				" s.StudyDateTime >= " + startTime + " and  s.StudyDateTime <= " + endTime
			rows, err := maridb_db.Query(sqlstr)
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
	// println(string(js))
	return c.String(http.StatusOK, string(js))
}

func UpdateDBStudyData(c echo.Context) error {
	var studyData Study.StudyData
	var bodyBytes []byte
	if c.Request().Body != nil {
		bodyBytes, _ = ioutil.ReadAll(c.Request().Body)
		err := json.Unmarshal(bodyBytes, &studyData)
		if err != nil {
			println(err)
		}
	}
	PatientIdentity := studyData.PatientIdentity
	PatientID := studyData.PatientID
	// println(string(bodyBytes))
	// println("PatientID:" + PatientID)
	// println("PatientIdentity:" + PatientIdentity)
	if maridb_db != nil {
		var sqlstr string
		if PatientID == "" { //create a study
			PatientID = Units.GetRandUID()
			PatientIdentity = Units.GetRandUID()
			studyData.PatientIdentity = PatientIdentity
			sqlstr = "insert into h_patient(`PatientIdentity`,`PatientName`,`PatientBirthday`,`PatientSex`,`PatientID`," +
				"`patientTelNumber`,`PatientAddr`,`PatientCarID`,`PatientType`,`PatientEmail`) values(?,?,?,?,?,?,?,?,?,?)"
			stmt, err := maridb_db.Prepare(sqlstr)
			if err != nil {
				println("------fail insert into h_patient maridb_db.Prepare--------")
				println("PatientID:" + PatientID)
				println("PatientIdentity:" + PatientIdentity)
				log.Fatal(err)
			} else {
				println("PatientID:" + PatientID)
				println("PatientIdentity:" + PatientIdentity)
			}
			if studyData.PatientType == "" {
				studyData.PatientType = "0"
			}
			affect_count, err := stmt.Exec(PatientIdentity, studyData.PatientName, studyData.PatientBirthday, studyData.PatientSex,
				PatientID, studyData.PatientTelNumber, studyData.PatientAddr, studyData.PatientCarID, studyData.PatientType, studyData.PatientEmail)
			if err != nil {
				println("fail to  insert into h_patient affect_count:")
				log.Print(affect_count)
				log.Fatal(err)
			} else {
				lastInsertId, err := affect_count.RowsAffected()
				if err != nil {
					println("lastInsertId:")
					println(lastInsertId)
					log.Fatal(err)
				} /*else {
					println("--insert into h_patient--lastInsertId:")
					println(lastInsertId)
				}*/
			}
		} else {
			sqlstr = "update  h_patient set `PatientAddr`=?,`PatientName`=?,`PatientBirthday`=?,`PatientSex`=?," +
				" `patientTelNumber`=?,`PatientCarID`=?,`PatientType`=? ,`PatientEmail`=? where `PatientID`=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println("------fail update PatientID:--------")
				println(PatientID)
				log.Fatal(perr)
			} /*else {
				println("------ok maridb_db.Prepare: --------" + sqlstr)
			}*/
			if studyData.PatientType == "" {
				studyData.PatientType = "0"
			}
			affect_count, err := stmt.Exec(studyData.PatientAddr, studyData.PatientName, studyData.PatientBirthday,
				studyData.PatientSex, studyData.PatientTelNumber, studyData.PatientCarID, studyData.PatientType, studyData.PatientEmail, studyData.PatientID)
			if err != nil {
				println("fail to  update PatientI affect_count:")
				log.Print(affect_count)
				log.Fatal(err)
			} /* else {
				println("ok update PatientI affect_count:")
				log.Print(affect_count)
			}*/
		}
		//update order table
		StudyOrderIdentity := studyData.StudyOrderIdentity
		if StudyOrderIdentity == "" {
			StudyOrderIdentity = Units.GetRandUID()
			if studyData.StudyID == "" {
				studyData.StudyID = Units.GetCurrentTime()
				studyData.StudyUID = Units.GetNewStudyUID()
			}
			sqlstr = "insert into h_order (`StudyOrderIdentity`,`StudyID`, `StudyUID`,`PatientIdentity`,`ScheduledDateTime`, `StudyDescription`,`StudyModality`, " +
				" `StudyCost`,`StudyCode`,`StudyDepart`,`CostType`) value(?,?,?,?,?,?,?,?,?,?,?)"
			stmt, err := maridb_db.Prepare(sqlstr)
			if err != nil {
				println("------fail  maridb_db.Prepare(sqlstr) insert into h_order:--------")
				println(StudyOrderIdentity)
				log.Fatal(err)
			}
			affect_count, err := stmt.Exec(StudyOrderIdentity, studyData.StudyID, studyData.StudyUID, studyData.PatientIdentity,
				studyData.ScheduledDate, studyData.StudyDescription, studyData.StudyModality,
				studyData.StudyCost, studyData.StudyCode, studyData.StudyDepart, studyData.CostType)
			if err != nil {
				println(affect_count)
				log.Fatal(err)
			} /*else {
				lastInsertId, err := affect_count.RowsAffected()
				if err != nil {
					log.Fatal(err)
				} else {
					println("--UpdateDBStudyData--lastInsertId:")
					println(lastInsertId)
				}
			}*/
		} else {
			sqlstr = "update h_order set `ScheduledDateTime`=?, `StudyDescription`=?,`StudyModality`=?,`StudyCost`=?,`StudyCode`=? " +
				" ,`StudyDepart`=?,`CostType`=? where StudyOrderIdentity=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println(sqlstr)
				println("StudyOrderIdentity:")
				println(StudyOrderIdentity)
				log.Fatal(perr)
			}
			affect_count, err := stmt.Exec(studyData.ScheduledDate, studyData.StudyDescription, studyData.StudyModality,
				studyData.StudyCost, studyData.StudyCode, studyData.StudyDepart, studyData.CostType, studyData.StudyOrderIdentity)
			if err != nil {
				println("studyData.ScheduledDate" + studyData.ScheduledDate)
				println(affect_count)
				log.Fatal(err)
			} /*else {
				println("affect_count:")
				println(affect_count)
			}*/
		}
	}
	return c.String(http.StatusOK, "OK")
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
		var sqlstr string
		sqlstr = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber," +
			" p.PatientAddr, p.PatientEmail, p.PatientCarID, s.StudyID ,s.StudyUID,s.StudyDepart,s.CostType," +
			" s.StudyOrderIdentity,s.ScheduledDateTime,s.ScheduledDateTime,s.StudyDescription, s.StudyModality, s.StudyCost, s.StudyState " +
			" from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and " +
			" s.ScheduledDateTime>=" + startTime + " and  s.ScheduledDateTime<=" + endTime + " order by s.StudyOrderIdentity " +
			" limit " + strconv.Itoa(count) + "," + limit
		// println(sqlstr)
		rows, err := maridb_db.Query(sqlstr)
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
			sqlstr = "select count(*) count from " +
				" h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and" +
				"  s.ScheduledDateTime>= " + startTime + " and  s.ScheduledDateTime <= " + endTime
			rows, err := maridb_db.Query(sqlstr)
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
	// println(string(js))
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
		// println(idd)
		// println(username)
		// println(password)
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
				// println(nm.String)
				// println(sex.String)
				// println(birth.String)
				// println(ext.String)
			}
		}
	}
}

// sqlContent := "select real_nm, sex, birth, ext from member where tenant=? and mem_id=?"
// rows, err := db.Query(sqlContent, "ST", "2017")
// stmt, err := maridb_db.Prepare(sql)
// rows, err := stmt.Exec()//https://www.cnblogs.com/jackylee92/p/6209596.html
// text1 := "abcdefg"
// fmt.Println(text1[n]) //获取字符串索引位置为n的原始字节，比如a为97
// fmt.Println(text1[n:m]) //截取得字符串索引位置为 n 到 m-1 的字符串
// fmt.Println(text1[n:]) //截取得字符串索引位置为 n 到 len(s)-1 的字符串
// fmt.Println(text1[:m]) //截取得字符串索引位置为 0 到 m-1 的字符串
// fmt.Println(len(text1)) //获取字符串的字节数
// fmt.Println(utf8.RuneCountInString(text1)) //获取字符串字符的个数
// fmt.Println([]rune(text1)) // 将字符串的每一个字节转换为码点值，比如这里会输出[97 98 99 100 101 102 103]
// fmt.Println(string(text1[n])) // 获取字符串索引位置为n的字符值
