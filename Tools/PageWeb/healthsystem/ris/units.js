var g_tinyID = 'ReportArea';
var g_initContent = 'frist';
var g_mess_savereportfail = 'please go to report editer! save report fail!';
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
// to do 深拷贝 浅拷贝 ????
var g_currentReportData = {
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

var g_server_json_report_url = '/healthsystem/ris/saverereportdata/';

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
    var date = new Date();
    laydate.render({
        elem: '#stduystart', //指定元素
        format: 'yyyyMMdd',
        value: date,
        calendar: true
    });
    laydate.render({
        elem: '#stduyend', //指定元素
        format: 'yyyyMMdd',
        value: date,
        calendar: true
    });
    laydate.render({
        elem: '#imagestduystart', //指定元素
        format: 'yyyyMMdd',
        value: date,
        calendar: true
    });
    laydate.render({
        elem: '#imagestduyend', //指定元素
        format: 'yyyyMMdd',
        value: date,
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

// $.ajax({
//     url: 'http://.......',
//     type: "post",
//     timeout: 5000,
//     async: true,
//     cache: true,
//     data: "user_id=12&page=0",
//     dataType: "json",
//     contentType: "application/x-www-form-urlencoded",
//     beforeSend: function(XMLHttpRequest) {
//         console.log(this);
//         $("#inp").val("正在获取数据...");
//     },
//     success: function(data) {
//         console.log(data);
//         $(".display").html("获取到的数据:</br>");
//         $(".display").append("总条数:" + data.data.all_count);
//         $("#inp").val("点击获取数据");
//     },
//     complete: function(XMLHttpRequest, textStatus) {
//         if (textStatus == 'timeout') {
//             var xmlhttp = window.XMLHttpRequest ? new window.XMLHttpRequest() : new ActiveXObject("Microsoft.XMLHttp");
//             xmlhttp.abort();
//             $(".box").html("网络超时！");　　　　
//         }
//         $("#inp").val("点击获取数据");
//     },
//     error: function(XMLHttpRequest, textStatus) {
//         console.log(XMLHttpRequest); //XMLHttpRequest.responseText    XMLHttpRequest.status   XMLHttpRequest.readyState
//         console.log(textStatus);
//         $(".box").html("服务器错误！");
//     }
// });