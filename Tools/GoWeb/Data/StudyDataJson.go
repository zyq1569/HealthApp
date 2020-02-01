// StudyDataJson
package Study

type StudyDataJson struct {
	Code  int         `json:"code"`
	Msg   string      `json:"msg"`
	Count int         `json:"count"`
	Data  []StudyData `json:"data"`
}

type StudyData struct {
	PatientIdentity    string `json:"patientIdentity"`
	PatientName        string `json:"patientName"`
	PatientID          string `json:"patientId"`
	PatientSex         string `json:"patientSex"`
	PatientBirthday    string `json:"patientBirthday"`
	PatientTelNumber   string `json:"patientTelNumber"`
	PatientAddr        string `json:"patientAddr"`
	PatientCarID       string `json:"patientCarID"`
	PatientType        string `json:"patientType"`
	PatientEmail       string `json:"patientEmail"`
	StudyOrderIdentity string `json:"studyOrderIdentity"`
	StudyID            string `json:"studyId"`
	StudyUID           string `json:"studyuid"`
	ScheduledDateTime  string `json:"scheduledDateTime"`
	OrderDateTime      string `json:"orderDateTime"`
	StudyDescription   string `json:"studyDescription"`
	StudyModality      string `json:"studyModality"`
	AETitle            string `json:"aETitle"`
	StudyType          string `json:"studyType"`
	StudyCode          string `json:"studyCode"`
	StudyState         string `json:"studyState"`
	StudyCost          string `json:"studyCost"`
	StudyDateTime      string `json:"studyDate"`
	// SstudyID           string `json:"sStudyID"`
	StudyDepart    string `json:"studyDepart"`
	SstudyModality string `json:"sStudyModality"`
	// SstudyUID          string `json:"sStudyUID"`
	CostType string `json:"costType"`
}

type ReportData struct {
	ReportIdentity     string
	StudyOrderIdentity string
	ReportTemplate     string
	ReportCreatDate    string
	ReportWriterID     string
	ReportCheckID      string
	ReportCheckDate    string
	ReportContent      string
	ReportState        string
	ReportSave         string
	ReportChange       string
	ReportOther        string
}
