// var script = document.createElement("script");
// script.type = "text/javascript";
// script.src =  "prefixfree.min.js";
tinymce.init({
    selector: '#ReportArea',
    // inline: true,
    height: 700,
    // plugins: "fullpage",
    // menubar: "file",
    // toolbar: "fullpage",
    fullpage_default_encoding: "UTF-8",
    plugins: [
        'advlist autolink lists link image charmap print preview anchor',
        'searchreplace visualblocks code fullscreen',
        'insertdatetime media table paste code help wordcount',
        'autosave'
    ],
    toolbar: 'undo redo | formatselect | ' +
        ' bold italic backcolor | alignleft aligncenter ' +
        ' alignright alignjustify | bullist numlist outdent indent |' +
        ' removeformat | help',
    menubar: 'edit view insert format tools table tc help',
    autosave_restore_when_empty: true,
    autosave_interval: "10s",
    init_instance_callback: function(editor) {
        // editor.notificationManager.open({
        //     text: 'report ready ok!',
        // });
        clearReportContent();
        //tinyMCE.editor[g_tinyID].setContent("");
    }
});
// 获得内容
function getReportContent() {
    g_initContent = tinyMCE.editors[g_tinyID].getContent();
    layer.alert(g_initContent);
}
// 设置内容
function setReportContent() {
    tinyMCE.editors[g_tinyID].setContent(g_initContent);
}

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
// 设置内容
function setStudyReportContent(data) {
    if (g_currentReportOrderIdentity == data.ReportIdentity) {
        layer.alert('Same study report!');
        return;
    } else {
        g_currentReportOrderIdentity = data.ReportIdentity;
        g_currentReportData = data;
        if (g_currentReportData.ReportCreatDate == '') {
            g_currentReportData.ReportCreatDate = getFormatDateTime();
            g_currentReportData.ReportCheckDate = getFormatDateTime();
            //layer.alert('-----ReportCreatDate|' + g_currentReportData.ReportCreatDate);
        }
    }
    //layer.msg('ReportIdentity:' + g_currentReportData.ReportIdentity);
    tinyMCE.editors[g_tinyID].setContent(g_currentReportData.ReportContent);
    g_initContent = data.ReportContent;
}
// 清空内容
function clearReportContent() {
    tinyMCE.editors[g_tinyID].setContent('');
    g_currentReportOrderIdentity = '';
}

function saveReport2ServerContent() {
    // tinyMCE.editors[g_tinyID].getContent(g_initContent);
    g_currentReportData.ReportContent = tinyMCE.editors[g_tinyID].getContent();
    var postdata = JSON.stringify(g_currentReportData);
    //layer.alert('-----postdata|' + postdata);
    var host = window.location.host;
    $.ajax({
        type: "POST",
        url: host + '/healthsystem/ris/reportdata/',
        async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
        data: postdata, //请求save处理数据
        success: function(mess) {
            if (mess == "OK") { //根据返回值进行跳转
                layer.msg('--save ok!--' + postdata);
            } else {
                layer.alert(mess + "--save fail:" + postdata);
            }
        }
    });
}
//  tinymce editer