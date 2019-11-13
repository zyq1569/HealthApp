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
import org.springframework.boot.autoconfigure.SpringBootApplication;

import java.io.FileInputStream;
import java.util.Properties;

//
//
//	public static void main(String[] args) {
//		SpringApplication.run(HealthApplication.class, args);
//	}
//
@SpringBootApplication
public class HealthApplication {

	static final boolean SSL = System.getProperty("ssl") != null;
	static final int PORT = Integer.parseInt(System.getProperty("port", SSL ? "8443" : "8080"));



	public static void main(String[] args) throws Exception {
		ServerConfig config =  new ServerConfig();
//		String PROPERTIES_NAME = System.getProperty("user.dir")+"/src/main/resources/application.properties";
//		FileInputStream in = null;
//		String config_port = "";
//		try {
//			Properties properties = new Properties();
//			in = new FileInputStream(PROPERTIES_NAME);
//			properties.load(in);
//			config_port = properties.getProperty("netty.dicom.port");
//			System.out.println("读取配置信息成功！config_port:" + config_port);
//		} finally {
//			if (in != null) {
//				try {
//					in.close();
//				} catch (Exception e) {
//					e.printStackTrace();
//				}
//			}
//		}
		// Configure SSL.
		final SslContext sslCtx;
		if (SSL) {
			SelfSignedCertificate ssc = new SelfSignedCertificate();
			sslCtx = SslContextBuilder.forServer(ssc.certificate(), ssc.privateKey())
					.sslProvider(SslProvider.JDK).build();
		} else {
			sslCtx = null;
		}

		EventLoopGroup bossGroup = new NioEventLoopGroup(2);
		EventLoopGroup workerGroup = new NioEventLoopGroup();
		try {
			ServerBootstrap b = new ServerBootstrap();
			b.group(bossGroup, workerGroup)
					.channel(NioServerSocketChannel.class)
					.handler(new LoggingHandler(LogLevel.INFO))
					.childHandler(new HttpStaticFileServerInitializer(sslCtx));

			Channel ch = b.bind(config.GetPort()/*PORT*/).sync().channel();
//			Channel ch = b.bind(PORT).sync().channel();

			System.err.println("Open your web browser and navigate to " +
					(SSL ? "https" : "http") + "://127.0.0.1:" + config.StrPort() + '/');

			ch.closeFuture().sync();
		} finally {
			bossGroup.shutdownGracefully();
			workerGroup.shutdownGracefully();
		}
	}
}
