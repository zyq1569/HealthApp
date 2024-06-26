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
error!-----> 需要当前目录命令行 go mod tidy
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
	"GoWeb/Data"
	"GoWeb/Units"
	"database/sql"
	"encoding/json"
	"errors"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"

	_ "github.com/go-sql-driver/mysql"
	log4go "github.com/jeanphorn/log4go"
	"github.com/labstack/echo"
	_ "github.com/mattn/go-sqlite3"
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
	AppName_path = 0
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
var sqlite3_Driver string
var Go_Level int
var sqlite_db bool

// var name string
var maridb_db *sql.DB

func main() {
	DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
	sqlite3_Driver = "hitSqlite.db"
	CONFIG[IMAGE_Dir] = "F:/temp/HealthApp/DCM_SAVE/DCM_SAVE/Images"
	CONFIG[PAGE_Dir] = "F:/temp/HealthApp/PageWeb/PageWeb"
	CONFIG[Web_Port] = "9090"
	//log4go.LoadConfiguration("./config/goWebConfig.json") // to do set ?
	//1 mysql: 1 ip 2 name 3 user  4pwd	//5 page web / 6 port//7 studyimage dir	//8 level
	//var PAGE_Dir, Web_Port, IMAGE_Dir, MySQL_IP, MySQL_User, MySQL_PWD, MySQL_Name string
	Go_Level = -1
	if len(os.Args) > 8 {
		// for idx, args := range os.Args {		// 	//println("参数"+strconv.Itoa(idx)+":", args)		// }
		// MySQL_IP, MySQL_User, MySQL_PWD, MySQL_Name
		for i := 1; i < CONGIG_SIZE; i++ {
			CONFIG[i] = os.Args[i]
			log4go.Info(os.Args[i])
		}
		Level, err := strconv.Atoi(CONFIG[LOG_Level])
		if err != nil {
			Go_Level = 1
			log4go.Error("set default Go_Level::1")
			// log4go.Info(Go_Level)
		} else {
			Go_Level = Level
			log4go.Info("Get Go_Level::" + CONFIG[LOG_Level])
		}
		//DB_Driver = "root:root@tcp(127.0.0.1:3306)/hit?charset=utf8"
		DB_Driver = CONFIG[MySQL_User] + ":" + CONFIG[MySQL_PWD] + "@tcp(" + CONFIG[MySQL_IP] + ":3306)/" + CONFIG[MySQL_DBName] + "?charset=utf8"
	}
	///-set log4go--
	if Go_Level < 2 {
		//输出到控制台,级别为DEBUG
		log4go.AddFilter("stdout", log4go.DEBUG, log4go.NewConsoleLogWriter())
		//输出到文件,级别为DEBUG,文件名为test.log,每次追加该原文件
		log4go.AddFilter("file", log4go.DEBUG, log4go.NewFileLogWriter("./win32/log/GoWebServer.log", false, true))
		log4go.Info("----------Level:DEBUG---------------")
	} else if Go_Level == 2 {
		log4go.AddFilter("stdout", log4go.INFO, log4go.NewConsoleLogWriter())
		log4go.AddFilter("file", log4go.INFO, log4go.NewFileLogWriter("./win32/log/GoWebServer.log", false, true))
		log4go.Info("----------Level:INFO---------------")
	} else if Go_Level == 3 {
		log4go.AddFilter("stdout", log4go.WARNING, log4go.NewConsoleLogWriter())
		log4go.AddFilter("file", log4go.WARNING, log4go.NewFileLogWriter("./win32/log/GoWebServer.log", false, true))
		log4go.Info("----------Level:WARNING---------------")
	} else if Go_Level == 4 {
		log4go.AddFilter("stdout", log4go.ERROR, log4go.NewConsoleLogWriter())
		log4go.AddFilter("file", log4go.ERROR, log4go.NewFileLogWriter("./win32/log/GoWebServer.log", false, true))
		log4go.Info("----------Level:ERROR---------------")
	} else if Go_Level > 4 {
		log4go.AddFilter("stdout", log4go.CRITICAL, log4go.NewConsoleLogWriter())
		log4go.AddFilter("file", log4go.CRITICAL, log4go.NewFileLogWriter("./win32/log/GoWebServer.log", false, true))
		log4go.Info("----------Level:CRITICAL---------------")
	}
	///------------------------------------------------------------
	exepath, err := GetCurrentPath()
	if err != nil {
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
	if CONFIG[MySQL_IP] == "0.0.0.0" {
		dir, err := filepath.Abs(filepath.Dir(os.Args[0]))
		if err != nil {
			log4go.Error(err)
		}
		sqlite3_Driver = dir + "\\hitSqlite.db"
		log4go.Info(sqlite3_Driver)

		open, db := OpenSqlite()
		if open == true {
			sqlite_db = true
			maridb_db = db
			log4go.Info("local sqlite3 DB")
			for i := 1; i < 9; i++ {
				CONFIG[i] = os.Args[i]
				log4go.Info("CONFIG:" + CONFIG[i])
			}
		}

	} else {
		sqlite_db = false
		open, db := OpenDB()
		if open == true {
			maridb_db = db
			log4go.Info("maridb DB")
			for i := 1; i < 9; i++ {
				CONFIG[i] = os.Args[i]
				log4go.Info("CONFIG:" + CONFIG[i])
			}
		}
	}

	// flag.Parse() //暂停获取参数
	WebServer := echo.New()

	//login
	WebServer.POST("/login/checkuser", CheckLogin)
	WebServer.GET("/login/checkuser", CheckLogin)
	// WebServer.GET("/Login/*", Login)
	WebServer.GET("/login/*", Login)

	//ris study order info
	WebServer.GET("/healthsystem/ris/studydata/", GetDBStudyData)
	//2020-1117 update--->ris study order info [save old fun up]
	WebServer.GET("/healthsystem/ris/StudyOrder/", GetStudyOrderFromDB)
	//ris study image info
	WebServer.GET("/healthsystem/ris/stduyimage/", GetDBStudyImage)
	//

	//ris//update ->studydata
	WebServer.POST("/healthsystem/ris/updata/", UpdateDBStudyData)
	WebServer.POST("/healthsystem/ris/SaveStudyOrde/", UpdateStudyOrderToDB)

	//ris/report
	WebServer.POST("/healthsystem/ris/getreportdata", GetReportdata)
	WebServer.POST("/healthsystem/ris/savereportdata/", SaveReportdata)
	WebServer.POST("/healthsystem/ris/saveodtreport/", SaveOdtReport)
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
	cookie_Auth_OK := false
	for _, cookie := range c.Cookies() {
		if cookie.Name == "user" || cookie.Name == "admin" {
			cookie_Auth_OK = true
			break
		} else {
			log4go.Error("Healthsystem cookie.Name:" + cookie.Name)
		}

	}
	if cookie_Auth_OK {
		req := c.Request()
		filepath := CONFIG[PAGE_Dir] + req.URL.Path
		return c.File(filepath)
	} else {
		log4go.Info("No cookie|" + c.Request().URL.Path + "---->Redirect:" + c.Scheme() + "://" + c.Request().Host + "/login/login.html")
		//StatusMovedPermanently 301
		return c.Redirect(http.StatusMovedPermanently, c.Scheme()+"://"+c.Request().Host+"/login/login.html")
	}
}

// /https://github.com/authelia/authelia
func Login(c echo.Context) error {
	log4go.Info("-------Login:" + c.Request().URL.Path)
	log4go.Info(c.Request().URL.Path)
	req := c.Request()
	// println("req.URL.Path:" + req.URL.Path)
	// println(PAGE_Dir + req.URL.Path)
	filepath := CONFIG[PAGE_Dir] + req.URL.Path
	log4go.Info("filepath:" + filepath)
	return c.File(filepath)
}

// / 获取odt文档时:如果对应的检查uid的odt文件没有找到，使用patient.odt，如果patient.odt也没有找到
// /在查找HealthApp\Server\PageWeb\Login\test 下面studyTemp.odt
// / get files: image | odt | json|
func LoadImageFile(c echo.Context) error {
	/// -------------------------------------------------------------------------
	///http://127.0.0.1:8080/WADO?
	///studyuid=1.2.826.1.1.3680043.2.461.20090916105245.168977.200909160196
	///&seriesuid=1.2.840.113619.2.55.3.604688119.969.1252951290.810.4
	///&sopinstanceuid=1.2.840.113619.2.55.3.604688119.969.1252951290.968.37
	/// -------------------------------------------------------------------------
	///http://127.0.0.1:8080/WADO?
	///studyuid=1.2.826.1.1.3680043.2.461.20090916105245.168977.200909160196&type=json
	///-------------------------------------------------------------------------
	log4go.Debug("----LoadImageFile-------" + c.Request().URL.Path)
	filetype := c.FormValue("type")
	studyuid := c.FormValue("studyuid")
	log4go.Debug("filetype:" + filetype)
	// if filetype == "" {
	// 	filetype == dcm
	// }
	if filetype == "odt" {
		odtpath := CONFIG[IMAGE_Dir] + "/Report/"
		odtpath += studyuid
		odtpath += ".odt"
		patientReport := CONFIG[IMAGE_Dir] + "/Report/patient.odt"
		//log4go.Debug("type == odt filepath:" + odtpath) //patient/.odt
		studyTemp := CONFIG[PAGE_Dir] + "/Login/report/patient.odt"
		if IsFileExists(odtpath) {
			return c.File(odtpath)
		} else if IsFileExists(patientReport) {
			log4go.Warn("use deault Report filepath:" + patientReport)
			//log4go.Error("No filepath:" + odtpath)
			return c.File(patientReport)
		}
		if IsFileExists(studyTemp) {
			log4go.Warn("use temp deault Report filepath:" + studyTemp)
			return c.File(studyTemp)
		} else {
			log4go.Error("No filepath:" + odtpath)
		}
		return c.Redirect(http.StatusMovedPermanently, c.Scheme()+"://"+c.Request().Host+"/login/login.html")
	}

	studyDate := c.FormValue("studyDate")
	//log4go.Info("studyDate:" + studyDate)
	image_hash_dir := Units.GetStudyHashDir(studyuid, studyDate)
	//log4go.Debug(image_hash_dir)
	filepath := CONFIG[IMAGE_Dir] + image_hash_dir
	filepath += "/"

	// println(filetype)
	if filetype == "json" {
		filepath += studyuid
		filepath += ".json"
		log4go.Debug("type == json filepath:" + filepath)
		if IsFileExists(filepath) {
			return c.File(filepath)
		} else {
			log4go.Error("studyuid:" + studyuid)
			log4go.Error("image_hash_dir:" + image_hash_dir + "No filepath:" + filepath)
		}
	} else {
		seriesuid := c.FormValue("seriesuid")
		sopinstanceuid := c.FormValue("sopinstanceuid")
		filepath += seriesuid
		filepath += "/"
		filepath += sopinstanceuid
		filepath += ".dcm"
		//log4go.Debug("dcm filepath:" + filepath)
		if IsFileExists(filepath) {
			return c.File(filepath)
		} else {
			log4go.Error("No filepath:" + filepath)
		}
	}
	// log4go.Info("No filepath:" + filepath)
	// return c.String(http.StatusOK, "No file! maybe remove!")
	// auto to login.html
	return c.Redirect(http.StatusMovedPermanently, c.Scheme()+"://"+c.Request().Host+"/login/login.html")
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

func getCookieHandler(w http.ResponseWriter, r *http.Request) {
	h := r.Header["Cookie"]
	log4go.Info("---getCookieHandler---")
	log4go.Info(w, h)
}

// func AuthLogin(c echo.Context) bool {
// 	for _, cookie := range c.Cookies() {
// 		if cookie.Name == "user" || cookie.Name == "admin" {
// 			return true
// 		} else {
// 			log4go.Error("AuthLogin:" + cookie.Name)
// 		}
// 	}
// 	return false
// }

func CheckLogin(c echo.Context) error {
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
			if err != nil {
				log4go.Error(err)
			} else {
				if id != "" && pwd == userpwd {
					//set cookie
					cookie := new(http.Cookie)
					cookie.Path = "/"
					cookie.Name = "user"
					cookie.Value = userpwd
					cookie.Secure = false
					cookie.HttpOnly = true
					// cookie.Expires = time.Now().Add(2 * time.Hour)
					c.SetCookie(cookie)
					return c.String(http.StatusOK, "OK")
				}
			}
		}
	} else {
		log4go.Info("OpenDB error! username:" + username + "/userpwd:" + userpwd)
		return c.String(http.StatusOK, "error! username or userpwd error? or table[h_user] no data, pls add data! } fail! ")
	}
	log4go.Info("username:" + username + "/userpwd:" + userpwd)
	return c.String(http.StatusOK, "username or userpwd error! fail")
}

