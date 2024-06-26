package com.dicomserver.health.handler;
//reference :netty demo
import com.dicomserver.health.HealthApplication;
import com.dicomserver.health.dao.StudyDataDao;
import com.dicomserver.health.dao.UserService;
import com.dicomserver.health.dao.impl.StudyDataDaoimpl;
import com.dicomserver.health.dao.impl.UserServiceimp;
import com.dicomserver.health.entity.LoginUser;
import com.dicomserver.health.entity.ReportData;
import com.dicomserver.health.entity.StudyData;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.*;
import io.netty.handler.codec.http.*;
import io.netty.handler.codec.http.cookie.ServerCookieDecoder;
import io.netty.handler.ssl.SslHandler;
import io.netty.handler.stream.ChunkedFile;
import io.netty.util.CharsetUtil;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.*;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Pattern;

import static io.netty.handler.codec.http.HttpMethod.GET;
import static io.netty.handler.codec.http.HttpMethod.POST;
import static io.netty.handler.codec.http.HttpResponseStatus.*;
import static io.netty.handler.codec.http.HttpVersion.HTTP_1_0;
import static io.netty.handler.codec.http.HttpVersion.HTTP_1_1;

/**
 * A  file handler that serves incoming HTTP requests to send their respective
 * HTTP responses.
 */
public class HttpStaticFileServerHandler extends SimpleChannelInboundHandler<FullHttpRequest> {
    public static final String HTTP_DATE_FORMAT = "EEE, dd MMM yyyy HH:mm:ss zzz";
    public static final String HTTP_DATE_GMT_TIMEZONE = "GMT";
    public static final int HTTP_CACHE_SECONDS = 60;
//    public static final String HTTP_PATH = SystemPropertyUtil.get("user.dir");
    private FullHttpRequest request;
    private static Logger log = LogManager.getLogger(HttpStaticFileServerHandler.class);

    public class OFHashValue {
        public int first = 0;
        public int second = 0;

        public OFHashValue() {
            this.first = this.second = 0;
        }

        //add :hashvalue
        public boolean CreateHashValue(final byte buffer[], final long length) {
            //素数2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
            //101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
            //181 191 193 197 199
            final int M1 = 71;
            final int M2 = 79;

            final int D1 = 197;
            final int D2 = 199;
			
			//size_t value = F + S;
			//for (size_t i = 0; i < len; i++)
			//{
			//	value = (value + str[i])*F + i*S;
			//}
	
            int value = M1 + D1;
            for (int i = 0; i < length; i++) {
                value = (value  + buffer[i] )* D1 + i*S;
            }

            this.first = value;

            value = M2 + D2;
            for (int i = 0; i < length; i++) {
                value = (value  + buffer[i] )* D2 + i*S;
            }

            this.second = value;
            return true;
        }

        public String GetHashValuePath(final byte buffer[], final long length) {
            String hashpath = "";
            if (CreateHashValue(buffer, length)) {
                hashpath = String.valueOf(this.first) + "/" + String.valueOf(this.second);
            }
            return hashpath;
        }
    }

