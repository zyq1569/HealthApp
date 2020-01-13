package com.dicomserver.health;
import com.dicomserver.health.config.ServerConfig;
import com.dicomserver.health.handler.HttpStaticFileServerInitializer;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.Channel;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslContextBuilder;
import io.netty.handler.ssl.SslProvider;
import io.netty.handler.ssl.util.SelfSignedCertificate;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.core.LoggerContext;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import java.io.File;
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
    private static final ServerConfig serverconfig = new ServerConfig();

    public static void main(String[] args) throws Exception {
//        Logger log = LogManager.getLogger(HealthApplication.class);
//        LoggerContext logContext = (LoggerContext) LogManager.getContext(false);
//        if (args.length > 17) {
//            File conFile = new File(args[7]);//"D:/code/C++/HealthApp/Tools/DicomDataWeb/src/main/resources/log4j2.xml");
////            File conFile = new File("D:/code/C++/HealthApp/bin/win32/log4j2.xml");
////        File conFile = new File("D:/code/C++/HealthApp/Tools/DicomDataWeb/src/main/resources/log4j2.xml");
//            logContext.setConfigLocation(conFile.toURI());
//            logContext.reconfigure();
//        }
        Logger log = LogManager.getLogger(HealthApplication.class);
        if (args.length >6 ) {
            serverconfig.bparm = true;
            serverconfig.driver = args[0];
            serverconfig.url = args[1];
            serverconfig.username = args[2];
            serverconfig.password = args[3];
            serverconfig.port = args[4];
            serverconfig.filePath = args[5];
            serverconfig.webdir = args[6];
            String port = serverconfig.getString("port");
//            System.err.println(port);
            log.info("HealthApplication port:{}", port);
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
                    .handler(new LoggingHandler(LogLevel.INFO))
                    .childHandler(new HttpStaticFileServerInitializer(sslCtx));
            Channel ch = b.bind(Integer.parseInt(serverconfig.getString("port")/*PORT*/)).sync().channel();
//			Channel ch = b.bind(PORT).sync().channel();
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