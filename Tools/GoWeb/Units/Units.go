// Units
package Units

import (
	"fmt"
	"math/rand"
	"strconv"
	"strings"
	"time"
)

const (
	PRE_STUDYUID = "1.2.826.0.1.3680043.9.7604."
)

type OFHashValue struct {
	first  string
	second string
}

func GetStudyHashDir(studyuid string, studyDate string) string {
	studyDate = strings.Replace(studyDate, "-", "", -1)
	studyDate = strings.Replace(studyDate, ":", "", -1)
	studyDate = strings.Replace(studyDate, " ", "", -1)
	dir := "/" + studyDate[0:4] + "/" + studyDate[4:6] + "/" + studyDate[6:8] + "/" + studyDate[8:10] + "/" + studyDate[10:12] + "/" + studyuid
	return dir

	// pos := strings.Index(studyuid, "|")
	// if pos > 1 {
	// 	// 2001 01 09 08 42 47
	// 	// 0123 45 67 89 0
	// 	//log4go.Info("studyuid:" + studyuid)
	// 	datestr := studyuid[pos+1:]
	// 	//log4go.Info("datestr:" + datestr)
	// 	year := datestr[0:3]  //2001
	// 	month := datestr[4:5] //01
	// 	day := datestr[6:7]   //09
	// 	hour := datestr[8:9]  //08
	// 	min := datestr[10:11] //42
	// 	dir := "/" + year + "/" + month + "/" + day + "/" + hour + "/" + min
	// 	//log4go.Info("dir:" + dir)
	// 	return dir
	// }

	// vl := CreateHashValue([]byte(studyuid), strings.Count(studyuid, "")-1)
	// dir := "/" + vl.first + "/" + vl.second
	// return dir
}

func GetCurrentTime() string {
	st := time.Now().String()
	st = strings.ReplaceAll(st, "-", "")
	st = strings.ReplaceAll(st, ":", "")
	st = strings.ReplaceAll(st, ".", "")
	st = strings.ReplaceAll(st, " ", "")
	st = st[0:21]
	return st
	// fmt.Println(st)
	// fmt.Println(time.Now())
	// return tm.Format("20060102130405000000")
}

func GetCurrentFormatTime() string {
	st := time.Now().String()
	st = st[0:19]
	return st
}

func GetRandUID() string {
	rand.Seed(int64(time.Now().UnixNano()))
	return strconv.Itoa(rand.Int())
}

func GetNewStudyUID() string {
	return PRE_STUDYUID + GetCurrentTime()
}

func CreateHashValue(buffer []byte, length int) OFHashValue {
	//2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
	//101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
	//181 191 193 197 199
	// var a string = "Runoob"
	var len int = 0
	len = length

	const M1 int = 71
	const M2 int = 79

	const D1 int = 197
	const D2 int = 199
	var value int = M1 + D1
	var hashvalue OFHashValue
	var i int = 0
	for i = 0; i < len; i++ {
		value = (value+(int)(buffer[i]))*M1 + i*D1
	}
	hashvalue.first = fmt.Sprintf(hashvalue.first, "%x", value)

	value = M2 + D2
	for i = 0; i < len; i++ {
		value = (value+(int)(buffer[i]))*M2 + i*D2
	}
	hashvalue.second = fmt.Sprintf(hashvalue.first, "%x", value)

	// println(hashvalue)
	return hashvalue
}

// type OFHashValue struct {
// 	first  uint
// 	second uint
// }

// func GetStudyHashDir(studyuid string) string {
// 	vl := CreateHashValue([]byte(studyuid), (uint)(strings.Count(studyuid, "")-1))
// 	dir := "/" + strconv.Itoa((int)(vl.first)) + "/" + strconv.Itoa((int)(vl.second))
// 	return dir
// }

// func CreateHashValue(buffer []byte, length int) OFHashValue {
// 	//2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
// 	//101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
// 	//181 191 193 197 199
// }
