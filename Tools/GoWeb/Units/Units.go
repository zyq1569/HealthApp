// Units
package Units

import (
	"strconv"
	"strings"
)

type OFHashValue struct {
	first  int
	second int
}

func GetStudyHashDir(studyuid string) string {
	vl := CreateHashValue([]byte(studyuid), strings.Count(studyuid, "")-1)
	dir := "/" + strconv.Itoa(vl.first) + "/" + strconv.Itoa(vl.second)
	return dir
}

func CreateHashValue(buffer []byte, length int) OFHashValue {
	//2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
	//101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
	//181 191 193 197 199
	// var a string = "Runoob"
	const M1 int = 71
	const M2 int = 79

	const D1 int = 197
	const D2 int = 199
	var value int = 0
	var hashvalue OFHashValue
	for i := 0; i < length; i++ {
		value = value*M1 + (int)(buffer[i]&0xFF)
	}
	value = value % D1
	if value < 0 {
		value = value + D1
	}
	hashvalue.first = value

	value = 0
	for i := 0; i < length; i++ {
		value = value*M2 + (int)(buffer[i]&0xFF)
	}
	value = value % D2
	if value < 0 {
		value = value + D2
	}
	hashvalue.second = value
	return hashvalue
}
