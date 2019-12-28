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

// 设置内容
function setStudyReportContent(data) {
    if (data.ReportIdentity == '' || data.StudyOrderIdentity == '') {
        layer.alert("data(ReportIdentity =='' or StudyOrderIdentity == '') error!");
        return -1;
    }
    if (g_currentReportOrderIdentity == data.ReportIdentity) {
        return 1;
    }
    if (g_currentReportOrderIdentity != '') {
        if (saveReport2ServerContent() < 0) {
            layer.alert(g_mess_savereportfail);
            return;
        }
    }
    g_currentReportOrderIdentity = data.ReportIdentity;
    g_currentReportData = data;
    if (g_currentReportData.ReportCreatDate == '') {
        g_currentReportData.ReportCreatDate = getFormatDateTime();
    }
    if (g_currentReportData.ReportCheckDate == '') {
        g_currentReportData.ReportCheckDate = getFormatDateTime();
    }
    //layer.alert('ok:' + data.ReportContent);
    tinyMCE.editors[g_tinyID].setContent(g_currentReportData.ReportContent);
    g_initContent = data.ReportContent;
    return 1;

}
// 清空内容
function clearReportContent() {
    tinyMCE.editors[g_tinyID].setContent('');
    g_currentReportOrderIdentity = '';
}

function saveReport2ServerContent() {
    var content = tinyMCE.editors[g_tinyID].getContent();
    if (content.length < 1) {
        return 1;
    }
    if (g_currentReportData.ReportSave == true) {
        return 1;
    }
    //判断是否修改过内容
    if (content == g_currentReportData.ReportContent) {
        return 1;
    }
    //修改过内容,保存到数据库中
    g_currentReportData.ReportContent = content;
    g_currentReportData.ReportSave = false;
    var postdata = JSON.stringify(g_currentReportData);
    var host = window.location.host;
    $.ajax({
        type: "POST",
        url: host + '/healthsystem/ris/saveportdata/',
        async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
        data: postdata, //请求save处理数据
        success: function(mess) {
            if (mess == "OK") { //根据返回值进行跳转
                g_currentReportData.ReportSave = true;
                layer.msg('--save ok!--' + postdata);
                return 1;
            } else {
                layer.alert(mess + "--save fail:" + postdata);
                return -1;
            }
        },
        error: function(XMLHttpRequest, textStatus, errorThrown) {
            layer.alert("--save fail:" + XMLHttpRequest + textStatus + postdata);
            return -1;
        }
    });
}
//  tinymce editer