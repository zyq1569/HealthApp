//表格设置
// console.log(window.location.host);
// console.log(window.location.port);

function SetFormValue(jsonstring) {
    var data = JSON.parse(jsonstring);
    //layer.alert(data);
    //给表单赋值
    layui.form.val("formpatient", {
        "PatientName": data.patientName,
        "PatientSex": data.patientSex,
        "PatientBirthday": data.patientBirthday,
        "PatientTelNumber": data.patientTelNumber,
        "PatientEmail": data.patientEmail,
        "PatientCarID": data.patientCarID,
        "PatientAddr": data.patientAddr,
        "StudyCost": data.studyCost,
        "StudyDepart": data.studyDepart,
        "StudyModality": data.studyModality,
        "ScheduledDate": data.scheduledDateTime,
        "CostType": data.costType,
        "StudyDescription": data.studyDescription,
        "PatientID": data.patientId,
        "PatientIdentity": data.patientIdentity,
        "StudyOrderIdentity": data.studyOrderIdentity
    });
}

function AddOrderStudyOfPatient(jsonstring) {
    var data = JSON.parse(jsonstring);
    //layer.alert(data);
    //给表单赋值
    layui.form.val("formpatient", {
        "PatientName": data.patientName,
        "PatientSex": data.patientSex,
        "PatientBirthday": data.patientBirthday,
        "PatientTelNumber": data.patientTelNumber,
        "PatientEmail": data.patientEmail,
        "PatientCarID": data.patientCarID,
        "PatientAddr": data.patientAddr,
        // "StudyCost": data.studyCost,
        // "StudyDepart": data.studyDepart,
        // "StudyModality": data.studyModality,
        // "ScheduledDate": data.scheduledDateTime,
        // "CostType": data.costType,
        // "StudyDescription": data.studyDescription,
        "PatientID": data.patientId,
        "PatientIdentity": data.patientIdentity,
        // "StudyOrderIdentity": data.studyOrderIdentity
    });
}

function ClearFormValue() {
    //layer.alert(data);
    //给表单赋值
    layui.form.val("formpatient", {
        "PatientName": "",
        "PatientSex": "",
        "PatientBirthday": "",
        "PatientTelNumber": "",
        "PatientEmail": "",
        "PatientCarID": "",
        "PatientAddr": "",
        "StudyCost": "",
        "StudyDepart": "",
        "StudyModality": "",
        "ScheduledDate": "",
        "CostType": "",
        "StudyDescription": "",
        "PatientID": "",
        "PatientIdentity": "",
        "StudyOrderIdentity": ""
    });
}

function ChangeTab(table, item) {
    ClearFormValue(); // don't submit data ,to do ....
    layui.element.tabChange(table, item);
}

var searchImageTime = 'start=19700101&end=20191230';
var searchStudyTime = 'start=19700101&end=20191230';

