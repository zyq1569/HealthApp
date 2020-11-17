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
	ScheduledDate      string `json:"ScheduledDate"`
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
	ReportIdentity     string `json:"ReportIdentity"`
	StudyOrderIdentity string `json:"StudyOrderIdentity"`
	ReportTemplate     string `json:"ReportTemplate"`
	ReportCreatDate    string `json:"ReportCreatDate"`
	ReportWriterID     string `json:"ReportWriterID"`
	ReportCheckID      string `json:"ReportCheckID"`
	ReportCheckDate    string `json:"ReportCheckDate"`
	ReportContent      string `json:"ReportContent"`
	ReportState        string `json:"ReportState"`
	ReportSave         string `json:"ReportSave"`
	ReportChange       string `json:"ReportChange"`
	ReportOther        string `json:"ReportOther"`
}

/// 2020-11-17 add :
type StudyOrderDataJson struct {
	Code  int              `json:"code"`
	Msg   string           `json:"msg"`
	Count int              `json:"count"`
	Data  []StudyOrderData `json:"data"`
}
type StudyOrderData struct {
	/// patient table
	PatientIdentity    string
	PatientID          string
	PatientName        string
	PatientNameEnglish string
	PatientSex         string
	PatientBirthday    string
	PatientAddr        string
	PatientEmail       string
	PatientCarID       string
	PatientTelNumber   string
	PatientType        string
	PatientState       string
	/// order table
	StudyOrderIdentity string
	///PatientIdentity        string
	StudyID                string
	StudyUID               string
	StudyModality          string
	StudyAge               string
	ScheduledDateTime      string
	AETitle                string
	OrderDateTime          string
	StudyDescription       string
	StudyDepart            string
	StudyCode              string
	StudyCost              string
	CostType               string
	StudyType              string
	StudyState             string
	StudyDateTime          string
	InstitutionName        string
	ProcedureStepStartDate string
	StudyModalityIdentity  string
	StudyManufacturer      string
	RegisterID             string
}

type JsonCode struct {
	Code int         `json:"code"`
	Msg  string      `json:"msg"`
	Data interface{} ` json:"data"`
}
