package com.dicomserver.health;

import com.dicomserver.health.config.ServerConfig;
import com.dicomserver.health.handler.HttpStaticFileServerInitializer;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelOption;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslContextBuilder;
import io.netty.handler.ssl.SslProvider;
import io.netty.handler.ssl.util.SelfSignedCertificate;
import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.core.appender.ConsoleAppender;
import org.apache.logging.log4j.core.config.Configurator;
import org.apache.logging.log4j.core.config.builder.api.*;
import org.apache.logging.log4j.core.config.builder.impl.BuiltConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;

//SpringBootBestPracticesSample  https://github.com/geekxingyun/SpringBootBestPracticesSample
//	public static void main(String[] args) {
//		SpringApplication.run(HealthApplication.class, args);
//	}
//
//https://xingyun.blog.csdn.net/article/details/100856124 Spring Boot 2.x 最佳实践之 Apache Log4j2 集成
@SpringBootApplication
public class HealthApplication {
    static final boolean SSL = System.getProperty("ssl") != null;
    //	static final int PORT = Integer.parseInt(System.getProperty("port", SSL ? "8443" : "8080"));
    public static final ServerConfig serverconfig = new ServerConfig();

    public static void SetLoggerConfig(String LogDir, String levels) {
        ConfigurationBuilder<BuiltConfiguration> builder = ConfigurationBuilderFactory.newConfigurationBuilder();
        Level level = Level.WARN;
        levels = levels.toUpperCase();
        if (levels.length()>1){
            if (levels.contains("WARN")){
                level = Level.WARN;
            }else if (levels.contains("INFO")){
                level = Level.INFO;
            }else if (levels.contains("ERROR")){
                level = Level.ERROR;
            }else if (levels.contains("FATAL")){
                level = Level.FATAL;
            }else if (levels.contains("OFF")){
                level = Level.OFF;
            }
        }
        builder.setStatusLevel(level);//Level.WARN);
        builder.setConfigurationName("RollingBuilder");
// create a console appender
        AppenderComponentBuilder appenderBuilder = builder.newAppender("Stdout", "CONSOLE").addAttribute("target",
                ConsoleAppender.Target.SYSTEM_OUT);
//        appenderBuilder.add(builder.newLayout("PatternLayout")
//                .addAttribute("pattern", "%d [%t] %-5level: %msg%n%throwable"));
        appenderBuilder.add(builder.newLayout("PatternLayout")
                .addAttribute("pattern", "%d{HH:mm:ss:SSS} %t %p %logger{36} %method %m%n"));
        builder.add(appenderBuilder);
// create a rolling file appender
        LayoutComponentBuilder layoutBuilder = builder.newLayout("PatternLayout")
                .addAttribute("pattern", "%d{yyyyMMdd HH:mm:ss:SSS} %t %p  %m%n");
        ComponentBuilder triggeringPolicy = builder.newComponent("Policies")
                .addComponent(builder.newComponent("CronTriggeringPolicy").addAttribute("schedule", "0 0 0 * * ?"))
                .addComponent(builder.newComponent("SizeBasedTriggeringPolicy").addAttribute("size", "100M"));
        appenderBuilder = builder.newAppender("rolling", "RollingFile")
                .addAttribute("fileName", LogDir + "/HealthApp.log")
                .addAttribute("filePattern", LogDir + "/logs/rolling-%d{MM-dd-yy}.log.gz")
                .add(layoutBuilder)
                .addComponent(triggeringPolicy);
        builder.add(appenderBuilder);
// create the new logger
//        过滤掉指定一些无用的INFO信息
//        builder.add(builder.newLogger("StdoutSet", Level.INFO)
//                .add(builder.newAppenderRef("Stdout")).add(builder.newAppenderRef("Stdout"))
//                .addAttribute("additivity", false));
        builder.add(builder.newRootLogger(level).add(builder.newAppenderRef("rolling")).add(builder.newAppenderRef("Stdout")));
        Configurator.initialize(builder.build());
//        LoggerContext logContext = Configurator.initialize(builder.build());
//        logContext.updateLoggers();
    }

    public static void main(String[] args) throws Exception {
        if (args.length > 7) {
            String LogDir = args[7];
            String levels="";
            if (args.length >8){
                levels = args[8];
            }
            SetLoggerConfig(LogDir,levels);
        }
        Logger log = LogManager.getLogger(HealthApplication.class);
        if (args.length > 6) {
            serverconfig.bparm = true;
            serverconfig.driver = args[0];
            serverconfig.url = args[1];
            serverconfig.username = args[2];
            serverconfig.password = args[3];
            serverconfig.port = args[4];
            serverconfig.filePath = args[5];
            serverconfig.webdir = args[6];
            String port = serverconfig.getString("port");
            log.error("HealthApplication port:{}", port);
            for (String s : args) {
                log.info("args string:{}", s);
            }
        }
        //////////////--------------------------
        final SslContext sslCtx;
        if (SSL) {
            SelfSignedCertificate ssc = new SelfSignedCertificate();
            sslCtx = SslContextBuilder.forServer(ssc.certificate(), ssc.privateKey())
                    .sslProvider(SslProvider.JDK).build();
        } else {
            sslCtx = null;
        }
        EventLoopGroup bossGroup = new NioEventLoopGroup(3);
        EventLoopGroup workerGroup = new NioEventLoopGroup();
        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .handler(new LoggingHandler())
                    .option(ChannelOption.SO_BACKLOG,1024)
                    .childHandler(new HttpStaticFileServerInitializer(sslCtx));
            Channel ch = b.bind(Integer.parseInt(serverconfig.getString("port")/*PORT*/)).sync().channel();
//			Channel ch = b.bind(PORT).sync().channel();//LogLevel.ERROR
            log.info("HealthApplication :{}", "Open your web browser and navigate to " + (SSL ? "https" : "http") + "://127.0.0.1:" + serverconfig.getString("port") + '/');
            ch.closeFuture().sync();
        } finally {
            bossGroup.shutdownGracefully();
            workerGroup.shutdownGracefully();
        }
    }
}

//        LoggerContext logContext = (LoggerContext) LogManager.getContext(false);
//        Configuration loggerConfiguration = logContext.getConfiguration();
//        String strname = loggerConfiguration.getName();
//        File conFile = new File("D:/code/C++/HealthApp/Tools/DicomDataWeb/src/main/resources/log4j2.xml");
//        logContext.setConfigLocation(conFile.toURI());
//        logContext.reconfigure();
//        Logger log = LogManager.getLogger(HealthApplication.class);

// set cofigfile log4j2.xml
//        Logger log = LogManager.getLogger(HealthApplication.class);
//        LoggerContext logContext = (LoggerContext) LogManager.getContext(false);
//        if (args.length > 7) {
//            File conFile = new File(args[7]);//"D:/code/C++/HealthApp/Tools/DicomDataWeb/src/main/resources/log4j2.xml");
////            File conFile = new File("D:/code/C++/HealthApp/bin/win32/log4j2.xml");
////            File conFile = new File("D:/code/C++/HealthApp/Tools/DicomDataWeb/src/main/resources/log4j2.xml");
//            logContext.setConfigLocation(conFile.toURI());
//            logContext.reconfigure();
//        }