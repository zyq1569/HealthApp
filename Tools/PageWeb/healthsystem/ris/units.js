var g_tinyID = 'ReportArea';
var g_initContent = 'frist';
var g_reportData = {
    ReportIdentity: 0,
    StudyOrderIdentity: 0,
    ReportTemplate: '',
    ReportCreatDate: '',
    ReportWriterID: '',
    ReportCheckID: '',
    ReportCheckDate: '',
    ReportContent: '',
    ReportState: '0', //????? 0.空文档 1.草稿 2.加载 3.审核
    ReportSave: false,
    ReportChange: false,
    ReportOther: ''
};

var g_currentReportOrderIdentity = '';
var g_currentReportData = g_reportData;
var g_server_json_report_url = '/healthsystem/ris/saveportdata/';

function getFormatDateTime() {
    var date = new Date();
    var y = date.getFullYear();
    var m = date.getMonth() + 1;
    m = m < 10 ? ('0' + m) : m;
    var d = date.getDate();
    d = d < 10 ? ('0' + d) : d;
    var h = date.getHours();
    h = h < 10 ? ('0' + h) : h;
    var minute = date.getMinutes();
    var second = date.getSeconds();
    minute = minute < 10 ? ('0' + minute) : minute;
    second = second < 10 ? ('0' + second) : second;
    //var currentDateTime = y + '' + m + '' + d + '' + h + '' + minute + '' + second;
    var currentDateTime = '' + y + m + d + h + minute + second;
    return currentDateTime;
}

function getCurrentFormatDateTime() {
    return getFormatDateTime();
}
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