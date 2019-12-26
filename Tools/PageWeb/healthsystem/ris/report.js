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
    layer.msg('ReportIdentity:' + data.ReportIdentity);
    tinyMCE.editors[g_tinyID].setContent(data.ReportConten);
    g_initContent = data.ReportConten;
}
// 清空内容
function clearReportContent() {
    tinyMCE.editors[g_tinyID].setContent('');
}

function saveReportContent(data) {
    // tinyMCE.editors[g_tinyID].getContent(g_initContent);
    var postdata = JSON.stringify(data.field);
    var host = window.location.host;
    $.ajax({
        type: "POST",
        url: host + '/healthsystem/ris/Report/',
        async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
        data: postdata, //请求save处理数据
        success: function(mess) {
            if (mess == "OK") { //根据返回值进行跳转
                // layer.alert(mess + '--save ok!' + postdata);
                tableObj.reload(options); //重载表格
                layer.msg('--save ok!--' + postdata);
                ChangeTab('TabBrief', 'layid_addedit');
                //table.render("studytabledatas");
            } else {
                layer.alert(mess + "--save fail:" + postdata);
            }
        }
    });
}
//  tinymce editer