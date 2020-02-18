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
	// "log"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"

	//"./jsonparser"
	// "bytes"
	"database/sql"
	"encoding/json"

	// "flag"
	"net/http"

	// "os"
	"strconv"
	"strings"

	// "encoding/json"
	"io/ioutil"

	"./Data"
	"./Units"

	// "fmt"
	"time"

	_ "github.com/go-sql-driver/mysql"
	"github.com/labstack/echo"

	log4go "github.com/jeanphorn/log4go"
	// "github.com/labstack/echo/middleware"
)

const (
	// DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	PAGE_TEST = "F:/temp/HealthApp/PageWeb"
	// PAGE_Dir    = "D:/code/C++/HealthApp/Tools/PageWeb"
	TIME_LAYOUT = "2000-01-02 15:04:05"
	PRE_UID     = "1.2.826.0.1.3680043.9.7604."

	//size
	CONGIG_SIZE = 9
	//---------------------
	MySQL_IP     = 1
	MySQL_DBName = 2
	MySQL_User   = 3
	MySQL_PWD    = 4
	PAGE_Dir     = 5
	Web_Port     = 6
	IMAGE_Dir    = 7
	LOG_Level    = 8
	//DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	//DB_Driver = MySQL_User + ":" + MySQL_PWD + "@tcp(" + MySQL_IP + ":3306)/" + MySQL_DBName + "?charset=utf8"
	//--------------------
)

var CONFIG [CONGIG_SIZE]string
var DB_Driver string

// var name string
var maridb_db *sql.DB

func main() {
	DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	CONFIG[IMAGE_Dir] = "F:/temp/HealthApp/DCM_SAVE/DCM_SAVE/Images"
	CONFIG[PAGE_Dir] = "F:/temp/HealthApp/PageWeb/PageWeb"
	CONFIG[Web_Port] = "9090"
	log4go.LoadConfiguration("./logConfig.json") // to do set ?
	//1 mysql: 1 ip 2 name 3 user  4pwd	//5 page web / 6 port//7 studyimage dir	//8 level
	//var PAGE_Dir, Web_Port, IMAGE_Dir, MySQL_IP, MySQL_User, MySQL_PWD, MySQL_Name string
	if len(os.Args) > 8 {
		// for idx, args := range os.Args {		// 	//println("参数"+strconv.Itoa(idx)+":", args)		// }
		// MySQL_IP, MySQL_User, MySQL_PWD, MySQL_Name
		for i := 1; i < 9; i++ {
			CONFIG[i] = os.Args[i]
			log4go.Info(CONFIG[i])
		}
		//DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
		DB_Driver = CONFIG[MySQL_User] + ":" + CONFIG[MySQL_PWD] + "@tcp(" + CONFIG[MySQL_IP] + ":3306)/" + CONFIG[MySQL_DBName] + "?charset=utf8"
	}
	exepath, err := GetCurrentPath()
	if err != nil {
		// log.Fatal(err)
		log4go.Error(err)
		os.Exit(1)
	} else {
		CONFIG[PAGE_Dir] = exepath + "/PageWeb"
		exist, err := PathExists(CONFIG[PAGE_Dir])
		if err != nil {
			println("get dir error![%v]\n", err)
			CONFIG[PAGE_Dir] = "D:/code/C++/HealthApp/Tools/PageWeb"
			//log.Fatal(err)
		}
		if exist {
			println("PAGE_Dir:" + CONFIG[PAGE_Dir])
		} else {
			CONFIG[PAGE_Dir] = "D:/code/C++/HealthApp/Tools/PageWeb"
			println("use coe page:" + CONFIG[PAGE_Dir])
			//log.Fatal(err)
		}
		println(exepath)
	}
	// println(hash)
	maridb_db = nil
	open, db := OpenDB()
	if open == true {
		maridb_db = db
		for i := 1; i < 9; i++ {
			CONFIG[i] = os.Args[i]
			log4go.Info("CONFIG:" + CONFIG[i])
		}
	}

	// flag.Parse() //暂停获取参数
	WebServer := echo.New()

	//login
	WebServer.POST("/login/checkuser", CheckLogin)
	// WebServer.GET("/Login/*", Login)
	WebServer.GET("/login/*", Login)

	//ris
	WebServer.GET("/healthsystem/ris/studydata/", GetDBStudyData)
	WebServer.GET("/healthsystem/ris/stduyimage/", GetDBStudyImage)

	//ris//update ->studydata
	WebServer.POST("/healthsystem/ris/updata/", UpdateDBStudyData)
	WebServer.POST("/healthsystem/ris/update/", UpdateDBStudyData)

	//ris/report
	WebServer.POST("/healthsystem/ris/getreportdata", GetReportdata)
	WebServer.POST("/healthsystem/ris/savereportdata/", SaveReportdata)

	WebServer.GET("/healthsystem/*", Healthsystem)

	// view dicom
	WebServer.GET("/view/*", LoadViewPage)

	// other
	WebServer.GET("/favicon.ico", func(c echo.Context) error {
		// println("----------favicon.ico--------")
		return c.File(CONFIG[PAGE_Dir] + "/favicon.ico")
	})

	// load image file
	//需要定义/WADO?*路由??
	WebServer.GET("/*", LoadImageFile) //WADO?*
	// WebServer.GET("/", OutRouter) //WADO?*
	// log4go.LOGGER("Test").Info("Start Web:" + CONFIG[Web_Port])
	println("Start Web:" + CONFIG[Web_Port])
	WebServer.Logger.Fatal(WebServer.Start(":" + CONFIG[Web_Port])) // e.Logger.Fatal(e.Start(":9090"))
}

