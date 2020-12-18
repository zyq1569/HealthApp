// patient report code
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
    plugins: ['save',
        'advlist autolink lists link image charmap print preview anchor',
        'searchreplace visualblocks code fullscreen',
        'insertdatetime media table paste code help wordcount',
        'autosave', 'emoticons', 'fullscreen'
    ],
    toolbar: 'save |' +
        'SaveReport |undo redo|formatselect| ' +
        ' bold italic backcolor| alignleft aligncenter ' +
        ' alignright alignjustify | bullist numlist outdent indent |' +
        ' removeformat|visualchars|emoticons|fullscreen|charmap|GetContent|ClearContent|ReturnPatient|print|help',
    menubar: 'edit view insert format tools table tc help',
    autosave_restore_when_empty: true,
    save_enablewhendirty: true,
    autosave_interval: "10s",
    save_onsavecallback: function () {
        if (saveReport2ServerContent() > 1) {
            layer.msg('save ok!');
        }
    },
    init_instance_callback: function (editor) {
        // editor.notificationManager.open({
        //     text: 'report ready ok!',
        // });
        clearReportContent();
        //tinyMCE.editor[g_tinyID].setContent("");
    },
    setup: function (editor) {
        editor.ui.registry.addButton('SaveReport', {
            text: 'SaveReport',
            onAction: function (_) {
                if (saveReport2ServerContent() > 1) {
                    layer.msg('save ok!');
                }
                //editor.insertContent('&nbsp;<strong>It\'s my button!</strong>&nbsp;');
            }
        });
        editor.ui.registry.addButton('ReturnPatient', {
            text: 'ReturnPatient',
            onAction: function (_) {
                ChangeTab('TabBrief', 'layid_patients');
            }
        });
        editor.ui.registry.addButton('ClearContent', {
            text: 'ClearContent',
            onAction: function (_) {
                clearReportContent();
            }
        });
        editor.ui.registry.addButton('GetContent', {
            text: 'GetContent',
            onAction: function (_) {
                getReportContent();
            }
        });
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
    g_currentReportData.ReportSave = false;
    return 1;

}
// 清空内容
function clearReportContent() {
    tinyMCE.editors[g_tinyID].setContent('');
    g_currentReportOrderIdentity = '';
}

function saveReport2ServerContent() {
    if (g_currentReportOrderIdentity == '') {
        layer.alert('no patients info! pls select patient!');
        return;
    }
    var content = tinyMCE.editors[g_tinyID].getContent();
    if (content.length < 1) {
        // console.log('content.length < 1');
        return 1;
    }
    // if (g_currentReportData.ReportSave == true) {
    //     // console.log('g_currentReportData.ReportSave == true');
    //     return 1;
    // }
    //判断是否修改过内容
    if (content == g_currentReportData.ReportContent && g_currentReportOrderIdentity == g_currentReportData.ReportIdentity) {
        // console.log('content == g_currentReportData.ReportContent && g_currentReportOrderIdentity == g_currentReportData.ReportIdentity');
        return 1;
    }
    //修改过内容,保存到数据库中
    g_currentReportData.ReportContent = content;
    g_currentReportData.ReportSave = false;
    var postdata = JSON.stringify(g_currentReportData);
    var host = window.location.host;
    $.ajax({
        type: "POST",
        url: 'http://' + host + '/healthsystem/ris/savereportdata/',
        async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
        data: postdata, //请求save处理数据
        success: function (mess) {
            if (mess.toUpperCase() == "OK") { //根据返回值进行跳转
                g_currentReportData.ReportSave = true;
                layer.msg('--save ok!--'); //+ postdata
                return 1;
            } else {
                layer.alert(mess + "--save fail:" + postdata);
                return -1;
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            layer.alert("--save fail:" + XMLHttpRequest + textStatus + postdata);
            return -1;
        }
    });
}

function saveReport2ServerOfChange() {
    if (g_currentReportOrderIdentity == '') {
        layer.alert('no patients info! pls select patient!');
        return;
    }
    var content = tinyMCE.editors[g_tinyID].getContent();
    if (content.length < 1) {
        // console.log('content.length < 1');
        return 1;
    }
    if (g_currentReportData.ReportSave == true) {
        // console.log('g_currentReportData.ReportSave == true');
        layer.msg('--No Modify--');
        return 1;
    }
    //判断是否修改过内容
    // if (content == g_currentReportData.ReportContent && g_currentReportOrderIdentity == g_currentReportData.ReportIdentity) {
    //     // console.log('content == g_currentReportData.ReportContent && g_currentReportOrderIdentity == g_currentReportData.ReportIdentity');
    //     return 1;
    // }
    //修改过内容,保存到数据库中
    g_currentReportData.ReportContent = content;
    g_currentReportData.ReportSave = false;
    var postdata = JSON.stringify(g_currentReportData);
    var host = window.location.host;
    $.ajax({
        type: "POST",
        url: 'http://' + host + '/healthsystem/ris/savereportdata/',
        async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
        data: postdata, //请求save处理数据
        success: function (mess) {
            if (mess.toUpperCase() == "OK") { //根据返回值进行跳转
                g_currentReportData.ReportSave = true;
                layer.msg('--save ok!--'); //+ postdata
                return 1;
            } else {
                layer.alert(mess + "--save fail:" + postdata);
                return -1;
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            layer.alert("--save fail:" + XMLHttpRequest + textStatus + postdata);
            return -1;
        }
    });
}
//  tinymce editer