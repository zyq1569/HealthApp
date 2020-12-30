//表格设置
// console.log(window.location.host);
// console.log(window.location.port);
function SetFormValue(jsonstring) {
    var data = JSON.parse(jsonstring);
    //给表单赋值
    layui.form.val("formpatient", {
        "PatientName": data.PatientName,
        "PatientSex": data.PatientSex,
        "PatientBirthday": data.PatientBirthday,
        "PatientTelNumber": data.PatientTelNumber,
        "PatientEmail": data.PatientEmail,
        "PatientCarID": data.PatientCarID,
        "PatientAddr": data.PatientAddr,
        "StudyCost": data.StudyCost,
        "StudyDepart": function () {
            if (data.StudyDepart.indexOf("普放") != -1) {
                return 10
            } else if (data.StudyDepart.indexOf("核医") != -1) {
                return 11
            } else if (data.StudyDepart.indexOf("心脏") != -1) {
                return 20
            } else if (data.StudyDepart.indexOf("彩超") != -1) {
                return 21
            } else if (data.StudyDepart.indexOf("肠镜") != -1) {
                return 30
            } else if (data.StudyDepart.indexOf("胃镜") != -1) {
                return 31
            } else {
                return 10
            }
        },
        "StudyModality": data.StudyModality,
        "ScheduledDate": data.ScheduledDateTime,
        "CostType": function () {//data.CostType,
            if (data.CostType.indexOf("自费") != -1) {
                return data.CostType
            } else if (data.CostType.indexOf("公费") != -1) {
                return data.CostType
            } else if (data.CostType.indexOf("其它") != -1) {
                return data.CostType
            } else if (data.CostType.indexOf("半公费") != -1) {
                return data.CostType
            } else {
                return "自费"
            }
        },
        "StudyDescription": data.StudyDescription,
        "PatientID": data.PatientID,
        "PatientIdentity": data.PatientIdentity,
        "StudyOrderIdentity": data.StudyOrderIdentity
    });
    //layer.msg(data);
}