    //add 20191113
    public static Map<String, Object> getParameter(String url) {
        Map<String, Object> map = new HashMap<String, Object>();
        try {
            final String charset = "utf-8";
            url = URLDecoder.decode(url, charset);
            if (url.indexOf('?') != -1) {
                final String contents = url.substring(url.indexOf('?') + 1);
                String[] keyValues = contents.split("&");
                for (int i = 0; i < keyValues.length; i++) {
                    String key = keyValues[i].substring(0, keyValues[i].indexOf("="));
                    String value = keyValues[i].substring(keyValues[i].indexOf("=") + 1);
                    map.put(key, value);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        System.out.println(map);
//        syste (map);
        return map;
    }

    public String getJsonPath(String studyuid) {
        OFHashValue h = new OFHashValue();
        String filePath = "";
        String hashPath = h.GetHashValuePath(studyuid.getBytes(), studyuid.length());
        if (!hashPath.equals("")) {
            filePath = hashPath + "/" + studyuid + "/" + studyuid;
        }
        return filePath;
    }

    public String getDicomPath(String studyuid, String seriesuid, String sopinstanceuid) {
        OFHashValue h = new OFHashValue();
        String filePath = "";
        String hashPath = h.GetHashValuePath(studyuid.getBytes(), studyuid.length());
        if (!hashPath.equals("")) {
            filePath = hashPath + "/" + studyuid + "/" + seriesuid + "/" + sopinstanceuid;
            if (sopinstanceuid.indexOf(".dcm") < 1) {
                filePath += ".dcm";
            }
        }
        return filePath;
    }

    public String getImageTableData(String startTime, String endTime, String page, String limit) {
        String strvalue;
        StudyDataDao studydata = new StudyDataDaoimpl();
        List<StudyData> list = studydata.getAllStudyImageData(startTime, endTime, page, limit);
        int size = studydata.getAllStudyImageCount(startTime, endTime, page, limit);//list.size();
        if (list.size() > size) {
            size = list.size();
        }
        JsonObject respjson = new JsonObject();
        JsonArray rarray = new JsonArray();
        for (StudyData stu : list) {
            JsonObject robject = new JsonObject();
            robject.addProperty("patientId", stu.getPatientID());
            robject.addProperty("patientName", stu.getPatientName());
            robject.addProperty("studyDate", stu.getStudyDateTime());
            robject.addProperty("patientSex", stu.getPatientSex());
            robject.addProperty("studyModality", stu.getStudyModality());
            robject.addProperty("studyId", stu.getStudyID());
            robject.addProperty("patientBirthday", stu.getPatientBirthday());
            robject.addProperty("studyDescription", stu.getStudyDescription());
            robject.addProperty("scheduledDateTime", stu.getScheduledDateTime());
            robject.addProperty("studyuid", stu.getStudyUID());
            robject.addProperty("patientIdentity", stu.getPatientIdentity());
            robject.addProperty("studyDescription", stu.getStudyDescription());
            robject.addProperty("studyOrderIdentity", stu.getStudyOrderIdentity());
            rarray.add(robject);
        }
        respjson.addProperty("code", 0);
        respjson.addProperty("msg", "");
        respjson.addProperty("count", size);
        respjson.add("data", rarray);
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        //log.info ( gson.toJson(respjson));
        strvalue = gson.toJson(respjson);
        return strvalue;
    }

    public String getStudysImageData() {
        String strvalue;
        StudyDataDao studydata = new StudyDataDaoimpl();
//        String startTime = null, endTime= null,  page= null,  limint= null;
        List<StudyData> list = studydata.getAllStudyImageData(null, null, null, null);
        int size = list.size();
        JsonObject respjson = new JsonObject();
        JsonArray rarray = new JsonArray();
        for (StudyData stu : list) {
            JsonObject robject = new JsonObject();
            robject.addProperty("patientName", stu.getPatientName());
            robject.addProperty("patientId", stu.getPatientID());
            robject.addProperty("studyDate", stu.getStudyDateTime());
            robject.addProperty("modality", stu.getStudyModality());
            robject.addProperty("studyDescription", stu.getStudyDescription());
            robject.addProperty("numImages", 1);
            robject.addProperty("studyId", stu.getStudyID());
            robject.addProperty("studyuid", stu.getStudyUID());
            rarray.add(robject);
        }
        respjson.add("studyList", rarray);
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        log.info (gson.toJson(respjson));
        strvalue = gson.toJson(respjson);
        return strvalue;
    }

    public String getPatientStudyData(String startTime, String endTime, String page, String limit) {
        String strvalue;
        StudyDataDao studydata = new StudyDataDaoimpl();
        List<StudyData> list = studydata.getAllStudy(startTime, endTime, page, limit);
        int size = studydata.getAllStudyCount(startTime, endTime, page, limit);//list.size();
        if (list.size() > size) {
            size = list.size();
        }
        JsonObject respjson = new JsonObject();
        JsonArray rarray = new JsonArray();
        for (StudyData stu : list) {
            JsonObject robject = new JsonObject();
            robject.addProperty("patientId", stu.getPatientID());
            robject.addProperty("patientName", stu.getPatientName());
            robject.addProperty("studyDate", stu.getStudyDateTime());
            robject.addProperty("patientSex", stu.getPatientSex());
            robject.addProperty("studyModality", stu.getStudyModality());
            robject.addProperty("studyId", stu.getStudyID());
            robject.addProperty("patientBirthday", stu.getPatientBirthday());
            robject.addProperty("studyDescription", stu.getStudyDescription());
            robject.addProperty("scheduledDateTime", stu.getScheduledDateTime());
            robject.addProperty("studyuid", stu.getStudyUID());
            if (stu.getStudyState().equals("1")) {
                robject.addProperty("studystate", "已预约");
            } else if (stu.getStudyState().equals("2")) {
                robject.addProperty("studystate", "检查中");
            } else if (stu.getStudyState().equals("3")) {
                robject.addProperty("studystate", "已检查");
            } else if (stu.getStudyState().equals("4")) {
                robject.addProperty("studystate", "诊断");
            } else if (stu.getStudyState().equals("5")) {
                robject.addProperty("studystate", "已审核");
            }
            robject.addProperty("patientIdentity", stu.getPatientIdentity());
            robject.addProperty("patientEmail", stu.getPatientEmail());
            robject.addProperty("patientAddr", stu.getPatientAddr());
            robject.addProperty("patientCarID", stu.getPatientCarID());
            robject.addProperty("patientTelNumber", stu.getPatientTelNumber());
            robject.addProperty("studyDepart", stu.getStudyDepart());
            robject.addProperty("studyCost", stu.getStudyCost());
            robject.addProperty("costType", stu.getCostType());
            robject.addProperty("studyDescription", stu.getStudyDescription());
            robject.addProperty("studyOrderIdentity", stu.getStudyOrderIdentity());
            rarray.add(robject);
        }
        respjson.addProperty("code", 0);
        respjson.addProperty("msg", "");
        respjson.addProperty("count", size);
        respjson.add("data", rarray);
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        //log.info ( gson.toJson(respjson));
        strvalue = gson.toJson(respjson);
        return strvalue;
    }

    public String getStudyReport(ReportData reportData) {
        String strvalue = "";
        StudyDataDao addReport = new StudyDataDaoimpl();
        int result = addReport.getStudyReport(reportData);
        if (result > 0) {
            Gson gson = new GsonBuilder().setPrettyPrinting().create();
            strvalue = gson.toJson(reportData, ReportData.class);
//            log.info ("------StudyReport:--"+strvalue);
        } else if (result == 0) {
            Gson gson = new GsonBuilder().setPrettyPrinting().create();
            strvalue = gson.toJson(reportData, ReportData.class);
//            log.info ("------StudyReport is empty:--" + strvalue);
            log.warn("------StudyReport is empty:--", strvalue);
            return strvalue;
        }
        return strvalue;
    }

    public String readToString(String fileName) {
        String encoding = "UTF-8";
        File file = new File(fileName);
        Long filelength = file.length();
        byte[] filecontent = new byte[filelength.intValue()];
        try {
            FileInputStream in = new FileInputStream(file);
            in.read(filecontent);
            in.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            return new String(filecontent, encoding);
        } catch (UnsupportedEncodingException e) {
//            log.error("The OS does not support " + encoding);
            log.error("The OS does not support ", encoding);
            e.printStackTrace();
            return null;
        }
    }

    public void returnWeb(ChannelHandlerContext ctx, FullHttpRequest request, String path, boolean keepAlive) throws Exception {
        // to do :Optimize the code
        if (!path.contains(".css")) {
            int pos = path.indexOf("?v=");
            if (pos > 0) {
                path = path.substring(0, pos);
                SendFileData(ctx, request, path, path, keepAlive, true);
                return;
            }
        }
        int pos = path.indexOf("?v=");
        if (pos > 0) {
            path = path.substring(0, pos);
        }
        File file = new File(path);
        Long filelength = file.length();
        byte[] filecontent = new byte[filelength.intValue()];
        try {
            FileInputStream in = new FileInputStream(file);
            in.read(filecontent);
            in.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        ByteBuf buffer = ctx.alloc().buffer(Math.toIntExact(filelength));
        buffer.writeBytes(filecontent);
        FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
        response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
        ///
        if (path.endsWith(".html")) {
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/html; charset=UTF-8");
        } else if (path.endsWith(".js")) {
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/x-javascript");
        } else if (path.endsWith(".css")) {
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/css");//text/css
        } else if (path.endsWith(".woff")) {
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/woff");
        } else if (path.endsWith(".woff2")) {
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/woff2");
        } else if (path.endsWith(".jpg")) {
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/x-jpg");
        }
        this.sendAndCleanupConnection(ctx, response);
    }

    public void SendFileData(ChannelHandlerContext ctx, FullHttpRequest request, String uri, String path, boolean keepAlive, boolean bSetFilename) throws Exception {
        File file = new File(path);
        if (file.isHidden() || !file.exists()) {
            this.sendError(ctx, NOT_FOUND);
            log.info("not found:",path);
            return;
        }

        if (file.isDirectory()) {
            if (uri.endsWith("/")) {
                this.sendListing(ctx, file, uri);
            } else {
                this.sendRedirect(ctx, uri + '/');
            }
            return;
        }

        if (!file.isFile()) {
            sendError(ctx, FORBIDDEN);
            return;
        }

        // Cache Validation
        String ifModifiedSince = request.headers().get(HttpHeaderNames.IF_MODIFIED_SINCE);
        if (ifModifiedSince != null && !ifModifiedSince.isEmpty()) {
            SimpleDateFormat dateFormatter = new SimpleDateFormat(HTTP_DATE_FORMAT, Locale.US);
            Date ifModifiedSinceDate = dateFormatter.parse(ifModifiedSince);

            // Only compare up to the second because the datetime format we send to the client
            // does not have milliseconds
            long ifModifiedSinceDateSeconds = ifModifiedSinceDate.getTime() / 1000;
            long fileLastModifiedSeconds = file.lastModified() / 1000;
            if (ifModifiedSinceDateSeconds == fileLastModifiedSeconds) {
                FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, NOT_MODIFIED, Unpooled.EMPTY_BUFFER);
                if (bSetFilename) { //根据web协议请求的dicom文件设置文件名
                    String filename = file.getName();
                    response.headers().set(HttpHeaderNames.CONTENT_DISPOSITION, "attachment;filename=" + URLEncoder.encode(filename, "UTF-8"));
                }
                this.sendNotModified(ctx, response);
                return;
            }
        }

        RandomAccessFile raf;
        try {
            raf = new RandomAccessFile(file, "r");
        } catch (FileNotFoundException ignore) {
            sendError(ctx, NOT_FOUND);
            return;
        }
        long fileLength = raf.length();

        HttpResponse response = new DefaultHttpResponse(HTTP_1_1, OK);
        HttpUtil.setContentLength(response, fileLength);
        setContentTypeHeader(response, file);
        setDateAndCacheHeaders(response, file);

        response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
        if (!keepAlive) {
            response.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.CLOSE);
        } else if (request.protocolVersion().equals(HTTP_1_0)) {
            response.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.KEEP_ALIVE);
        }

        // Write the initial line and the header.
        ctx.write(response);

        // Write the content.
        ChannelFuture sendFileFuture;
        ChannelFuture lastContentFuture;
        if (ctx.pipeline().get(SslHandler.class) == null) {
            sendFileFuture = ctx.write(new DefaultFileRegion(raf.getChannel(), 0, fileLength), ctx.newProgressivePromise());
            // Write the end marker.
            lastContentFuture = ctx.writeAndFlush(LastHttpContent.EMPTY_LAST_CONTENT);
        } else {
            sendFileFuture =
                    ctx.writeAndFlush(new HttpChunkedInput(new ChunkedFile(raf, 0, fileLength, 8192)),
                            ctx.newProgressivePromise());
            // HttpChunkedInput will write the end marker (LastHttpContent) for us.
            lastContentFuture = sendFileFuture;
        }

        sendFileFuture.addListener(new ChannelProgressiveFutureListener() {
            @Override
            public void operationProgressed(ChannelProgressiveFuture future, long progress, long total) {
                if (total < 0) { // total unknown
                    log.info(future.channel() + " Transfer progress: " + progress);
                } else {
                    log.error(future.channel() + " Transfer progress: " + progress + " / " + total);
                }
            }

            @Override
            public void operationComplete(ChannelProgressiveFuture future) {
                log.info(future.channel() + " Transfer complete.");
            }
        });

        // Decide whether to close the connection or not.
        if (!keepAlive) {
            // Close the connection when the whole content is written out.
            lastContentFuture.addListener(ChannelFutureListener.CLOSE);
        }
    }

    public void SavePatient(ChannelHandlerContext ctx, FullHttpRequest request, String uri, boolean keepAlive) throws Exception {
        String path = uri;
        log.info (path);
        // to do ... change use json
        String content = request.content().toString(CharsetUtil.UTF_8);
        Gson gson = new Gson();
        StudyData stu = gson.fromJson(content, StudyData.class);
        // to do save&update patient data
        stu.setPatientType("0");
        StudyDataDao addStudy = new StudyDataDaoimpl();
        if (addStudy.addPatient(stu) > 0) {
            if (addStudy.addOrderStudy(stu) > 0) {
                String message = "OK";
                FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, Unpooled.wrappedBuffer(message.getBytes("UTF-8")));
                response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/plain");
                this.sendAndCleanupConnection(ctx, response);
                return;
            }
        }

        String message = "fail";
        FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, Unpooled.wrappedBuffer(message.getBytes("UTF-8")));
        response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");
        response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/plain");
        this.sendAndCleanupConnection(ctx, response);
    }

    public void LoginHealth(ChannelHandlerContext ctx, FullHttpRequest request, String uri, boolean keepAlive) throws Exception {
        String path = uri;
        log.info (path);
        int login = 0;
        LoginUser user = new LoginUser();
        if (path.toLowerCase().contains("checkuser")) {
            Map<String, Object> map = getParameter("?" + request.content().toString(CharsetUtil.UTF_8));
            String userName = "";
            String password = "";
            for (String key : map.keySet()) {
                String value = (String) map.get(key);
                if (key.equals("user")) {
                    userName = value;
                } else if (key.equals("password")) {
                    password = value;
                }
            }
            UserService uerService = new UserServiceimp();
            user.setUsername(userName);
            user.setPassword(password);
            if (uerService.findUser(user)) {//存在这个用户，可以正常访问信息
                request.headers().set(HttpHeaderNames.SET_COOKIE, user);
                login = 1;
                path = HealthApplication.serverconfig.getString("webdir") + "/view/imageview.html";
            } else {//不存在这个用户，给出提示
                String message = "用户名或密码错误";
                request.headers().set(HttpHeaderNames.SET_COOKIE, message);
                login = 2;
                path = HealthApplication.serverconfig.getString("webdir") + "/Login/index.html";
            }

            if (login == 1) {
                io.netty.handler.codec.http.cookie.DefaultCookie cookie = new io.netty.handler.codec.http.cookie.DefaultCookie("user", user.getUsername());
                cookie.setPath("/");
                cookie.setHttpOnly(true);
                cookie.setSecure(false);
                String message = "OK";
                FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, Unpooled.wrappedBuffer(message.getBytes("UTF-8")));
                response.headers().add(HttpHeaderNames.SET_COOKIE, io.netty.handler.codec.http.cookie.ServerCookieEncoder.LAX.encode(cookie));
                response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/plain");
                this.sendAndCleanupConnection(ctx, response);
            } else {
                String message = "用户名或密码错误!";
                io.netty.handler.codec.http.cookie.DefaultCookie cookie = new io.netty.handler.codec.http.cookie.DefaultCookie("user", message);
                cookie.setPath("/");
                //cookie.setHttpOnly(true);
                cookie.setSecure(false);
                FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, Unpooled.wrappedBuffer(message.getBytes("UTF-8")));
                response.headers().add(HttpHeaderNames.SET_COOKIE, cookie);
                response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/plain");
                this.sendAndCleanupConnection(ctx, response);
            }

        } else {
            StringBuilder buf = new StringBuilder();
            buf.append(readToString(path));
            ByteBuf buffer = ctx.alloc().buffer(buf.length());
            buffer.writeCharSequence(buf.toString(), CharsetUtil.UTF_8);
            FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
            response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
            if (path.endsWith(".html")) {
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/html; charset=UTF-8");//.txt	text/plain
            } else if (path.endsWith(".js")) {
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/x-javascript");
            } else if (path.endsWith(".css")) {
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/css; charset=UTF-8");
            } else if (path.endsWith(".jpg")) {
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/x-jpg");
            }
            this.sendAndCleanupConnection(ctx, response);
        }
    }

    // to do ... route [different]
    public void HealthSystem(ChannelHandlerContext ctx, FullHttpRequest request, String uri, boolean keepAlive) throws Exception {
        String lowerUri = uri.toLowerCase();
        if (lowerUri.contains("/healthsystem/ris/updata")) {
            SavePatient(ctx, request, uri, keepAlive);
            return;
        }
        if (lowerUri.contains("/healthsystem/ris/studydata")) {
            Map<String, Object> map = getParameter(lowerUri);
            String startTime = "";
            String endTime = "";
            String page = "";
            String limit = "";
            for (String key : map.keySet()) {
                String value = (String) map.get(key);
                if (key.contains("start")) {
                    startTime = value;
                } else if (key.contains("end")) {
                    endTime = value;
                } else if (key.contains("page")) {
                    page = value;
                } else if (key.contains("limit")) {
                    limit = value;
                }
            }
            //get patientstudy data json
            String buf = getPatientStudyData(startTime, endTime, page, limit);
            ByteBuf buffer = ctx.alloc().buffer(buf.length());
            buffer.writeCharSequence(buf, CharsetUtil.UTF_8);
            FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
            response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/json;charset=UTF-8;");
            this.sendAndCleanupConnection(ctx, response);
            return;
        }
        if (lowerUri.contains("/healthsystem/ris/stduyimage")) {
            //get patientstudy data json
            Map<String, Object> map = getParameter(lowerUri);
            String startTime = "";
            String endTime = "";
            String page = "";
            String limit = "";
            for (String key : map.keySet()) {
                String value = (String) map.get(key);
                if (key.contains("start")) {
                    startTime = value;
                } else if (key.contains("end")) {
                    endTime = value;
                } else if (key.contains("page")) {
                    page = value;
                } else if (key.contains("limit")) {
                    limit = value;
                }
            }
            String buf = getImageTableData(startTime, endTime, page, limit);
            ByteBuf buffer = ctx.alloc().buffer(buf.length());
            buffer.writeCharSequence(buf, CharsetUtil.UTF_8);
            FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
            response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/json;charset=UTF-8;");
            this.sendAndCleanupConnection(ctx, response);
            return;
        }
        if (lowerUri.contains("/healthsystem/ris/savereportdata")) {
            String content = request.content().toString(CharsetUtil.UTF_8);
            log.info("---content:" + content);
            Gson gson = new Gson();
            ReportData reportData = gson.fromJson(content, ReportData.class);
            log.debug("---fromJson ReportContent:--" + reportData.getReportContent());
            StudyDataDao addReport = new StudyDataDaoimpl();
            if (addReport.addStudyReport(reportData) > 0) {
                String message = "OK";
                FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, Unpooled.wrappedBuffer(message.getBytes("UTF-8")));
                response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");
                response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/plain");
                this.sendAndCleanupConnection(ctx, response);
                return;
            }
            sendError(ctx, METHOD_NOT_ALLOWED);
            return;
        }
        if (lowerUri.contains("/healthsystem/ris/getreportdata")) {
            String content = request.content().toString(CharsetUtil.UTF_8);
            Gson gson = new Gson();
            ReportData reportData = gson.fromJson(content, ReportData.class);
//            ReportData reportData = gson.fromJson(request.content().toString(CharsetUtil.UTF_8),ReportData.class);
            String buf = getStudyReport(reportData);
            if (buf.equals("")) {
                sendError(ctx, METHOD_NOT_ALLOWED);
                return;
            }
            ByteBuf buffer = ctx.alloc().buffer(buf.length());
            buffer.writeCharSequence(buf, CharsetUtil.UTF_8);
            FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
            response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/json;charset=UTF-8;");
            this.sendAndCleanupConnection(ctx, response);
            return;
        }
        if (!lowerUri.contains("/stduydata/")) {
            int pos = uri.indexOf(".html?");
            if (pos > 0) {
                uri = uri.substring(0, pos + 5);
            }
            String path = HealthApplication.serverconfig.getString("webdir") + uri;
            returnWeb(ctx, request, path, keepAlive);
            return;
        }
    }

    public boolean CheckCookieLogin(FullHttpRequest request) {
        String cookiestr = request.headers().get(HttpHeaderNames.COOKIE);
//        log.info ("------COOKIE str:" + cookiestr);
        if (cookiestr != null) {
            Set<io.netty.handler.codec.http.cookie.Cookie> cookies = ServerCookieDecoder.STRICT.decode(cookiestr);
            if (!cookies.isEmpty()) {
                for (io.netty.handler.codec.http.cookie.Cookie cookie : cookies) {
                    if (cookie != null) {
//                        log.info ("---cookie.name()---" + cookie.name());
//                        log.info ("---cookie.value()---" + cookie.value());
//                        log.info ("---cookie.path()---" + cookie.path());
                        if (cookie.name().equals("user")) {
                            // todo login check
                            if (cookie.value() != "") {
                                log.info ("---cookie.value()---" + cookie.value());
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    @Override
    public void channelRead0(ChannelHandlerContext ctx, FullHttpRequest request) throws Exception {
        this.request = request;
        boolean bSetFilename = false;
//        log.info ("---content:"+ request.content().toString(CharsetUtil.UTF_8));
//        log.info ("---request:" + request);
        if (!request.decoderResult().isSuccess()) {
            sendError(ctx, BAD_REQUEST);
            return;
        }
        if (!GET.equals(request.method()) && !POST.equals(request.method())) {
            this.sendError(ctx, METHOD_NOT_ALLOWED);
            return;
        }
        final boolean keepAlive = HttpUtil.isKeepAlive(request);
        final String uri = request.uri();
        log.info ("channelRead0----uri:" + uri);
        String path = sanitizeUri(uri);//        final String path = sanitizeUri(uri);
        if (uri.toLowerCase().contains("/healthsystem")) {
            if (!CheckCookieLogin(request)) {
                String newuri = "http://" + request.headers().get(HttpHeaderNames.HOST) + "/login/login.html";
                sendRedirect(ctx, newuri);
                return;
            }
            HealthSystem(ctx, request, uri, keepAlive);
            return;
        }
        if (uri.toUpperCase().contains("/LOGIN")) {
            path = HealthApplication.serverconfig.getString("webdir") + File.separator + uri;
            if (path.contains(".jpg")) {
                SendFileData(ctx, request, uri, path, keepAlive, true);
                return;
            }
            LoginHealth(ctx, request, path, keepAlive);
            return;
        }
        if (path == null) {
            boolean bstuid = false;
            boolean bseuid = false;
            boolean bsouid = false;
            Map<String, Object> map = getParameter(uri);
            String studyuid = "";
            String seruid = "";
            String sopinstanceuid = "";
            for (String key : map.keySet()) {
                String value = (String) map.get(key);
                if (key.equals("studyuid")) {
                    studyuid = value;
                    bstuid = true;
                } else if (key.equals("seriesuid")) {
                    seruid = value;
                    bseuid = true;
                } else if (key.equals("sopinstanceuid")) {
                    sopinstanceuid = value;
                    bsouid = true;
                }
                log.info (key + ":" + value);
            }
            if (bstuid && bseuid && bsouid) {
                //根据请求参数查找请求的dicom
                String filePath = HealthApplication.serverconfig.getString("filePath");
                path = filePath + File.separator + getDicomPath(studyuid, seruid, sopinstanceuid);
                bSetFilename = true;
            } else if (bstuid) {
                String filePath = HealthApplication.serverconfig.getString("filePath");
                path = filePath + File.separator + getJsonPath(studyuid);
                //GetJsonData(path);
                path = path + ".json";
                bSetFilename = true;
            }
            if (path == null) {
                this.sendError(ctx, FORBIDDEN);
                return;
            }
        }

        //return all study data info
        if (path.contains("studyList.json")) {
            String buf = getStudysImageData();
            ByteBuf buffer = ctx.alloc().buffer(buf.length());
            buffer.writeCharSequence(buf, CharsetUtil.UTF_8);
            FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
            response.headers().set(HttpHeaderNames.ACCESS_CONTROL_ALLOW_ORIGIN, "*");//@@@@@@@@@测试使用允许同个域客户端访问
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/json;charset=UTF-8;");
            this.sendAndCleanupConnection(ctx, response);
            return;
        }
        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        if (uri.toLowerCase().contains("/view")) {
            if (!CheckCookieLogin(request)) {
                //String webhost = request.headers().get(HttpHeaderNames.REFERER);
                String newuri = "http://" + request.headers().get(HttpHeaderNames.HOST) + "/login/login.html";
                sendRedirect(ctx, newuri);
                return;
            }
//            if (uri.toLowerCase().contains("/imageview.html")) {
//            }
            if (uri.contains(".html") || uri.contains(".js") || uri.contains(".css")) {
                String htmlurl = uri.toLowerCase();
                int pos = htmlurl.lastIndexOf(".html?");
                if (pos > 1) {
                    htmlurl = htmlurl.substring(0, pos + 5);
                }
//                String htmlname = HealthApplication.serverconfig.getString("webdir") + htmlurl;
                path = HealthApplication.serverconfig.getString("webdir") + htmlurl;
                returnWeb(ctx, request, path, keepAlive);
                return;
            }
            if (path.contains(".woff") || uri.contains(".ttf") || uri.contains(".otf") || uri.contains(".eot") || uri.contains(".map")) {
                String htmlname = HealthApplication.serverconfig.getString("webdir") + uri;
                path = htmlname;
                int pos = path.indexOf("?v=");
                if (pos > -1) {
                    path = path.substring(0, pos);
                }
            }
        }
        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        if (path.contains("favicon.ico")) {
//            this.sendError(ctx, NOT_FOUND);
//            log.info ("not found:" + path);
//            return;
            path = HealthApplication.serverconfig.getString("webdir") + "/favicon.ico";
            SendFileData(ctx, request, uri, path, keepAlive, true);
            return;
        }
        SendFileData(ctx, request, uri, path, keepAlive, bSetFilename);
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        cause.printStackTrace();
        if (ctx.channel().isActive()) {
            sendError(ctx, INTERNAL_SERVER_ERROR);
        }
    }

    private static final Pattern INSECURE_URI = Pattern.compile(".*[<>&\"].*");

    // test : //http://127.0.0.1:8080/WADO?studyuid=1.3.51.0.7.633918642.633920010109.6339100821&
    // seriesuid=1.3.51.5145.15142.20010109.1105627&
    // sopinstanceuid=1.3.51.5145.5142.20010109.1105627.1.0.1
    private static String sanitizeUri(String uri) {
        // Decode the path.
        if (uri.indexOf("WADO?studyuid=") > -1) {
            return null;
        }
        try {
            uri = URLDecoder.decode(uri, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new Error(e);
        }

        if (uri.isEmpty() || uri.charAt(0) != '/') {
            return null;
        }

        // Convert file separators.
        uri = uri.replace('/', File.separatorChar);

        // Simplistic dumb security check.
        // You will have to do something serious in the production environment.
        if (uri.contains(File.separator + '.') ||
                uri.contains('.' + File.separator) ||
                uri.charAt(0) == '.' || uri.charAt(uri.length() - 1) == '.' ||
                INSECURE_URI.matcher(uri).matches()) {
            return null;
        }

        // Convert to absolute path.
        String filePath = HealthApplication.serverconfig.getString("filePath");
        ;
        return filePath + File.separator + uri;
//        return SystemPropertyUtil.get("user.dir") + File.separator + uri;
    }

    private static final Pattern ALLOWED_FILE_NAME = Pattern.compile("[^-\\._]?[^<>&\\\"]*");

    private void sendListing(ChannelHandlerContext ctx, File dir, String dirPath) {
        StringBuilder buf = new StringBuilder()
                .append("<!DOCTYPE html>\r\n")
                .append("<html><head><meta charset='utf-8' /><title>")
                .append("Listing of: ")
                .append(dirPath)
                .append("</title></head><body>\r\n")

                .append("<h3>Listing of: ")
                .append(dirPath)
                .append("</h3>\r\n")

                .append("<ul>")
                .append("<li><a href=\"../\">..</a></li>\r\n");

        for (File f : dir.listFiles()) {
            if (f.isHidden() || !f.canRead()) {
                continue;
            }

            String name = f.getName();
            if (!ALLOWED_FILE_NAME.matcher(name).matches()) {
                continue;
            }

            buf.append("<li><a href=\"")
                    .append(name)
                    .append("\">")
                    .append(name)
                    .append("</a></li>\r\n");
        }

        buf.append("</ul></body></html>\r\n");

        ByteBuf buffer = ctx.alloc().buffer(buf.length());
        buffer.writeCharSequence(buf.toString(), CharsetUtil.UTF_8);

        FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, OK, buffer);
        response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/html; charset=UTF-8");

        this.sendAndCleanupConnection(ctx, response);
    }

    private void sendRedirect(ChannelHandlerContext ctx, String newUri) {
        FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, FOUND, Unpooled.EMPTY_BUFFER);
        response.headers().set(HttpHeaderNames.LOCATION, newUri);

        this.sendAndCleanupConnection(ctx, response);
    }

    private void sendError(ChannelHandlerContext ctx, HttpResponseStatus status) {
        FullHttpResponse response = new DefaultFullHttpResponse(
                HTTP_1_1, status, Unpooled.copiedBuffer("Failure: " + status + "\r\n", CharsetUtil.UTF_8));
        response.headers().set(HttpHeaderNames.CONTENT_TYPE, "text/plain; charset=UTF-8");

        this.sendAndCleanupConnection(ctx, response);
    }

    /**
     * When file timestamp is the same as what the browser is sending up, send a "304 Not Modified"
     *
     * @param ctx Context
     */
    private void sendNotModified(ChannelHandlerContext ctx, FullHttpResponse response) {
        //FullHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, NOT_MODIFIED, Unpooled.EMPTY_BUFFER);
        setDateHeader(response);

        this.sendAndCleanupConnection(ctx, response);
    }

    /**
     * If Keep-Alive is disabled, attaches "Connection: close" header to the response
     * and closes the connection after the response being sent.
     */
    private void sendAndCleanupConnection(ChannelHandlerContext ctx, FullHttpResponse response) {
        final FullHttpRequest request = this.request;
        final boolean keepAlive = HttpUtil.isKeepAlive(request);
        HttpUtil.setContentLength(response, response.content().readableBytes());
        if (!keepAlive) {
            // We're going to close the connection as soon as the response is sent,
            // so we should also make it clear for the client.
            response.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.CLOSE);
        } else if (request.protocolVersion().equals(HTTP_1_0)) {
            response.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.KEEP_ALIVE);
        }

        ChannelFuture flushPromise = ctx.writeAndFlush(response);

        if (!keepAlive) {
            // Close the connection as soon as the response is sent.
            flushPromise.addListener(ChannelFutureListener.CLOSE);
        }
    }

    /**
     * Sets the Date header for the HTTP response
     *
     * @param response HTTP response
     */
    private static void setDateHeader(FullHttpResponse response) {
        SimpleDateFormat dateFormatter = new SimpleDateFormat(HTTP_DATE_FORMAT, Locale.US);
        dateFormatter.setTimeZone(TimeZone.getTimeZone(HTTP_DATE_GMT_TIMEZONE));

        Calendar time = new GregorianCalendar();
        response.headers().set(HttpHeaderNames.DATE, dateFormatter.format(time.getTime()));
    }

    /**
     * Sets the Date and Cache headers for the HTTP Response
     *
     * @param response    HTTP response
     * @param fileToCache file to extract content type
     */
    private static void setDateAndCacheHeaders(HttpResponse response, File fileToCache) {
        SimpleDateFormat dateFormatter = new SimpleDateFormat(HTTP_DATE_FORMAT, Locale.US);
        dateFormatter.setTimeZone(TimeZone.getTimeZone(HTTP_DATE_GMT_TIMEZONE));

        // Date header
        Calendar time = new GregorianCalendar();
        response.headers().set(HttpHeaderNames.DATE, dateFormatter.format(time.getTime()));

        // Add cache headers
        time.add(Calendar.SECOND, HTTP_CACHE_SECONDS);
        response.headers().set(HttpHeaderNames.EXPIRES, dateFormatter.format(time.getTime()));
        response.headers().set(HttpHeaderNames.CACHE_CONTROL, "private, max-age=" + HTTP_CACHE_SECONDS);
        response.headers().set(
                HttpHeaderNames.LAST_MODIFIED, dateFormatter.format(new Date(fileToCache.lastModified())));
    }

    /**
     * Sets the content type header for the HTTP Response
     *
     * @param response HTTP response
     * @param file     file to extract content type
     */
    private static void setContentTypeHeader(HttpResponse response, File file) {
//        MimetypesFileTypeMap mimeTypesMap = new MimetypesFileTypeMap();//http://tool.oschina.net/commons
        response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/octet-stream");// ""mimeTypesMap.getContentType(file.getPath())"");
    }
}

//
//文件扩展名	Content-Type(Mime-Type)	文件扩展名	Content-Type(Mime-Type)
//.*（ 二进制流，不知道下载文件类型）	application/octet-stream	.tif	image/tiff
//.001	application/x-001	.301	application/x-301
//.323	text/h323	.906	application/x-906
//.907	drawing/907	.a11	application/x-a11
//.acp	audio/x-mei-aac	.ai	application/postscript
//.aif	audio/aiff	.aifc	audio/aiff
//.aiff	audio/aiff	.anv	application/x-anv
//.asa	text/asa	.asf	video/x-ms-asf
//.asp	text/asp	.asx	video/x-ms-asf
//.au	audio/basic	.avi	video/avi
//.awf	application/vnd.adobe.workflow	.biz	text/xml
//.bmp	application/x-bmp	.bot	application/x-bot
//.c4t	application/x-c4t	.c90	application/x-c90
//.cal	application/x-cals	.cat	application/vnd.ms-pki.seccat
//.cdf	application/x-netcdf	.cdr	application/x-cdr
//.cel	application/x-cel	.cer	application/x-x509-ca-cert
//.cg4	application/x-g4	.cgm	application/x-cgm
//.cit	application/x-cit	.class	java/*
//.cml	text/xml	.cmp	application/x-cmp
//.cmx	application/x-cmx	.cot	application/x-cot
//.crl	application/pkix-crl	.crt	application/x-x509-ca-cert
//.csi	application/x-csi	.css	text/css
//.cut	application/x-cut	.dbf	application/x-dbf
//.dbm	application/x-dbm	.dbx	application/x-dbx
//.dcd	text/xml	.dcx	application/x-dcx
//.der	application/x-x509-ca-cert	.dgn	application/x-dgn
//.dib	application/x-dib	.dll	application/x-msdownload
//.doc	application/msword	.dot	application/msword
//.drw	application/x-drw	.dtd	text/xml
//.dwf	Model/vnd.dwf	.dwf	application/x-dwf
//.dwg	application/x-dwg	.dxb	application/x-dxb
//.dxf	application/x-dxf	.edn	application/vnd.adobe.edn
//.emf	application/x-emf	.eml	message/rfc822
//.ent	text/xml	.epi	application/x-epi
//.eps	application/x-ps	.eps	application/postscript
//.etd	application/x-ebx	.exe	application/x-msdownload
//.fax	image/fax	.fdf	application/vnd.fdf
//.fif	application/fractals	.fo	text/xml
//.frm	application/x-frm	.g4	application/x-g4
//.gbr	application/x-gbr	.	application/x-
//.gif	image/gif	.gl2	application/x-gl2
//.gp4	application/x-gp4	.hgl	application/x-hgl
//.hmr	application/x-hmr	.hpg	application/x-hpgl
//.hpl	application/x-hpl	.hqx	application/mac-binhex40
//.hrf	application/x-hrf	.hta	application/hta
//.htc	text/x-component	.htm	text/html
//.html	text/html	.htt	text/webviewhtml
//.htx	text/html	.icb	application/x-icb
//.ico	image/x-icon	.ico	application/x-ico
//.iff	application/x-iff	.ig4	application/x-g4
//.igs	application/x-igs	.iii	application/x-iphone
//.img	application/x-img	.ins	application/x-internet-signup
//.isp	application/x-internet-signup	.IVF	video/x-ivf
//.java	java/*	.jfif	image/jpeg
//.jpe	image/jpeg	.jpe	application/x-jpe
//.jpeg	image/jpeg	.jpg	image/jpeg
//.jpg	application/x-jpg
// .js	application/x-javascript
//.jsp	text/html	.la1	audio/x-liquid-file
//.lar	application/x-laplayer-reg	.latex	application/x-latex
//.lavs	audio/x-liquid-secure	.lbm	application/x-lbm
//.lmsff	audio/x-la-lms	.ls	application/x-javascript
//.ltr	application/x-ltr	.m1v	video/x-mpeg
//.m2v	video/x-mpeg	.m3u	audio/mpegurl
//.m4e	video/mpeg4	.mac	application/x-mac
//.man	application/x-troff-man	.math	text/xml
//.mdb	application/msaccess	.mdb	application/x-mdb
//.mfp	application/x-shockwave-flash	.mht	message/rfc822
//.mhtml	message/rfc822	.mi	application/x-mi
//.mid	audio/mid	.midi	audio/mid
//.mil	application/x-mil	.mml	text/xml
//.mnd	audio/x-musicnet-download	.mns	audio/x-musicnet-stream
//.mocha	application/x-javascript	.movie	video/x-sgi-movie
//.mp1	audio/mp1	.mp2	audio/mp2
//.mp2v	video/mpeg	.mp3	audio/mp3
//.mp4	video/mpeg4	.mpa	video/x-mpg
//.mpd	application/vnd.ms-project	.mpe	video/x-mpeg
//.mpeg	video/mpg	.mpg	video/mpg
//.mpga	audio/rn-mpeg	.mpp	application/vnd.ms-project
//.mps	video/x-mpeg	.mpt	application/vnd.ms-project
//.mpv	video/mpg	.mpv2	video/mpeg
//.mpw	application/vnd.ms-project	.mpx	application/vnd.ms-project
//.mtx	text/xml	.mxp	application/x-mmxp
//.net	image/pnetvue	.nrf	application/x-nrf
//.nws	message/rfc822	.odc	text/x-ms-odc
//.out	application/x-out	.p10	application/pkcs10
//.p12	application/x-pkcs12	.p7b	application/x-pkcs7-certificates
//.p7c	application/pkcs7-mime	.p7m	application/pkcs7-mime
//.p7r	application/x-pkcs7-certreqresp	.p7s	application/pkcs7-signature
//.pc5	application/x-pc5	.pci	application/x-pci
//.pcl	application/x-pcl	.pcx	application/x-pcx
//.pdf	application/pdf	.pdf	application/pdf
//.pdx	application/vnd.adobe.pdx	.pfx	application/x-pkcs12
//.pgl	application/x-pgl	.pic	application/x-pic
//.pko	application/vnd.ms-pki.pko	.pl	application/x-perl
//.plg	text/html	.pls	audio/scpls
//.plt	application/x-plt	.png	image/png
//.png	application/x-png	.pot	application/vnd.ms-powerpoint
//.ppa	application/vnd.ms-powerpoint	.ppm	application/x-ppm
//.pps	application/vnd.ms-powerpoint	.ppt	application/vnd.ms-powerpoint
//.ppt	application/x-ppt	.pr	application/x-pr
//.prf	application/pics-rules	.prn	application/x-prn
//.prt	application/x-prt	.ps	application/x-ps
//.ps	application/postscript	.ptn	application/x-ptn
//.pwz	application/vnd.ms-powerpoint	.r3t	text/vnd.rn-realtext3d
//.ra	audio/vnd.rn-realaudio	.ram	audio/x-pn-realaudio
//.ras	application/x-ras	.rat	application/rat-file
//.rdf	text/xml	.rec	application/vnd.rn-recording
//.red	application/x-red	.rgb	application/x-rgb
//.rjs	application/vnd.rn-realsystem-rjs	.rjt	application/vnd.rn-realsystem-rjt
//.rlc	application/x-rlc	.rle	application/x-rle
//.rm	application/vnd.rn-realmedia	.rmf	application/vnd.adobe.rmf
//.rmi	audio/mid	.rmj	application/vnd.rn-realsystem-rmj
//.rmm	audio/x-pn-realaudio	.rmp	application/vnd.rn-rn_music_package
//.rms	application/vnd.rn-realmedia-secure	.rmvb	application/vnd.rn-realmedia-vbr
//.rmx	application/vnd.rn-realsystem-rmx	.rnx	application/vnd.rn-realplayer
//.rp	image/vnd.rn-realpix	.rpm	audio/x-pn-realaudio-plugin
//.rsml	application/vnd.rn-rsml	.rt	text/vnd.rn-realtext
//.rtf	application/msword	.rtf	application/x-rtf
//.rv	video/vnd.rn-realvideo	.sam	application/x-sam
//.sat	application/x-sat	.sdp	application/sdp
//.sdw	application/x-sdw	.sit	application/x-stuffit
//.slb	application/x-slb	.sld	application/x-sld
//.slk	drawing/x-slk	.smi	application/smil
//.smil	application/smil	.smk	application/x-smk
//.snd	audio/basic	.sol	text/plain
//.sor	text/plain	.spc	application/x-pkcs7-certificates
//.spl	application/futuresplash	.spp	text/xml
//.ssm	application/streamingmedia	.sst	application/vnd.ms-pki.certstore
//.stl	application/vnd.ms-pki.stl	.stm	text/html
//.sty	application/x-sty	.svg	text/xml
//.swf	application/x-shockwave-flash	.tdf	application/x-tdf
//.tg4	application/x-tg4	.tga	application/x-tga
//.tif	image/tiff	.tif	application/x-tif
//.tiff	image/tiff	.tld	text/xml
//.top	drawing/x-top	.torrent	application/x-bittorrent
//.tsd	text/xml	.txt	text/plain
//.uin	application/x-icq	.uls	text/iuls
//.vcf	text/x-vcard	.vda	application/x-vda
//.vdx	application/vnd.visio	.vml	text/xml
//.vpg	application/x-vpeg005	.vsd	application/vnd.visio
//.vsd	application/x-vsd	.vss	application/vnd.visio
//.vst	application/vnd.visio	.vst	application/x-vst
//.vsw	application/vnd.visio	.vsx	application/vnd.visio
//.vtx	application/vnd.visio	.vxml	text/xml
//.wav	audio/wav	.wax	audio/x-ms-wax
//.wb1	application/x-wb1	.wb2	application/x-wb2
//.wb3	application/x-wb3	.wbmp	image/vnd.wap.wbmp
//.wiz	application/msword	.wk3	application/x-wk3
//.wk4	application/x-wk4	.wkq	application/x-wkq
//.wks	application/x-wks	.wm	video/x-ms-wm
//.wma	audio/x-ms-wma	.wmd	application/x-ms-wmd
//.wmf	application/x-wmf	.wml	text/vnd.wap.wml
//.wmv	video/x-ms-wmv	.wmx	video/x-ms-wmx
//.wmz	application/x-ms-wmz	.wp6	application/x-wp6
//.wpd	application/x-wpd	.wpg	application/x-wpg
//.wpl	application/vnd.ms-wpl	.wq1	application/x-wq1
//.wr1	application/x-wr1	.wri	application/x-wri
//.wrk	application/x-wrk	.ws	application/x-ws
//.ws2	application/x-ws	.wsc	text/scriptlet
//.wsdl	text/xml	.wvx	video/x-ms-wvx
//.xdp	application/vnd.adobe.xdp	.xdr	text/xml
//.xfd	application/vnd.adobe.xfd	.xfdf	application/vnd.adobe.xfdf
//.xhtml	text/html	.xls	application/vnd.ms-excel
//.xls	application/x-xls	.xlw	application/x-xlw
//.xml	text/xml	.xpl	audio/scpls
//.xq	text/xml	.xql	text/xml
//.xquery	text/xml	.xsd	text/xml
//.xsl	text/xml	.xslt	text/xml
//.xwd	application/x-xwd	.x_b	application/x-x_b
//.sis	application/vnd.symbian.install	.sisx	application/vnd.symbian.install
//.x_t	application/x-x_t	.ipa	application/vnd.iphone
//.apk	application/vnd.android.package-archive	.xap	application/x-silverlight-app