func OutRouter(c echo.Context) error {
	println(c.Request().URL.Path)
	if c.Request().URL.Path == "/WADO" {
		return LoadImageFile(c)
	}
	log4go.LOGGER("Test").Info("OutRouter URL:" + c.Request().URL.Path)
	return c.String(http.StatusOK, "No Page! maybe remove!"+c.Request().URL.Path)
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

func IsPathExists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}
	if os.IsNotExist(err) {
		return false
	}
	return false
}

func IsFileExists(filename string) bool {
	_, err := os.Stat(filename)
	if err == nil {
		return true
	}
	if os.IsNotExist(err) {
		return false
	}
	return false
}

func GetCurrentPathStr() string {
	path, err := GetCurrentPath()
	if err != nil {
		return ""
	} else {
		return path
	}
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
	// log4go.Info("---AuthLogin---- Healthsystem:" + c.Request().URL.Path)
	cookie_Auth_OK := false
	for _, cookie := range c.Cookies() {
		log4go.Info("--Healthsystem read Cookies-" + cookie.Name)
		log4go.Info("--Healthsystem read Cookies-" + cookie.Value)
		if cookie.Name == "admin" {
			cookie_Auth_OK = true
			break
		}
	}
	if cookie_Auth_OK {
		log4go.Info("Healthsystem: AuthLogin true")
		req := c.Request()
		filepath := CONFIG[PAGE_Dir] + req.URL.Path
		return c.File(filepath)
	} else {
		log4go.Info("No cookie ---->Redirect:" + c.Scheme() + "://" + c.Request().Host + "/login/login.html")
		//StatusMovedPermanently 301
		return c.Redirect(http.StatusMovedPermanently, c.Scheme()+"://"+c.Request().Host+"/login/login.html")
	}
}

func Login(c echo.Context) error {
	// log4go.Info("-------Login:" + c.Request().URL.Path)
	// log4go.Info(c.Request().URL.Path)
	req := c.Request()
	// println("req.URL.Path:" + req.URL.Path)
	// println(PAGE_Dir + req.URL.Path)
	filepath := CONFIG[PAGE_Dir] + req.URL.Path
	// log4go.Info("filepath:" + filepath)
	return c.File(filepath)
}

func LoadImageFile(c echo.Context) error {
	// log4go.Info(c.Request().URL.Path)
	studyuid := c.FormValue("studyuid")
	image_hash_dir := Units.GetStudyHashDir(studyuid)
	filepath := CONFIG[IMAGE_Dir] + image_hash_dir
	filepath += "/"
	filepath += studyuid
	filetype := c.FormValue("type")
	// println(filetype)
	if filetype == "json" {
		filepath += "/"
		filepath += studyuid
		filepath += ".json"
		if IsFileExists(filepath) {
			return c.File(filepath)
		}
	} else {
		seriesuid := c.FormValue("seriesuid")
		sopinstanceuid := c.FormValue("sopinstanceuid")
		filepath += "/"
		filepath += seriesuid
		filepath += "/"
		filepath += sopinstanceuid
		filepath += ".dcm"
		if IsFileExists(filepath) {
			return c.File(filepath)
		}
	}
	log4go.Info("No filepath:" + filepath)
	return c.String(http.StatusOK, "No file! maybe remove!")
}

