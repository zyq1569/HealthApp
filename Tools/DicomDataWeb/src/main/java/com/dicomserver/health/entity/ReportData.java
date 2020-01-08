package com.dicomserver.health.entity;

public class ReportData {
    private String ReportIdentity;
    private String StudyOrderIdentity;
    private String ReportTemplate;
    private String ReportCreatDate;
    private String ReportWriterID;
    private String ReportCheckID;
    private String ReportCheckDate;
    private String ReportContent;
    private String ReportState; //????? 0.空文档 1.草稿 2.加载 3.审核
    private String ReportSave;
    private String ReportChange;
    private String ReportOther;

    public String getReportIdentity(){      return ReportIdentity;    }
    public void setReportIdentity(String ReportIdentity ){      this.ReportIdentity = ReportIdentity;   }
    public String getReportTemplate(){      return ReportTemplate;    }
    public void setReportTemplate(String ReportTemplate){       this.ReportTemplate = ReportTemplate;   }
    public String getStudyOrderIdentity(){  return StudyOrderIdentity;}
    public void setStudyOrderIdentity(String StudyOrderIdentity){       this.StudyOrderIdentity = StudyOrderIdentity;   }
    public String getReportCreatDate(){     return ReportCreatDate;   }
    public void setReportCreatDate(String ReportCreatDate){       this.ReportCreatDate = ReportCreatDate;   }
    public String getReportWriterID(){      return ReportWriterID;    }
    public void setReportWriterID(String ReportWriterID){       this.ReportWriterID = ReportWriterID;   }
    public String getReportCheckID(){       return ReportCheckID;     }
    public void setReportCheckID(String ReportCheckID){       this.ReportCheckID = ReportCheckID;   }
    public String getReportCheckDate(){     return ReportCheckDate;   }
    public void setReportCheckDate(String ReportCheckDate){       this.ReportCheckDate = ReportCheckDate;   }
    public String getReportContent(){       return ReportContent;     }
    public void setReportContent(String ReportContent){       this.ReportContent = ReportContent;   }
    public String getReportState(){return  ReportState;}
    public void setReportState(String ReportState){       this.ReportState = ReportState;   }
    public String getReportSave(){return  ReportSave;}
    public void setReportSave(String ReportSave){       this.ReportSave = ReportSave;   }
    public String getReportChange(){return  ReportChange;}
    public void setReportChange(String ReportChange){       this.ReportChange = ReportChange;   }
    public String getReportOther(){return  ReportOther;}
    public void setReportOther(String ReportOther){       this.ReportOther = ReportOther;   }
}