// /"http://" + serverHost + "/healthsystem/ris/saveodtreport/?StudyOrderIdentity=" + orderid;
func SaveOdtReport(c echo.Context) error {
	var reportdata Study.ReportData
	reportdata.ReportIdentity = ""
	var bodyBytes []byte
	if c.Request().Body != nil {
		bodyBytes, _ = ioutil.ReadAll(c.Request().Body)
		reportdata.ReportIdentity = c.FormValue("StudyOrderIdentity")
		if reportdata.ReportIdentity == "" {
			log4go.Error("reportdata.ReportIdentity == 0 ")
			return c.String(http.StatusBadRequest, "error")
		}
		/// to do : MkdirAll ???? 增加了体积
		err := os.MkdirAll(CONFIG[IMAGE_Dir]+"/Report", os.ModePerm) //在当前目录下生成md目录
		if err != nil {
			log4go.Error(err)
		} else {
			log4go.Warn("os.Mkdir" + CONFIG[IMAGE_Dir] + "/Report/")
		}
		///
		reportIdentity := reportdata.ReportIdentity
		fileName := CONFIG[IMAGE_Dir] + "/Report/" + reportIdentity + ".odt"
		f, err := os.OpenFile(fileName, os.O_RDWR|os.O_CREATE /*|os.O_TRUNC*/, 0666)
		defer f.Close()
		if err != nil {
			log4go.Error(err.Error())
			return c.String(http.StatusBadRequest, "error")
		} else {
			_, err = f.Write(bodyBytes)
			checkErr(err)
		}
	}
	return c.String(http.StatusOK, "ok")
}