func LoadViewPage(c echo.Context) error {
	//log4go.Info(c.Request().URL.Path)
	req := c.Request()
	filepath := CONFIG[PAGE_Dir] + req.URL.Path
	index := strings.Index(req.URL.Path, ".html?")
	if index > -1 {
		filename := req.URL.Path[0 : index+5]
		filepath = CONFIG[PAGE_Dir] + filename
		return c.File(filepath)
	} else {
		return c.File(CONFIG[PAGE_Dir] + req.URL.Path)
	}
}

func AuthLogin(c echo.Context) bool {
	log4go.Info("---to read --c.Cookies-------AuthLogin-------------------")
	for _, cookie := range c.Cookies() {
		log4go.Info("--Cookies-" + cookie.Name)
		log4go.Info("--Cookies-" + cookie.Value)
	}
	cookie, err := c.Cookie("username")
	if err != nil {
		// return err
		// log4go.Error(err)
		log4go.Info("-----Cookie:err---------")
		return false
	} else {
		log4go.Info("---cookie.Name-----" + cookie.Name)
		log4go.Info("---cookie.Value----" + cookie.Value)
		if cookie.Name == "admin" {
			log4go.Info("cookie.Name" + cookie.Name)
			log4go.Info("cookie.Value" + cookie.Value)
			return true
		}
	}
	log4go.Info("-- AuthLogin --no user in cookie")
	return false
}

func CheckLogin(c echo.Context) error {
	//log4go.Info(c.Request().URL.Path)
	username := c.FormValue("user")
	userpwd := c.FormValue("password")
	if maridb_db != nil {
		var sqlstr, id, user, pwd string
		sqlstr = "select * from h_user where username='" + username + "'"
		rows, err := maridb_db.Query(sqlstr)
		if err != nil {
			log4go.Error(err)
		} else {
			for rows.Next() {
				err = rows.Scan(&id, &user, &pwd)
			}
			// log4go.Info("CheckLogin:" + id + "/" + pwd)
			if err != nil {
				log4go.Error(err)
			} else {
				if id != "" && pwd == userpwd {
					//set cookie
					cookie := new(http.Cookie)
					cookie.Path = "/"
					cookie.Name = "admin"
					cookie.Value = userpwd
					cookie.HttpOnly = true
					cookie.Expires = time.Now().Add(24 * time.Hour)
					c.SetCookie(cookie)
					log4go.Info("SetCookie / username:" + username + "/userpwd:" + userpwd)
					// return c.Redirect(http.StatusMovedPermanently, c.Scheme()+"://"+c.Request().Host+"/healthsystem/ris/studys.html")
					// for _, cookie := range c.Cookies() {
					// 	log4go.Info("--read Cookies- Path" + cookie.Path)
					// 	log4go.Info("--read Cookies- Name" + cookie.Name)
					// 	log4go.Info("--read Cookies- Value" + cookie.Value)
					// }
					return c.String(http.StatusOK, "OK")
				}
			}
		}
	}
	log4go.Info("username:" + username + "/userpwd:" + userpwd)
	return c.String(http.StatusOK, "username or userpwd error! fail")
}

func SaveReportdata(c echo.Context) error {
	//log4go.Info(c.Request().URL.Path)
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
				log4go.Error(perr)
				os.Exit(1)
			}
			affect_count, err := stmt.Exec(reportdata.ReportState, reportdata.ReportTemplate, reportdata.ReportCheckID, reportdata.ReportCheckDate, reportdata.ReportContent, reportdata.ReportOther, reportdata.ReportIdentity)
			if err != nil {
				println("affect_count")
				println(affect_count)
				log4go.Error(err)
				os.Exit(1)
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
				log4go.Error(perr)
				os.Exit(1)
			} /* else {
				println(sqlstr)
			}*/
			reportdata.ReportIdentity = reportdata.StudyOrderIdentity
			affect_count, err := stmt.Exec(reportdata.StudyOrderIdentity, reportdata.ReportIdentity, reportdata.ReportState,
				reportdata.ReportTemplate, reportdata.ReportCreatDate, reportdata.ReportWriterID, reportdata.ReportCheckID, reportdata.ReportCheckDate, reportdata.ReportContent, reportdata.ReportOther)
			if err != nil {
				println(err)
				log4go.Error(err)
				os.Exit(1)
			} /*else {
				println("affect_count:")
				println(affect_count)
			}*/
			lastInsertId, err := affect_count.RowsAffected()
			if err != nil {
				println("lastInsertId:")
				println(lastInsertId)
				println(err)
				log4go.Error(err)
				os.Exit(1)
			} /*else {
				println("lastInsertId:")
				println(lastInsertId)
			}*/
		}
	}
	return c.String(http.StatusOK, "ok")
}

