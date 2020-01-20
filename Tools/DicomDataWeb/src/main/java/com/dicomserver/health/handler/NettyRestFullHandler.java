package com.dicomserver.health.handler;

import com.dicomserver.health.HealthApplication;
import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class NettyRestFullHandler  extends ChannelDuplexHandler {
    private static Logger log = LogManager.getLogger(NettyRestFullHandler.class);
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        try {
            String body = (String) msg;
            log.info ("msg:"+ body);
/*            if (body.substring(0, 1) != new String(0x0b)) {
            	return;
            }*/
            //return;

        } catch (Exception e) {
        }
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) {
        ctx.flush();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        ctx.close();
    }
    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        log.info("客户端已断开连接: {}", ctx.channel().remoteAddress());
        super.channelInactive(ctx);
        disconnect(ctx);
    }
    private void disconnect(ChannelHandlerContext ctx) {
        if (ctx != null && ctx.channel().isActive()) {
            ctx.channel().close();
            ctx.close();
        }
    }
}
