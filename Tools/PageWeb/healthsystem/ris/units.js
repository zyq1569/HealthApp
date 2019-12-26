var g_tinyID = 'ReportArea';
var g_initContent = 'frist';
var g_reportData = {
    ReportIdentity: 0,
    StudyOrderIdentity: 0,
    ReportModality: '',
    ReportCreatDate: '',
    ReportWriterID: '',
    ReportCheckID: '',
    ReportCheckDate: '',
    ReportDate: '',
    ReportConten: '',
    ReportState: '0', //????? 0.空文档 1.草稿 2.加载 3.审核
    ReportSave: false,
    ReportChange: false,
    ReportOther: ''
};

var g_currentReportOrderIdentity = '';

layui.use('laydate', function() {
    var laydate = layui.laydate; //执行一个laydate实例
    laydate.render({
        elem: '#stduystart', //指定元素
        format: 'yyyyMMdd',
        calendar: true
    });
    laydate.render({
        elem: '#stduyend', //指定元素
        format: 'yyyyMMdd',
        calendar: true
    });
    laydate.render({
        elem: '#birthdate', //指定元素
        format: 'yyyyMMdd',
        calendar: true
    });
    laydate.render({
        elem: '#ScheduledDate', //指定元素
        format: 'yyyyMMdd',
        calendar: true
    });
});