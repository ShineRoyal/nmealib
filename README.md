# nmealib

## 1、介绍

这是一个NMEA Library在RT-Thread上的移植。

### 1.1 目录结构



| 名称 | 说明 |
| ---- | ---- |
| docs  | 文档目录 |
| examples | 例子目录，并有相应的一些说明 |
| include  | 头文件目录 |
| src  | 源代码目录 |


### 1.2 许可证



nmealib 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 3.0+

## 2、如何打开 nmealib


使用 nmealib package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    IoT - internet of things --->
        [*] nmealib: A NMEA Library for RT-Thread
            [*] Enable nmealib uart sample
            (uart3) uart name e.g. uart3
            (9600)  uart baudrate used by sample
            [*] Enable nmealib trace and error
                Version(v1.0.0)
```
`Enable nmealib uart sample`用于使能串口接收GPS数据并解析的示例，`Enable nmealib trace and error`用于使能DEBUG调试信息。

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 nmealib

在打开 nmealib package 后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。

### 3.1 创建解码所需要的变量
```
nmeaINFO info;          //GPS解码后得到的信息
nmeaPARSER parser;      //解码时使用的数据结构
```
### 3.2 初始化nmeaINFO和nmeaPARSER结构体
```
nmea_zero_INFO(&info);
nmea_parser_init(&parser);
```
### 3.3 数据解析
```
nmea_parse(&parser, (const char *) &data_buffer[0], data_length, &info);
```
`data_buffer`保存GPS模块发送过来的NMEA协议字符串，`data_length`保存字符串的长度，如果接收的字符串内容不完整，将保存至`parser`中，待接收完一个完整的NMEA字符串后，把解析的内容存入`info`中。

## 4、注意事项

nmealib库仅支持`GPGGA`、`GPGSA`、`GPGSV`、`GPRMC`、`GPVTG`语句的解析。
某些支持多卫星GPS模块，其输出可能是混合输出，此时无法完成正确解析。

## 5、联系方式 & 感谢

* 维护：ShineRoyal
* 主页：https://github.com/ShineRoyal/nmealib
* nmea官网：http://nmea.sourceforge.net/