function AddOrderStudyOfPatient(jsonstring) {
    var data = JSON.parse(jsonstring);
    //layer.alert(data);
    //给表单赋值
    layui.form.val("formpatient", {
        "PatientName": data.PatientName,
        "PatientSex": data.PatientSex,
        "PatientBirthday": data.PatientBirthday,
        "PatientTelNumber": data.PatientTelNumber,
        "PatientEmail": data.PatientEmail,
        "PatientCarID": data.PatientCarID,
        "PatientAddr": data.PatientAddr,
        // "StudyCost": data.studyCost,
        // "StudyDepart": data.studyDepart,
        // "StudyModality": data.studyModality,
        // "ScheduledDate": data.scheduledDateTime,
        // "CostType": data.costType,
        // "StudyDescription": data.studyDescription,
        "PatientID": data.PatientID,
        "PatientIdentity": data.PatientIdentity,
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
var flag = true;
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
            //url: 'http://' + window.location.host + '/healthsystem/ris/studydata/?' + searchStudyTime//abandon
            url: 'http://' + window.location.host + '/healthsystem/ris/StudyOrder/?' + searchStudyTime
        });
        // var type = $(this).data('type');
        // active[type] ? active[type].call(this) : '';
    });
    //testWebDOF
    $('.testWebDOF .layui-btn').on('click', function () {
        window.open('http://' + window.location.host + '/login/test/testReport.html#A');
        //window.location.replace('http://' + window.location.host + '/healthsystem/ris/StudyReport.html')
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
    //-----------studyimage-------------------------------------
    var host = window.location.host;
    var imageview_url = "http://" + host + "/view/view.html?";
    //监听行工具事件 row event
    table.on('tool(studytabledatas)', function (obj) {
        var data = obj.data, //获得当前行数据
            layEvent = obj.event; //获得 lay-event 对应的值
        switch (layEvent) {
            case 'detail':
                {
                    //layer.msg('查看操作');  
                    //layer.alert('detail： rowIndex:' + obj.tr[0].rowIndex + ' 选中数据:' + JSON.stringify(obj.data));
                    var json = obj.data;
                    //var reportview_url ='http://' + window.location.host + '/login/test/testReport.html#'+json.StudyOrderIdentity;
                    if (flag) {
                        window.open('http://' + window.location.host + '/login/test/studyReport.html?' + Math.random() + '#' + json.StudyOrderIdentity);
                    } else {
                        window.open('http://' + window.location.host + '/login/test/testReport.html?' + Math.random() + '#' + json.StudyOrderIdentity);
                    }
                    flag = !flag;
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
                    //var patient = JSON.stringify(obj.data);
                    var json = obj.data; // JSON.parse(patient);
                    // layer.alert(JSON.stringify(obj.data));
                    //layer.msg(JSON.stringify(obj.data));
                    if (parseInt(json.StudyState) > 2 && parseInt(json.StudyType) == 0) {
                        //if (parseInt(json.studyState) >= 2) {
                        window.open(imageview_url + json.StudyUID);
                    }
                    else {
                        //layer.msg(json.studystate);
                        layer.msg('没有检查图像');
                    }
                    // layer.alert(json.studyuid);
                    break;
                }
        }
    });
    //监听表格行双击
    table.on('rowDouble(studytabledatas)', function (obj) {
        //var patient = JSON.stringify(obj.data);
        var json = obj.data; // JSON.parse(patient);
        //<!-- begin 20201218 add:new report -->
        if (flag) {
            window.open('http://' + window.location.host + '/login/test/studyReport.html?' + Math.random() + '#' + json.StudyOrderIdentity);
        } else {
            window.open('http://' + window.location.host + '/login/test/testReport.html?' + Math.random() + '#' + json.StudyOrderIdentity);
        }
        flag = !flag;
        //<!-- end 20201218 add:new report -->

        /// <!-- abandon 2020-1229: old report & load -->
        // if (json.StudyOrderIdentity == g_currentReportOrderIdentity) {
        //     element.tabChange('TabBrief', 'layid_report');
        //     return;
        // } else if (g_currentReportOrderIdentity.length > 0) {
        //     if (saveReport2ServerContent() < 0) {
        //         layer.alert(g_mess_savereportfail);
        //         element.tabChange('TabBrief', 'layid_report');
        //         return -1;
        //     }
        // }
        // var reportdata = g_reportData; //temp_report
        // reportdata.StudyOrderIdentity = json.StudyOrderIdentity;
        // reportdata.ReportIdentity = json.StudyOrderIdentity;
        // reportdata.ReportContent = ''; // to do ... next
        // //set title content
        // // document.getElementById("reporpatientname").innerText = "Name:" + json.patientName;
        // //document.getElementById("reporpatientid").innerText = "ID:" + json.patientId;
        // var postdata = JSON.stringify(reportdata);
        // //获取数据库检查报告信息
        // $.ajax({
        //     type: "POST",
        //     url: 'http://' + window.location.host + '/healthsystem/ris/getreportdata',
        //     async: false, //同步：意思是当有返回值以后才会进行后面的js程序。
        //     data: postdata, //JSON.stringify(reportdata), //请求save处理数据
        //     // dataType: "json",
        //     success: function (result) {
        //         try {
        //             if (typeof result == 'string') {
        //                 var jsondata;
        //                 jsondata = JSON.parse(result);
        //                 // console.log('result == string');
        //                 result = jsondata;
        //             }
        //         } catch (e) {
        //             console.log('result != string');
        //         }
        //         if (result.ReportContent.length > 0) {
        //             reportdata = result;
        //             // layer.alert('ok:' + reportdata.ReportContent);
        //             element.tabChange('TabBrief', 'layid_report');
        //             setStudyReportContent(reportdata);
        //             return;
        //         } else {
        //             setStudyReportContent(reportdata);
        //             reportdata.ReportIdentity = "";
        //             layer.alert('no report data! to create new empty report!');
        //             element.tabChange('TabBrief', 'layid_report');
        //         }
        //     },
        //     error: function (XMLHttpRequest, textStatus, errorThrown) {
        //         setStudyReportContent(reportdata);
        //         layer.alert('Get report error! no report data!');
        //         element.tabChange('TabBrief', 'layid_report');
        //     }
        // });
    });

    //submit patientform //自动加载表单所有数据 来自 属性：<form class="layui-form layui-form-pane1" action="" lay-filter="formpatient">
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

    // url: window.location.host + '/healthsystem/ris/studydata/?' + searchStudyTime, //数据接口 http://127.0.0.1:80/
    // url: window.location.host + '/healthsystem/ris/studydata/?' + 'start=' + $('#stduystart').val() + '&end=' + $('#stduyend').val(), //数据接口 http://127.0.0.1:80/
    //url: 'http://127.0.0.1/healthsystem/ris/stduydata/?start=20190101&end=20191219',
    //var posturl = 'http://' + window.location.host + '/healthsystem/ris/studydata/?' + searchStudyTime;//abandon
    var posturl = 'http://' + window.location.host + '/healthsystem/ris/StudyOrder/?' + searchStudyTime;//20201218
    // console.log("posturl-" + posturl);
    var tablePatient = table.render({
        elem: '#studytable',
        height: 'full-200',
        url: posturl,
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
                    field: 'PatientID',
                    title: 'PatientID',
                    width: 100,
                    sort: true,
                    fixed: 'left',
                    event: 'setSign',
                    totalRowText: '合计：'
                }, {
                    field: 'PatientName',
                    width: 120,
                    event: 'setSign',
                    title: 'Name'
                }, {
                    field: 'PatientSex',
                    title: 'Sex',
                    width: 60,
                    event: 'setSign',
                    sort: false
                }, {
                    field: 'StudyDateTime',
                    title: 'StudyTime',
                    sort: true,
                    event: 'setSign',
                    totalRow: false
                }, {
                    field: 'StudyModality',
                    title: 'Modality',
                    event: 'setSign',
                    width: 100
                }, {
                    field: 'StudyID',
                    title: 'StudyId',
                    width: 100,
                    sort: true,
                    hide: true,
                    event: 'setSign',
                    totalRow: false
                }, {
                    field: 'PatientBirthday',
                    title: 'Birthday',
                    width: 100,
                    event: 'setSign',
                    sort: true
                }, {
                    field: 'StudyDescription',
                    event: 'setSign',
                    title: 'Description'
                }, {
                    field: 'ScheduledDateTime',
                    title: 'ScheduledTime',
                    event: 'setSign',
                    sort: true,
                    totalRow: false
                }, {
                    field: 'StudyUID',
                    title: 'Studyuid',
                    event: 'setSign',
                    sort: true,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'StudyState',
                    title: 'State',
                    event: 'setSign',
                    sort: true,
                    hide: false,
                    totalRow: false,
                    templet: function (data) {
                        if (parseInt(data.StudyState) == 1) {
                            return '预约'
                        } else if (parseInt(data.StudyState) == 1) {
                            return '待检查'
                        } else if (parseInt(data.StudyState) == 2) {
                            return '已检查'
                        } else if (parseInt(data.StudyState) == 3) {
                            return '已诊断'
                        } else if (parseInt(data.StudyState) > 3) {
                            return '已审核'
                        }
                    }
                }, {
                    field: 'StudyType',
                    title: 'StudyType',
                    event: 'setSign',
                    sort: true,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'PatientIdentity',
                    title: 'PatientIdentity',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'StudyOrderIdentity',
                    title: 'StudyOrderIdentity',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'PatientAddr',
                    title: 'PatientAddr',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'PatientEmail',
                    title: 'PatientEmail',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'PatientCarID',
                    title: 'PatientCarID',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'PatientTelNumber',
                    title: 'TelNumber',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'StudyDepart',
                    title: 'StudyDepart',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    field: 'StudyCost',
                    title: 'StudyCost',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: true
                }, {
                    field: 'CostType',
                    title: 'CostType',
                    event: 'setSign',
                    sort: false,
                    hide: true,
                    totalRow: false
                }, {
                    fixed: 'right',
                    align: 'center',//align: 'left',
                    width: 210, ///min width210
                    toolbar: '#table_row_btns'
                }
            ]
        ]
    });
});
    ///------------20201218-------------------------
    //----------------abandon-----------------
    //执行一个 table 实例
    // var tableStudyImage = table.render({
    //     elem: '#studyimage',
    //     height: 'full-200',
    //     url: 'http://' + window.location.host + '/healthsystem/ris/stduyimage/?' + searchImageTime, //数据接口 http://127.0.0.1:80/
    //     //url: 'http://127.0.0.1/healthsystem/ris/stduydata/?start=20190101&end=20191219',
    //     title: 'studyimages',
    //     page: true, //开启分页
    //     toolbar: '#toolbarDemo', //开启头部工具栏，并为其绑定左侧模板
    //     defaultToolbar: ['filter', 'exports', 'print', { //自定义头部工具栏右侧图标。如无需自定义，去除该参数即可
    //         title: '提示',
    //         layEvent: 'LAYTABLE_TIPS',
    //         icon: 'layui-icon-tips'
    //     }],
    //     totalRow: true, //开启合计行
    //     loading: true,
    //     //skin: 'row' ,
    //     limits: [10, 20, 40, 60, 80, 100, 150, 200, 300],
    //     limit: 20,
    //     patientId: "testReload",
    //     size: 'sm',
    //     // type: 'radio',
    //     // even: true, //开启隔行背景
    //     cols: [
    //         [ //表头
    //             {
    //                 type: 'radio', //'checkbox',
    //                 fixed: 'left'
    //             }, {
    //                 field: 'patientId',
    //                 title: 'PatientId',
    //                 width: 100,
    //                 sort: true,
    //                 fixed: 'left',
    //                 event: 'setSign',
    //                 totalRowText: '合计：'
    //             }, {
    //                 field: 'patientName',
    //                 width: 120,
    //                 event: 'setSign',
    //                 title: 'PatientName'
    //             }, {
    //                 field: 'studyDate',
    //                 title: 'StudyDateTime',
    //                 width: 100,
    //                 sort: true,
    //                 event: 'setSign',
    //                 totalRow: false
    //             }, {
    //                 field: 'patientSex',
    //                 title: 'PatientSex',
    //                 width: 100,
    //                 event: 'setSign',
    //                 sort: false
    //             }, {
    //                 field: 'studyModality',
    //                 title: 'StudyModality',
    //                 event: 'setSign',
    //                 width: 100
    //             }, {
    //                 field: 'studyId',
    //                 title: 'StudyId',
    //                 width: 120,
    //                 sort: true,
    //                 hide: true,
    //                 event: 'setSign',
    //                 totalRow: false
    //             }, {
    //                 field: 'patientBirthday',
    //                 title: 'PatientBirthday',
    //                 event: 'setSign',
    //                 sort: true,
    //                 width: 100
    //             }, {
    //                 field: 'studyDescription',
    //                 event: 'setSign',
    //                 title: 'StudyDescription',
    //                 width: 100
    //             }, {
    //                 field: 'studyuid',
    //                 title: 'Studyuid',
    //                 event: 'setSign',
    //                 sort: true,
    //                 hide: false,
    //                 totalRow: false
    //             }, {
    //                 field: 'studystate',
    //                 title: 'Studystate',
    //                 event: 'setSign',
    //                 sort: true,
    //                 hide: true,
    //                 totalRow: false
    //             }, {
    //                 field: 'studytype',
    //                 title: 'StudyType',
    //                 event: 'setSign',
    //                 sort: true,
    //                 hide: true,
    //                 totalRow: false
    //             }, {
    //                 field: 'patientIdentity',
    //                 title: 'PatientIdentity',
    //                 event: 'setSign',
    //                 sort: false,
    //                 hide: true,
    //                 totalRow: false
    //             }, {
    //                 field: 'studyOrderIdentity',
    //                 title: 'StudyOrderIdentity',
    //                 event: 'setSign',
    //                 sort: false,
    //                 hide: true,
    //                 totalRow: false
    //             }, {
    //                 field: 'studyDepart',
    //                 title: 'StudyDepart',
    //                 event: 'setSign',
    //                 sort: false,
    //                 hide: true,
    //                 totalRow: false
    //             }
    //             , {
    //                 fixed: 'right',
    //                 width: 360,
    //                 align: 'center',
    //                 toolbar: '#toolbarDemo'
    //             }
    //         ]
    //     ]
    // });
    //上传
    // upload.render({
    //     elem: '#uploadDemo',
    //     url: '', //上传接口
    //     done: function(res) {
    //         console.log(res);
    //     }
    // });
    //studyimage
    // var host = window.location.host;
    // var imageview_url = "http://" + host + "/view/view.html?";
    // table.on('rowDouble(studyimagedatas)', function (obj) {
    //     //var patient = JSON.stringify(obj.data);
    //     var json = obj.data; // JSON.parse(patient);
    //     // layer.alert(JSON.stringify(obj.data));
    //     window.open(imageview_url + json.studyuid);
    //     // layer.alert(json.studyuid);
    // });
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
    // table.on('toolbar(studyimagedatas)', function (obj) {
    //     var checkStatus = table.checkStatus(obj.config.id),
    //         data = checkStatus.data; //获取选中的数据
    //     switch (obj.event) {
    //         //自定义头工具栏右侧图标 - 提示
    //         case 'LAYTABLE_TIPS':
    //             layer.alert('检查图像表,不一定在预约表中');
    //             break;
    //     }
    // });
    // $('.SerarchImageTable .layui-btn').on('click', function () {
    //     searchImageTime = 'start=' + $('#imagestduystart').val() + '&end=' + $('#imagestduyend').val();
    //     tableStudyImage.reload({
    //         url: 'http://' + window.location.host + '/healthsystem/ris/stduyimage/?' + searchImageTime
    //     });
    //     // var type = $(this).data('type');
    //     // active[type] ? active[type].call(this) : '';
    // });