func GetReportdata(c echo.Context) error {
	//log4go.Info(c.Request().URL.Path)
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
				log4go.Error(err)
				os.Exit(1)
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
				log4go.Info(affect_count)
				log4go.Error(err)
				os.Exit(1)
			} else {
				lastInsertId, err := affect_count.RowsAffected()
				if err != nil {
					println("lastInsertId:")
					println(lastInsertId)
					log4go.Error(err)
					os.Exit(1)
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
				log4go.Error(perr)
				os.Exit(1)
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
				log4go.Info(affect_count)
				log4go.Error(err)
				os.Exit(1)
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
				log4go.Error(err)
				os.Exit(1)
			}
			affect_count, err := stmt.Exec(StudyOrderIdentity, studyData.StudyID, studyData.StudyUID, studyData.PatientIdentity,
				studyData.ScheduledDate, studyData.StudyDescription, studyData.StudyModality,
				studyData.StudyCost, studyData.StudyCode, studyData.StudyDepart, studyData.CostType)
			if err != nil {
				println(affect_count)
				log4go.Error(err)
				os.Exit(1)
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
				log4go.Error(perr)
				os.Exit(1)
			}
			affect_count, err := stmt.Exec(studyData.ScheduledDate, studyData.StudyDescription, studyData.StudyModality,
				studyData.StudyCost, studyData.StudyCode, studyData.StudyDepart, studyData.CostType, studyData.StudyOrderIdentity)
			if err != nil {
				println("studyData.ScheduledDate" + studyData.ScheduledDate)
				println(affect_count)
				log4go.Error(err)
				os.Exit(1)
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
		log4go.Error("Exit 0. Open db fail!")
		log4go.Error(err)
		os.Exit(1)
	} else {
		isOpen = true
	}
	checkErr(err)
	return isOpen, db
}

//----------------------------------------------------------------------------------
// for idx, args := range os.Args {
// 	// println("参数"+strconv.Itoa(idx)+":", args)
// 	log4go.Info("参数" + strconv.Itoa(idx) + ":" + args)
// }
//参考
// func queryMySal() {
// 	var err error
// 	connect := "f:f@tcp(192.168.1.0:3306)/lr1"
// 	db, err := sql.Open("mysql", connect)
// 	if err != nil {
// 		println("connect mysql failed, address = "+connect, err)
// 	} else {
// 		sqlContent := "select real_nm, sex, birth, ext from member where tenant=? and mem_id=?"
// 		rows, err := db.Query(sqlContent, "ST", "2017")
// 		if err != nil {
// 			println(err)
// 		} else {
// 			for rows.Next() {
// 				nm := sql.NullString{String: "", Valid: false}
// 				sex := sql.NullString{String: "", Valid: false}
// 				birth := sql.NullString{String: "", Valid: false}
// 				ext := sql.NullString{String: "", Valid: false}
// 				err := rows.Scan(&nm, &sex, &birth, &ext) // birth字段在数据库中是空字段
// 				if err != nil {
// 					println("d12 error. ", err)
// 				}
// 			}
// 		}
// 	}
// }
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
// CONFIG[MySQL_IP] = os.Args[1]
// CONFIG[MySQL_DBName] = os.Args[2]
// CONFIG[MySQL_User] = os.Args[3]
// CONFIG[MySQL_PWD] = os.Args[4]
// CONFIG[PAGE_Dir] = os.Args[5]
// CONFIG[Web_Port] = os.Args[6]
// CONFIG[IMAGE_Dir] = os.Args[7]
// var hash string = Units.GetStudyHashDir("1.2.840.113619.2.55.3.604688119.868.1249343483.504")
// rand.Seed(int64(time.Now().UnixNano()))
// id := rand.Int()
// println(strconv.Itoa(id))
// println(strconv.Itoa(rand.Int()))
// println(Units.GetCurrentTime())
// type CustomContext struct {
// 	echo.Context
// }
// func init() {
// 	flag.StringVar(&name, "name", "default", "log in user")
// }
