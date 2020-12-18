// StudyDataJson
package Study

type StudyDataJson struct {
	Code  int         `json:"code"`
	Msg   string      `json:"msg"`
	Count int         `json:"count"`
	Data  []StudyData `json:"data"`
}

///----abandon 20201218-----------
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

///////-------------------------------------------------------------------------

///-------------2020-11-17 add :------------------------------------------------
type StudyOrderDataJson struct {
	Code  int              `json:"code"`
	Msg   string           `json:"msg"`
	Count int              `json:"count"`
	Data  []StudyOrderData `json:"data"`
}
type StudyOrderData struct {
	///-----------patient table------------------------
	PatientIdentity    string ///`json:"patientIdentity"`
	PatientID          string ///`json:"patientID"`
	PatientName        string ///`json:"patientName"`
	PatientNameEnglish string ///`json:"patientNameEnglish"`
	PatientSex         string ///`json:"patientSex"`
	PatientBirthday    string ///`json:"patientBirthday"`
	PatientAddr        string ///`json:"patientAddr"`
	PatientEmail       string ///`json:"patientEmail"`
	PatientCarID       string ///`json:"patientCarID"`
	PatientTelNumber   string ///`json:"patientTelNumber"`
	PatientType        string ///`json:"patientType"`
	PatientState       string ///`json:"patientState"`
	PatientJob         string ///`json:"patientJob"`
	PatientNation      string ///`json:"patientNation"`
	PatientMarriage    string ///`json:"patientMarriage"`
	PatientHometown    string ///`json:"patientHometown"`
	PatientHisID       string ///`json:"patientHisID"`
	PatientHistoryTell string ///`json:"patientHistoryTell"`
	///--------order table--------------
	StudyOrderIdentity string ///`json:"studyOrderIdentity"`
	///--PatientIdentity ----------
	StudyID                string ///`json:"studyID"`
	ClinicID               string ///`json:"clinicID"`
	StudyUID               string ///`json:"studyUID"`
	StudyModality          string ///`json:"studyModality"`
	StudyAge               string ///`json:"studyAge"`
	ScheduledDateTime      string ///`json:"scheduledDateTime"`
	AETitle                string ///`json:"aETitle"`
	OrderDateTime          string ///`json:"orderDateTime"`
	StudyDescription       string ///`json:"studyDescription"`
	StudyDepart            string ///`json:"studyDepart"`
	StudyCode              string ///`json:"studyCode"`
	StudyCost              string ///`json:"studyCost"`
	CostType               string ///`json:"costType"`
	StudyType              string ///`json:"studyType"`
	StudyState             string ///`json:"studyState"`
	StudyDateTime          string ///`json:"studyDateTime"`
	InstitutionName        string ///`json:"institutionName"`
	ProcedureStepStartDate string ///`json:"procedureStepStartDate"`
	StudyModalityIdentity  string ///`json:"studyModalityIdentity"`
	StudyManufacturer      string ///`json:"studyManufacturer"`
	RegisterID             string ///`json:"registerID"`
}

type JsonCode struct {
	Code int         `json:"code"`
	Msg  string      `json:"msg"`
	Data interface{} ` json:"data"`
}