layui.use(['laypage', 'table', 'element', 'upload', 'form'], function () {
    var laypage = layui.laypage, //分页
        table = layui.table, //表格
        element = layui.element, //元素操作
        form = layui.form, //表单
        upload = layui.upload; //上传
    layer.msg('show patients study datas');

    //监听Tab切换
    element.on('tab(TabBrief)', function (data) {
        // layer.tips('切换了 ' + data.index + '：' + this.innerHTML, this, {
        //     tips: 1
        // });
    });
    var $ = layui.$,
        active = {
            reload: function () {
                var demoReload = $('#searchReload');
                //执行重载
                table.reload('testReload', {
                    page: {
                        curr: 1 //重新从第 1 页开始
                    },
                    where: {
                        key: {
                            id: demoReload.val()
                        }
                    }
                }, 'data');
            }
        };

    $('.SerarchTable .layui-btn').on('click', function () {
        searchStudyTime = 'start=' + $('#stduystart').val() + '&end=' + $('#stduyend').val();
        tablePatient.reload({
            //window.location.host + '/healthsystem/ris/stduydata/?start=20190101&end=20191219'
            url: 'http://' + window.location.host + '/healthsystem/ris/studydata/?' + searchStudyTime
        });
        // var type = $(this).data('type');
        // active[type] ? active[type].call(this) : '';
    });
    $('.SerarchImageTable .layui-btn').on('click', function () {
        searchImageTime = 'start=' + $('#imagestduystart').val() + '&end=' + $('#imagestduyend').val();
        tableStudyImage.reload({
            url: 'http://' + window.location.host + '/healthsystem/ris/stduyimage/?' + searchImageTime
        });
        // var type = $(this).data('type');
        // active[type] ? active[type].call(this) : '';
    });

    //监听头工具栏事件
    table.on('toolbar(studytabledatas)', function (obj) {
        var checkStatus = table.checkStatus(obj.config.id),
            data = checkStatus.data; //获取选中的数据
        switch (obj.event) {
            case 'add':
                {
                    ClearFormValue();
                    element.tabChange('TabBrief', 'layid_addedit');
                    if (data.length === 1) {
                        AddOrderStudyOfPatient(JSON.stringify(data[0]));
                    }
                    break;
                }
            case 'update':
                if (data.length === 0) {
                    layer.msg('请选择一行');
                } else if (data.length > 1) {
                    layer.msg('只能同时编辑一个');
                } else {
                    // layer.alert('编辑 [id]：' + data[0].id);
                    //layer.alert('编辑 [id]：' + JSON.stringify(data[0]));
                    // element.tabChange('TabBrief', 'layid_addedit');
                    element.tabChange('TabBrief', 'layid_addedit');
                    SetFormValue(JSON.stringify(data[0]));
                }
                break;
            case 'delete':
                layer.msg('权限禁止删除！');
                // if (data.length === 0) {
                //     layer.msg('请选择一行');
                // } else {
                //     layer.msg('删除');
                // }
                break;
        }
    });
    table.on('toolbar(studyimagedatas)', function (obj) {
        var checkStatus = table.checkStatus(obj.config.id),
            data = checkStatus.data; //获取选中的数据
        switch (obj.event) {
            //自定义头工具栏右侧图标 - 提示
            case 'LAYTABLE_TIPS':
                layer.alert('检查图像表,不一定在预约表中');
                break;
        }
    });

    //监听行工具事件 row event
    table.on('tool(studytabledatas)', function (obj) {
        var data = obj.data, //获得当前行数据
            layEvent = obj.event; //获得 lay-event 对应的值
        switch (layEvent) {
            case 'detail':
                {
                    //layer.msg('查看操作');  
                    layer.alert('detail： rowIndex:' + obj.tr[0].rowIndex + ' 选中数据:' + JSON.stringify(obj.data));
                    break;
                }
            case 'del ':
                {
                    layer.msg('权限禁止删除！');
                    // layer.confirm('真的删除行么', function(index) {
                    // obj.del(); //删除对应行（tr）的DOM结构
                    // layer.close(index);
                    //向服务端发送删除指令
                    break;
                }
            case 'edit':
                {
                    element.tabChange('TabBrief', 'layid_addedit');
                    //layer.alert('rowDouble rowIndex:' + obj.tr[0].rowIndex + ' 选中数据:' + JSON.stringify(data));
                    SetFormValue(JSON.stringify(obj.data));
                    //layer.msg('编辑操作');
                    break;
                }
            case 'image':
                {

                    layer.msg('image操作');
                    break;
                }
        }
    });
    // table.on('radio(studytabledatas)', function(obj) {
    //     layer.alert('---' + obj.tr.data('isChecked'));
    //     if (obj.tr.data('isChecked')) {
    //         obj.checked = false;
    //         obj.tr.find(':radio[lay-type="layTableRadio"]').prop('checked', false);
    //         layui.form.render();
    //     }
    // });
    //监听表格行点击
    // table.on('row(studytabledatas)', function(obj) {
    // if (obj.checked) {
    //     obj.checked = false;
    //     // obj.tr.find(':radio[lay-type="layTableRadio"]').prop('checked', false);
    //     layui.form.render();
    // }
    // $('input:radio[name=sex]')[0].checked = false;
    // form.render('radio');
    // console.log(obj);
    // layer.alert('row rowIndex:'+ obj.tr[0].rowIndex +' 选中数据:'+JSON.stringify(obj.data));
    // });
    //监听表格行双击
    table.on('rowDouble(studytabledatas)', function (obj) {
        //var patient = JSON.stringify(obj.data);
        var json = obj.data; // JSON.parse(patient);
        if (json.studyOrderIdentity == g_currentReportOrderIdentity) {
            element.tabChange('TabBrief', 'layid_report');
            return;
        } else if (g_currentReportOrderIdentity.length > 0) {
            if (saveReport2ServerContent() < 0) {
                layer.alert(g_mess_savereportfail);
                element.tabChange('TabBrief', 'layid_report');
                return -1;
            }
        }
        var reportdata = g_reportData; //temp_report
        reportdata.StudyOrderIdentity = json.studyOrderIdentity;
        reportdata.ReportIdentity = json.studyOrderIdentity;
        reportdata.ReportContent = ''; // to do ... next
        //set title content
        // document.getElementById("reporpatientname").innerText = "Name:" + json.patientName;
        //document.getElementById("reporpatientid").innerText = "ID:" + json.patientId;
        var postdata = JSON.stringify(reportdata);
        //获取数据库检查报告信息
        $.ajax({
            type: "POST",
            url: 'http://' + window.location.host + '/healthsystem/ris/getreportdata',
            async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
            data: postdata, //JSON.stringify(reportdata), //请求save处理数据
            // dataType: "json",
            success: function (result) {
                try {
                    if (typeof result == 'string') {
                        var jsondata;
                        jsondata = JSON.parse(result);
                        // console.log('result == string');
                        result = jsondata;
                    }
                } catch (e) {
                    console.log('result != string');
                }
                if (result.ReportContent.length > 0) {
                    reportdata = result;
                    // layer.alert('ok:' + reportdata.ReportContent);
                    element.tabChange('TabBrief', 'layid_report');
                    setStudyReportContent(reportdata);
                    return;
                } else {
                    setStudyReportContent(reportdata);
                    reportdata.ReportIdentity = "";
                    layer.alert('no report data! to create new empty report!');
                    element.tabChange('TabBrief', 'layid_report');
                }
            },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                setStudyReportContent(reportdata);
                layer.alert('Get report error! no report data!');
                element.tabChange('TabBrief', 'layid_report');
            }
        });
    });
    //上传
    // upload.render({
    //     elem: '#uploadDemo',
    //     url: '', //上传接口
    //     done: function(res) {
    //         console.log(res);
    //     }
    // });
    //studyimage
    var host = window.location.host;
    var imageview_url = "http://" + host + "/view/view.html?";
    table.on('rowDouble(studyimagedatas)', function (obj) {
        //var patient = JSON.stringify(obj.data);
        var json = obj.data; // JSON.parse(patient);
        // layer.alert(JSON.stringify(obj.data));
        window.open(imageview_url + json.studyuid);
        // layer.alert(json.studyuid);
    });
    //submit patientform
    form.on('submit(patientform)', function (data) {
        var postdata = JSON.stringify(data.field);
        var host = window.location.host;
        $.ajax({
            type: "POST",
            url: 'http://' + host + '/healthsystem/ris/updata/',
            async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
            data: postdata, //请求save处理数据
            success: function (mess) {
                if (mess == "OK") { //根据返回值进行跳转
                    // layer.alert(mess + '--save ok!' + postdata);
                    tablePatient.reload(); //重载表格
                    ChangeTab('TabBrief', 'layid_addedit');
                    layer.msg('   save ok!   ');
                    //table.render("studytabledatas");
                } else {
                    layer.alert(mess + "--save fail:" + postdata);
                }
            }
        });
        return false;
    });
    // var myDate = new Date();
    // var mytime = myDate.toLocaleTimeString();
    // layer.msg('mytime:' + mytime);
    //执行一个 table 实例
    var posturl = 'http://' + window.location.host + '/healthsystem/ris/studydata/?' + searchStudyTime;
    // console.log("posturl-" + posturl);
    var tablePatient = table.render({
        elem: '#studytable',
        height: 'full-200',
        url: posturl,
        // url: window.location.host + '/healthsystem/ris/studydata/?' + searchStudyTime, //数据接口 http://127.0.0.1:80/
        // url: window.location.host + '/healthsystem/ris/studydata/?' + 'start=' + $('#stduystart').val() + '&end=' + $('#stduyend').val(), //数据接口 http://127.0.0.1:80/
        //url: 'http://127.0.0.1/healthsystem/ris/stduydata/?start=20190101&end=20191219',
        title: 'patients',
        page: true, //开启分页
        toolbar: 'default', //开启工具栏，此处显示默认图标
        totalRow: true, //开启合计行
        loading: true,
        //skin: 'row' ,
        limits: [10, 20, 40, 60, 80, 100, 150, 200, 300],
        limit: 20,
        patientId: "testReload",
        size: 'sm',
        // type: 'radio',
        // even: true, //开启隔行背景
        cols: [
            [ //表头
                {
                    type: 'radio', //'checkbox',
                    fixed: 'left'
                }, {
                    field: 'patientId',
                    title: 'PatientId',
                    width: 100,
                    sort: true,
                    fixed: 'left',
                    event: 'setSign',
                    totalRowText: '合计：'
                }, {
                    field: 'patientName',
                    width: 120,
                    event: 'setSign',
                    title: 'PatientName'
                }, {
                    field: 'studyDate',
                    title: 'StudyDateTime',
                    sort: true,
                    event: 'setSign',
                    totalRow: false
                }, {
                    field: 'patientSex',
                    title: 'PatientSex',
                    width: 100,
                    event: 'setSign',
                    sort: false
                }, {
                    field: 'studyModality',
                    title: 'StudyModality',
                    event: 'setSign',
                    width: 100
                }, {
                    field: 'studyId',
                    title: 'StudyId',
                    width: 120,
                    sort: true,
                    hide: true,
                    event: 'setSign',
                    totalRow: false
                }, {
                    field: 'patientBirthday',
                    title: 'PatientBirthday',
                    event: 'setSign',
                    sort: true
                }, {
                    field: 'studyDescription',
                    event: 'setSign',
                    title: 'StudyDescription'
                }, {
                    field: 'scheduledDateTime',
                    title: 'ScheduledDateTime',
                    event: 'setSign',
                    sort: true,
                    totalRow: false
                }, {
                    field: 'studyuid',
                    title: 'Studyuid',
                    event: 'setSign',
                    sort: true,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'studystate',
                    title: 'Studystate',
                    event: 'setSign',
                    sort: true,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'patientIdentity',
                    title: 'PatientIdentity',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'studyOrderIdentity',
                    title: 'StudyOrderIdentity',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'patientAddr',
                    title: 'PatientAddr',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'patientEmail',
                    title: 'PatientEmail',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'patientCarID',
                    title: 'PatientCarID',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'patientTelNumber',
                    title: 'TelNumber',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'studyDepart',
                    title: 'StudyDepart',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'studyCost',
                    title: 'StudyCost',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: true
                }, {
                    field: 'costType',
                    title: 'CostType',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    fixed: 'right',
                    align: 'center',
                    toolbar: '#table_row_btns'
                }
            ]
        ]
    });
    //执行一个 table 实例
    var tableStudyImage = table.render({
        elem: '#studyimage',
        height: 'full-200',
        url: 'http://' + window.location.host + '/healthsystem/ris/stduyimage/?' + searchImageTime, //数据接口 http://127.0.0.1:80/
        //url: 'http://127.0.0.1/healthsystem/ris/stduydata/?start=20190101&end=20191219',
        title: 'studyimages',
        page: true, //开启分页
        toolbar: '#toolbarDemo', //开启头部工具栏，并为其绑定左侧模板
        defaultToolbar: ['filter', 'exports', 'print', { //自定义头部工具栏右侧图标。如无需自定义，去除该参数即可
            title: '提示',
            layEvent: 'LAYTABLE_TIPS',
            icon: 'layui-icon-tips'
        }],
        totalRow: true, //开启合计行
        loading: true,
        //skin: 'row' ,
        limits: [10, 20, 40, 60, 80, 100, 150, 200, 300],
        limit: 20,
        patientId: "testReload",
        size: 'sm',
        // type: 'radio',
        // even: true, //开启隔行背景
        cols: [
            [ //表头
                {
                    type: 'radio', //'checkbox',
                    fixed: 'left'
                }, {
                    field: 'patientId',
                    title: 'PatientId',
                    width: 100,
                    sort: true,
                    fixed: 'left',
                    event: 'setSign',
                    totalRowText: '合计：'
                }, {
                    field: 'patientName',
                    width: 120,
                    event: 'setSign',
                    title: 'PatientName'
                }, {
                    field: 'studyDate',
                    title: 'StudyDateTime',
                    width: 100,
                    sort: true,
                    event: 'setSign',
                    totalRow: false
                }, {
                    field: 'patientSex',
                    title: 'PatientSex',
                    width: 100,
                    event: 'setSign',
                    sort: false
                }, {
                    field: 'studyModality',
                    title: 'StudyModality',
                    event: 'setSign',
                    width: 100
                }, {
                    field: 'studyId',
                    title: 'StudyId',
                    width: 120,
                    sort: true,
                    hide: true,
                    event: 'setSign',
                    totalRow: false
                }, {
                    field: 'patientBirthday',
                    title: 'PatientBirthday',
                    event: 'setSign',
                    sort: true,
                    width: 100
                }, {
                    field: 'studyDescription',
                    event: 'setSign',
                    title: 'StudyDescription',
                    width: 100
                }, {
                    field: 'studyuid',
                    title: 'Studyuid',
                    event: 'setSign',
                    sort: true,
                    hide: false,
                    totalRow: false
                }, {
                    field: 'studystate',
                    title: 'Studystate',
                    event: 'setSign',
                    sort: true,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'patientIdentity',
                    title: 'PatientIdentity',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'studyOrderIdentity',
                    title: 'StudyOrderIdentity',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'studyDepart',
                    title: 'StudyDepart',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }
                , {
                    fixed: 'right',
                    width: 360,
                    align: 'center',
                    toolbar: '#toolbarDemo'
                }
            ]
        ]
    });
});