// / for tinymce report ：： 目前没有用，web 页面已经屏蔽
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
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			affect_count, err := stmt.Exec(reportdata.ReportState, reportdata.ReportTemplate, reportdata.ReportCheckID, reportdata.ReportCheckDate, reportdata.ReportContent, reportdata.ReportOther, reportdata.ReportIdentity)
			if err != nil {
				println("affect_count")
				println(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
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
				"`ReportWriterID`,`ReportCheckID`, `ReportCheckDate`,`ReportContent`,`ReportOther`) values(?,?,?,?,?,?,?,?,?,?)"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println(sqlstr)
				log4go.Error(sqlstr)
				log4go.Error(perr)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			} /* else {
				println(sqlstr)
			}*/
			reportdata.ReportIdentity = reportdata.StudyOrderIdentity
			affect_count, err := stmt.Exec(reportdata.StudyOrderIdentity, reportdata.ReportIdentity, reportdata.ReportState,
				reportdata.ReportTemplate, reportdata.ReportCreatDate, reportdata.ReportWriterID, reportdata.ReportCheckID, reportdata.ReportCheckDate, reportdata.ReportContent, reportdata.ReportOther)
			if err != nil {
				println(err)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
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
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
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
	log4go.Info(c.Request().URL)
	///'http://127.0.0.1:8080/healthsystem/ris/stduyimage/?' + searchImageTime
	///http://127.0.0.1:8080/healthsystem/ris/stduydata/?start=20190101&end=20191219&page=1&limit=1
	//分页查询https://blog.csdn.net/myth_g/article/details/89672722
	startTime := c.FormValue("start")
	endTime := c.FormValue("end")
	var sqlstr string
	sqlstr = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday," +
		" p.PatientSex,s.StudyUID,s.StudyID,s.StudyOrderIdentity,s.StudyDateTime," +
		" s.StudyDescription, s.StudyModality, s.StudyState from " +
		" h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and " +
		" s.StudyDateTime >= '" + startTime + "' and  s.StudyDateTime <= '" + endTime +
		"' order by s.PatientIdentity limit "
	if sqlite_db {
		startTime = startTime[0:4] + "-" + startTime[4:6] + "-" + startTime[6:8]
		endTime = endTime[0:4] + "-" + endTime[4:6] + "-" + endTime[6:8]
		sqlstr = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday," +
			" p.PatientSex,s.StudyUID,s.StudyID,s.StudyOrderIdentity,strftime('%Y%m%d%H%M',s.StudyDateTime) as StudyDateTime," +
			" s.StudyDescription, s.StudyModality, s.StudyState from " +
			" h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and " +
			" s.StudyDateTime >= '" + startTime + "' and  s.StudyDateTime <= '" + endTime +
			"' order by s.PatientIdentity limit "
	}
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
		sqlstr += strconv.Itoa(count) + "," + limit
		// println(sqlstr)
		rows, err := maridb_db.Query(sqlstr)
		log4go.Debug(sqlstr)
		if err != nil {
			println(err)
		} else {
			studyjson.Code = 0
			studyjson.Msg = ""
			studyjson.Count = 0
			for rows.Next() {
				var data Study.StudyData
				err = rows.Scan(&data.PatientIdentity, &data.PatientName,
					&data.PatientID, &data.PatientBirthday,
					&data.PatientSex, &data.StudyUID, &data.StudyID,
					&data.StudyOrderIdentity, &data.StudyDateTime,
					&data.StudyDescription, &data.StudyModality, &data.StudyState)
				studyjson.Data = append(studyjson.Data, data)
				studyjson.Count++
			}
			//log4go.Debug(studyjson)
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

// / web->>html Update---->DBStudyData
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
	if maridb_db != nil {
		var sqlstr string
		if PatientID == "" { //create a study
			PatientID = Units.GetRandUID()
			PatientIdentity = Units.GetRandUID()
			studyData.PatientIdentity = PatientIdentity
			sqlstr = "insert into h_patient(`PatientIdentity`,`PatientID`,`PatientName`,`PatientSex`,`PatientBirthday`," +
				"`PatientAddr`,`PatientEmail`,`PatientCarID`,`PatientTelNumber`,`PatientType`) values(?,?,?,?,?,?,?,?,?,?)"
			stmt, err := maridb_db.Prepare(sqlstr)
			if err != nil {
				println("------fail insert into h_patient maridb_db.Prepare--------")
				println("PatientID:" + PatientID)
				println("PatientIdentity:" + PatientIdentity)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			} else {
				println("PatientID:" + PatientID)
				println("PatientIdentity:" + PatientIdentity)
			}
			if studyData.PatientType == "" {
				studyData.PatientType = "0"
			}
			affect_count, err := stmt.Exec(PatientIdentity, PatientID, studyData.PatientName, studyData.PatientSex,
				studyData.PatientBirthday, studyData.PatientAddr, studyData.PatientEmail,
				studyData.PatientCarID, studyData.PatientTelNumber, studyData.PatientType)
			if err != nil {
				println("fail to  insert into h_patient affect_count:")
				log4go.Info(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			} else {
				lastInsertId, err := affect_count.RowsAffected()
				if err != nil {
					println("lastInsertId:")
					println(lastInsertId)
					log4go.Error(err)
					//os.Exit(1)
					return c.String(http.StatusBadRequest, "error")
				}
			}
		} else {
			sqlstr = "update  h_patient set `PatientName`=?,`PatientSex`=?,`PatientBirthday`=?,`PatientAddr`=?," +
				"`PatientEmail`=?, `PatientCarID`=?,`patientTelNumber`=?,`PatientType`=?  where `PatientID`=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println("------fail update PatientID:--------")
				println(PatientID)
				log4go.Error(perr)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			if studyData.PatientType == "" {
				studyData.PatientType = "0"
			}
			affect_count, err := stmt.Exec(studyData.PatientName, studyData.PatientSex, studyData.PatientBirthday,
				studyData.PatientAddr, studyData.PatientEmail, studyData.PatientCarID,
				studyData.PatientTelNumber, studyData.PatientType, studyData.PatientID)
			if err != nil {
				println("fail to  update PatientI affect_count:")
				log4go.Info(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
		}
		//update order table
		StudyOrderIdentity := studyData.StudyOrderIdentity
		if StudyOrderIdentity == "" {
			StudyOrderIdentity = Units.GetRandUID()
			if studyData.StudyID == "" {
				studyData.StudyID = Units.GetCurrentTime()
				studyData.StudyUID = Units.GetNewStudyUID()
			}
			sqlstr = "insert into h_order (`StudyOrderIdentity`,`PatientIdentity`,`StudyID`, `StudyUID`,`StudyModality`," +
				"`ScheduledDateTime`,`StudyDateTime`,`StudyDescription`,`StudyDepart`,`StudyCode`,`StudyCost`,`CostType`)" +
				" values(?,?,?,?,?,?,?,?,?,?,?,?)"
			stmt, err := maridb_db.Prepare(sqlstr)
			if err != nil {
				println("------fail  maridb_db.Prepare(sqlstr) insert into h_order:--------")
				println(StudyOrderIdentity)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			if sqlite_db {
				studyData.ScheduledDate = studyData.ScheduledDate[0:4] + "-" + studyData.ScheduledDate[4:6] + "-" + studyData.ScheduledDate[6:8]
			}
			affect_count, err := stmt.Exec(StudyOrderIdentity, studyData.PatientIdentity, studyData.StudyID,
				studyData.StudyUID, studyData.StudyModality, studyData.ScheduledDate, studyData.ScheduledDate, studyData.StudyDescription,
				studyData.StudyDepart, studyData.StudyCode, studyData.StudyCost, studyData.CostType)
			if err != nil {
				println(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
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
			sqlstr = "update h_order set `StudyModality`=?, `ScheduledDateTime`=?, `StudyDescription`=?, `StudyDepart`=?," +
				"`StudyCode`=?, `StudyCost`=? ,  `CostType`=?  where StudyOrderIdentity=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				println(sqlstr)
				println("StudyOrderIdentity:")
				println(StudyOrderIdentity)
				log4go.Error(perr)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			affect_count, err := stmt.Exec(studyData.StudyModality, studyData.ScheduledDate, studyData.StudyDescription,
				studyData.StudyDepart, studyData.StudyCode, studyData.StudyCost, studyData.CostType, studyData.StudyOrderIdentity)
			if err != nil {
				println("studyData.ScheduledDate" + studyData.ScheduledDate)
				println(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
		}
	}
	return c.String(http.StatusOK, "OK")
}

func GetDBStudyData(c echo.Context) error {
	//'http://127.0.0.1:8080/healthsystem/ris/studydata/?' + searchStudyTime
	//分页查询https://blog.csdn.net/myth_g/article/details/89672722
	startTime := c.FormValue("start")
	endTime := c.FormValue("end")
	var sqlstr string
	sqlstr = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber," +
		" p.PatientAddr, p.PatientEmail, p.PatientCarID, s.StudyID ,s.StudyUID,s.StudyDepart,s.CostType," +
		" s.StudyOrderIdentity,s.ScheduledDateTime,s.StudyDateTime,s.StudyDescription, s.StudyModality, s.StudyCost, s.StudyState " +
		" from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and " +
		" s.StudyDateTime>= '" + startTime + "' and  s.StudyDateTime<= '" + endTime + "' order by s.StudyOrderIdentity " +
		" limit "
	if sqlite_db {
		startTime = startTime[0:4] + "-" + startTime[4:6] + "-" + startTime[6:8]
		endTime = endTime[0:4] + "-" + endTime[4:6] + "-" + endTime[6:8]
		sqlstr = "select p.PatientIdentity,p.PatientName,p.PatientID,p.PatientBirthday,p.PatientSex,p.PatientTelNumber," +
			" p.PatientAddr, p.PatientEmail, p.PatientCarID, s.StudyID ,s.StudyUID,s.StudyDepart,s.CostType," +
			" s.StudyOrderIdentity,s.ScheduledDateTime,strftime('%Y%m%d%H%M',s.StudyDateTime) as StudyDateTime,s.StudyDescription, s.StudyModality, s.StudyCost, s.StudyState " +
			" from h_patient p, h_order s where p.PatientIdentity = s.PatientIdentity and StudyState > 0 and " +
			" s.StudyDateTime>= '" + startTime + "' and  s.StudyDateTime<= '" + endTime + "' order by s.StudyOrderIdentity " +
			" limit "
	}
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
		sqlstr += strconv.Itoa(count) + "," + limit
		// println(sqlstr)
		if Go_Level < 2 {
			log4go.Debug(sqlstr)
		}
		rows, err := maridb_db.Query(sqlstr)
		if err != nil {
			println(err)
			log4go.Error(err)
		} else {
			studyjson.Code = 0
			studyjson.Msg = ""
			studyjson.Count = 0
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
				studyjson.Count++
			}
		}
	}
	js, err := json.Marshal(studyjson)
	if err != nil {
		println(err)
		log4go.Error(err)
		return c.String(http.StatusOK, "null")
	}
	// println(string(js))
	if Go_Level < 2 {
		log4go.Debug(string(js))
	}
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
func OpenSqlite() (success bool, db *sql.DB) {
	var isOpen bool
	db, err := sql.Open("sqlite3", sqlite3_Driver)
	if err != nil {
		log4go.Error(err)
	} else {
		isOpen = true
		log4go.Info("loacal sqlite3 db")
	}
	return isOpen, db
}
func OpenDB() (success bool, db *sql.DB) {
	var isOpen bool
	db, err := sql.Open("mysql", DB_Driver)
	if err != nil {
		isOpen = false
		log4go.Error("Exit 0. Open db fail!")
		log4go.Error(err)
		//os.Exit(1)
		//????
	} else {
		isOpen = true
	}
	checkErr(err)
	return isOpen, db
}

// /----------------2020-1117 add fun-------------------------------------------------------------------------------------
// /2020-1117:add{ 从新order表（增加字段）：查询检查信息（json data）}
func GetStudyOrderFromDB(c echo.Context) error {
	//log4go.Debug(c.Request().URL)
	//'http://127.0.0.1:8080/healthsystem/ris/StudyOrder/?' + searchStudyTime
	//分页查询https://blog.csdn.net/myth_g/article/details/89672722
	startTime := c.FormValue("start")
	endTime := c.FormValue("end")
	if sqlite_db {
		startTime = startTime[0:4] + "-" + startTime[4:6] + "-" + startTime[6:8]
		endTime = endTime[0:4] + "-" + endTime[4:6] + "-" + endTime[6:8]
	}
	page := c.FormValue("page")
	limit := c.FormValue("limit")
	var studyjson Study.StudyOrderDataJson
	if maridb_db != nil {
		var count int
		p, err := strconv.Atoi(page)
		checkErr(err)
		lim, err := strconv.Atoi(limit)
		checkErr(err)
		count = (p - 1) * lim
		var sqlstr string
		sqlstr = "select p.PatientIdentity , p.PatientID, p.PatientName, p.PatientNameEnglish," +
			"p.PatientSex , p.PatientBirthday , p.PatientAddr , p.PatientEmail , p.PatientCarID," +
			"p.PatientTelNumber , p.PatientType , p.PatientState ,p.PatientJob," +
			"p.PatientNation , p.PatientMarriage , p.PatientHometown ,p.PatientHisID," +
			"p.PatientHistoryTell ,o.StudyOrderIdentity," +
			"o.StudyID ,o.ClinicID , o.StudyUID,  o.StudyModality, o.StudyAge ," +
			"o.ScheduledDateTime , o.AETitle , o.OrderDateTime, o.StudyDescription," +
			"o.StudyDepart, o.StudyCode, o.StudyCost, o.CostType, o.StudyType," +
			"o.StudyState, o.StudyDateTime, o.InstitutionName, o.ProcedureStepStartDate," +
			"o.StudyModalityIdentity, o.StudyManufacturer , o.RegisterID " +
			"from h_patient p, h_order o  " +
			"where p.PatientIdentity = o.PatientIdentity and StudyState > 0 and " +
			"o.StudyDateTime>='" + startTime + "' and  o.StudyDateTime<='" + endTime + "' " +
			"order by o.StudyOrderIdentity " +
			"limit " + strconv.Itoa(count) + "," + limit

		log4go.Debug(sqlstr)

		rows, err := maridb_db.Query(sqlstr)
		if err != nil {
			println(err)
		} else {
			studyjson.Code = 0
			studyjson.Msg = ""
			studyjson.Count = 0
			for rows.Next() {
				var data Study.StudyOrderData
				err = rows.Scan(&data.PatientIdentity, &data.PatientID, &data.PatientName,
					&data.PatientNameEnglish, &data.PatientSex, &data.PatientBirthday,
					&data.PatientAddr, &data.PatientEmail, &data.PatientCarID,
					&data.PatientTelNumber, &data.PatientType, &data.PatientState,
					&data.PatientJob, &data.PatientNation, &data.PatientMarriage,
					&data.PatientHometown, &data.PatientHisID, &data.PatientHistoryTell,
					&data.StudyOrderIdentity, &data.StudyID, &data.ClinicID, &data.StudyUID, &data.StudyModality,
					&data.StudyAge, &data.ScheduledDateTime, &data.AETitle, &data.OrderDateTime,
					&data.StudyDescription, &data.StudyDepart, &data.StudyCode, &data.StudyCost, &data.CostType,
					&data.StudyType, &data.StudyState, &data.StudyDateTime, &data.InstitutionName,
					&data.ProcedureStepStartDate, &data.StudyModalityIdentity,
					&data.StudyManufacturer, &data.RegisterID)
				studyjson.Data = append(studyjson.Data, data)
				studyjson.Count++
			}
		}
	}
	js, err := json.Marshal(studyjson)
	if err != nil {
		println(err)
		return c.String(http.StatusOK, "null")
	}
	//log4go.Debug(string(js))
	log4go.Debug(string(js))
	return c.String(http.StatusOK, string(js))
}

func UpdateStudyOrderToDB(c echo.Context) error {
	var studyData Study.StudyOrderData
	var bodyBytes []byte
	if c.Request().Body != nil {
		bodyBytes, _ = ioutil.ReadAll(c.Request().Body)
		err := json.Unmarshal(bodyBytes, &studyData)
		if err != nil {
			println(err)
		}
	}
	log4go.Debug(string(bodyBytes))

	PatientIdentity := studyData.PatientIdentity
	PatientID := studyData.PatientID
	log4go.Debug("PatientID:" + PatientID)
	log4go.Debug("PatientIdentity:" + PatientIdentity)
	if maridb_db != nil {
		var sqlstr string
		if PatientIdentity == "" { //create a study
			PatientID = Units.GetRandUID()
			PatientIdentity = Units.GetRandUID()
			studyData.PatientIdentity = PatientIdentity
			sqlstr = "insert into h_patient(`PatientIdentity`,`PatientID`,`PatientName`,`PatientSex`,`PatientBirthday`," +
				"`PatientAddr`,`PatientEmail`,`PatientCarID`, `PatientTelNumber`,`PatientType`,`PatientJob`," +
				"`PatientNation`,`PatientMarriage`,`PatientHometown`,`PatientHisID`,`PatientHistoryTell`) " +
				" values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
			stmt, err := maridb_db.Prepare(sqlstr)
			if err != nil {
				log4go.Debug("sqlstr:" + sqlstr)
				log4go.Debug("------fail insert into h_patient maridb_db.Prepare--------")
				log4go.Debug(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			} else {
				log4go.Debug("sqlstr:" + sqlstr)
				log4go.Debug("PatientID:" + PatientID)
				log4go.Debug("PatientIdentity:" + PatientIdentity)
			}
			if studyData.PatientType == "" {
				studyData.PatientType = "0"
			}
			affect_count, err := stmt.Exec(PatientIdentity, PatientID, studyData.PatientName, studyData.PatientSex,
				studyData.PatientBirthday, studyData.PatientAddr, studyData.PatientEmail,
				studyData.PatientCarID, studyData.PatientTelNumber, studyData.PatientType,
				studyData.PatientJob, studyData.PatientNation, studyData.PatientMarriage,
				studyData.PatientHometown, studyData.PatientHisID, studyData.PatientHistoryTell)

			if err != nil {
				log4go.Error("fail to  insert into h_patient affect_count:")
				log4go.Error(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			} else {
				lastInsertId, err := affect_count.RowsAffected()
				if err != nil {
					log4go.Error("lastInsertId:")
					log4go.Error(lastInsertId)
					log4go.Error(err)
					//os.Exit(1)
					return c.String(http.StatusBadRequest, "error")
				}
			}
		} else {
			sqlstr = "update  h_patient set `PatientName`=?,`PatientSex`=?,`PatientBirthday`=?,`PatientAddr`=?," +
				"`PatientEmail`=?,`PatientCarID`=?,`PatientTelNumber`=?,`PatientType`=?," +
				"`PatientJob`=?,`PatientNation`=?,`PatientMarriage`=?,`PatientHometown`=?," +
				"`PatientHisID`=?,`PatientHistoryTell`=? where `PatientIdentity`=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				log4go.Error("------fail update PatientID:--------")
				log4go.Error(PatientID)
				log4go.Error(perr)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			if studyData.PatientType == "" {
				studyData.PatientType = "0"
			}
			affect_count, err := stmt.Exec(studyData.PatientName, studyData.PatientSex,
				studyData.PatientBirthday, studyData.PatientAddr, studyData.PatientEmail,
				studyData.PatientCarID, studyData.PatientTelNumber, studyData.PatientType,
				studyData.PatientJob, studyData.PatientNation, studyData.PatientMarriage,
				studyData.PatientHometown, studyData.PatientHisID, studyData.PatientHistoryTell, PatientIdentity)
			if err != nil {
				log4go.Error("fail to  update PatientI affect_count:")
				log4go.Error(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
		}
		//update order table
		StudyOrderIdentity := studyData.StudyOrderIdentity
		if StudyOrderIdentity == "" {
			StudyOrderIdentity = Units.GetRandUID()
			//if studyData.StudyID == "" {
			studyData.StudyID = Units.GetCurrentTime()
			studyData.StudyUID = Units.GetNewStudyUID()
			//}
			sqlstr = "insert into h_order (`StudyOrderIdentity`,`PatientIdentity`, `StudyID`,`StudyUID`, " +
				"`StudyModality`, `StudyAge`,`ScheduledDateTime`, " +
				"`AETitle`, `OrderDateTime`,`StudyDescription`, " +
				"`StudyDepart`,`StudyCode`, `StudyCost`,`CostType`, " +
				"`StudyType`, `StudyState`,`StudyDateTime`, `InstitutionName`," +
				"`ProcedureStepStartDate`,`StudyModalityIdentity`,`StudyManufacturer`,`RegisterID`)" +
				"values(?,?,?,?, ?,?,?,?, ?,?,?,?, ?,?,?,?, ?,?,?,?, ?,?)"
			stmt, err := maridb_db.Prepare(sqlstr)
			if err != nil {
				log4go.Error("------fail  maridb_db.Prepare(sqlstr) insert into h_order:--------")
				log4go.Error(StudyOrderIdentity)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			affect_count, err := stmt.Exec(StudyOrderIdentity, studyData.PatientIdentity,
				studyData.StudyID, studyData.StudyUID, studyData.StudyModality,
				studyData.StudyAge, studyData.ScheduledDateTime, studyData.AETitle,
				studyData.OrderDateTime, studyData.StudyDescription, studyData.StudyDepart, studyData.StudyCode,
				studyData.StudyCost, studyData.CostType, studyData.StudyType, studyData.StudyState,
				studyData.StudyDateTime, studyData.InstitutionName, studyData.ProcedureStepStartDate,
				studyData.StudyModalityIdentity, studyData.StudyManufacturer, studyData.RegisterID)
			if err != nil {
				log4go.Error(sqlstr)
				log4go.Error(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
		} else {
			sqlstr = "update h_order set `StudyModality`=?, `StudyAge`=?,`ScheduledDateTime`=?," +
				"`AETitle`=?, `OrderDateTime`=?,`StudyDescription`=?," +
				"`StudyDepart`=?, `StudyCode`=?,`StudyCost`=?,`CostType`=?," +
				"`StudyType`=?, `StudyState`=?,`StudyDateTime`=?,`InstitutionName`=?,`ProcedureStepStartDate`=?," +
				"`StudyModalityIdentity`=?  ,`StudyManufacturer`=?,`RegisterID`=? where StudyOrderIdentity=?"
			stmt, perr := maridb_db.Prepare(sqlstr)
			if perr != nil {
				log4go.Error(sqlstr)
				log4go.Error("StudyOrderIdentity:" + StudyOrderIdentity)
				log4go.Error(perr)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
			affect_count, err := stmt.Exec(studyData.StudyModality, studyData.StudyAge, studyData.ScheduledDateTime,
				studyData.AETitle, studyData.OrderDateTime, studyData.StudyDescription,
				studyData.StudyDepart, studyData.StudyCode, studyData.StudyCost, studyData.CostType,
				studyData.StudyType, studyData.StudyState, studyData.StudyDateTime, studyData.InstitutionName,
				studyData.ProcedureStepStartDate, studyData.StudyModalityIdentity,
				studyData.StudyManufacturer, studyData.RegisterID, StudyOrderIdentity)
			if err != nil {
				log4go.Error("studyData.ScheduledDate" + studyData.ScheduledDateTime)
				log4go.Error(affect_count)
				log4go.Error(err)
				//os.Exit(1)
				return c.String(http.StatusBadRequest, "error")
			}
		}
	} else {
		log4go.Error("maridb_db == nil ")
	}
	return c.String(http.StatusOK, "OK")
}

///--------------------------------------------------------------------------------------------------------------

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

///https://www.itread01.com/content/1494477725.html
// func initLogger() {
// 	var filenameOnly string
// 	filenameOnly = GetCurFilename()
// 	var logFilename string = filenameOnly + ".log"
// 	//gLogger = log4go.NewLogger()
// 	gLogger = make(log4go.Logger)
// 	//for console
// 	//gLogger.AddFilter("stdout", log4go.INFO, log4go.NewConsoleLogWriter())
// 	gLogger.AddFilter("stdout", log4go.INFO, log4go.NewConsoleLogWriter())
// 	//for log file
// 	if _, err := os.Stat(logFilename); err == nil {
// 		//fmt.Printf("found old log file %s, now remove it\n", logFilename)
// 		os.Remove(logFilename)
// 	}
// 	//gLogger.AddFilter("logfile", log4go.FINEST, log4go.NewFileLogWriter(logFilename, true))
// 	gLogger.AddFilter("logfile", log4go.FINEST, log4go.NewFileLogWriter(logFilename, false))
// 	gLogger.Info("Current time is : %s", time.Now().Format("15:04:05 MST 2006/01/02"))
// 	return
// }
