/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-08     Shine       the first version 
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>
#include "stdio.h"
#include "string.h"

#define DBG_TAG "nmea"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "nmea/nmea.h"

/* 串口接收消息结构*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

static rt_device_t serial;
static struct rt_messagequeue rx_mq;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
    if (result == -RT_EFULL)
    {
        /* 消息队列满 */
        rt_kprintf("message queue full！\n");
    }
    return result;
}

//#define __GPS_DEBUG
/**
 * @brief  trace 在解码时输出捕获的GPS语句
 * @param  str: 要输出的字符串，str_size:数据长度
 * @retval 无
 */
#ifdef NMEALIB_DEBUG
static void trace(const char *str, int str_size)
{
    rt_kprintf("\nnmea trace:");
    for (int i = 0; i < str_size; i++)
        rt_kprintf("%c", str[i]);
    rt_kprintf("\n");
}
#endif

/**
 * @brief  error 在解码出错时输出提示消息
 * @param  str: 要输出的字符串，str_size:数据长度
 * @retval 无
 */
#ifdef NMEALIB_DEBUG
static void error(const char *str, int str_size)
{

    rt_kprintf("\nnmea error:");
    for (int i = 0; i < str_size; i++)
        rt_kprintf("%c", str[i]);
    rt_kprintf("\n");
}
#endif

static void nmea_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

    char ss[128];   //打印字符串buffer

    double deg_lat; //转换成[degree].[degree]格式的纬度
    double deg_lon; //转换成[degree].[degree]格式的经度

    nmeaINFO info;          //GPS解码后得到的信息
    nmeaPARSER parser;      //解码时使用的数据结构
#ifdef NMEALIB_DEBUG
    nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;
#endif
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            /* 从串口读取数据*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rx_buffer[rx_length] = '\0';

            nmea_parse(&parser, (const char *) &rx_buffer[0], rx_length, &info);

            //info.lat lon中的格式为[degree][min].[sec/60]，使用以下函数转换成[degree].[degree]格式
            deg_lat = nmea_ndeg2degree(info.lat);
            deg_lon = nmea_ndeg2degree(info.lon);

            LOG_D("utc_time:%d-%02d-%02d,%d:%d:%d ", info.utc.year + 1900, info.utc.mon + 1, info.utc.day,
                    info.utc.hour, info.utc.min, info.utc.sec);
            //因为LOG_D不支持浮点数，所以此处使用snprintf进行打印，再用LOG_D输出
            snprintf(ss, 128, "wd:%f,jd:%f", deg_lat, deg_lon);
            LOG_D(ss);
            snprintf(ss, 128, "high:%f m", info.elv);
            LOG_D(ss);
            snprintf(ss, 128, "v:%f km/h", info.speed);
            LOG_D(ss);
            snprintf(ss, 128, "hangxiang:%f du", info.direction);
            LOG_D(ss);
            snprintf(ss, 128, "used GPS:%d,show GPS:%d", info.satinfo.inuse, info.satinfo.inview);
            LOG_D(ss);
            snprintf(ss, 128, "PDOP:%f,HDOP:%f,VDOP:%f", info.PDOP, info.HDOP, info.VDOP);
            LOG_D(ss);

        }
    }
}

static int nmea_thread_init(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    static char msg_pool[256];
    static char up_msg_pool[256];
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    /* 查找串口设备 */
    serial = rt_device_find(NMEALIB_UART_PORT);
    /* 初始化消息队列 */
    rt_mq_init(&rx_mq, "rx_mq", msg_pool, sizeof(struct rx_msg), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
    //修改波特率为9600
    config.baud_rate = NMEALIB_UART_BAUDRATE;
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX); /* 以 DMA 接收及轮询发送方式打开串口设备 */
    rt_device_set_rx_indicate(serial, uart_input); /* 设置接收回调函数 */

    rt_thread_t thread = rt_thread_create("nmea", nmea_thread_entry, RT_NULL, 4096, 25, 10); /* 创建 serial 线程 */

    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(nmea_thread_init, nmea thread init);
INIT_APP_EXPORT(nmea_thread_init);
