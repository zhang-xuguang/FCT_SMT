#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>         // 提供访问串口的功能
#include <QtSerialPort/QSerialPortInfo>     // 提供系统中存在的串口信息

#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>



#include "dataarchiving.h"

/*
SMT-FCT需求
    1、连接三个串口，分别是主板，继电器，测试工装
    2、可识别每个串口对应的设备（连上串口先发送一条协议，
    3、界面显示连接到三种设备
    4、可单独与每个设备通讯

SMT-FCT软件步骤
    1、扫码枪扫描主板二维码
    2、上位机检测到二维码信息刷新，闭合继电器2，给主板提供3.3V电压
    3、检测到主板在线，则关闭继电器2，并打开继电器1给主板提供15.5V电压
    4、开机、闭合继电器3（大于3S），启动主板，检测到AP在线则断开继电器3
    5、上位机开始测试，人工根据测试进度依次按下功能按键
    6、检测结束，判断通过或异常
    7、打开滚刷，断开继电器1，断开主板供电
*/


/*
 *控制指令 0x01
 *  1、打开所有功能
 *  2、反馈指令F3  MCU端只留一个触发按键，AP端不变
 *  3、读取参指令添加驱动轮码盘
 *
 *控制指令0x02
 *  1、关闭所有功能
 *
*/





QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void SerialPortInit();                  //串口初始化（参数配置）
    void FCT_Check(uchar * p, int len);
    void MaincontrolDataReceived();
    void forckDataReceived();
    void relayDataReceived();
    quint16 CRC_Compute(const quint8 *puchMsg, quint16 usDataLen);      //CRC16校验
    void serialCrc_Send( QSerialPort *serial,quint8 *puchMsg, quint16 usDataLen);   //串口发送函数，计算CRC

    void connectserial();
    void JudegVersion();    //判断版本号
    void updata_UI();       //更新UI界面

    void Table_init();      //UI界面tablewidget初始化
    void Table_reset();     //UI界面复位（除了配置文件的内容）

    void ReadConfigFile();  //读取配置文件

    void oneTestresule(bool result, int row);       //将某一项的检测结果写入界面(table widget)

    void ChangeFctNum_File();        //修改配置文件中的FCT总数
    void ChangeFctPass_File();       //修改配置文件中fct的通过总数


    //全局串口对象
    QSerialPort *serial_mainboard;  //连接主板串口
    QSerialPort *serial_relay;      //连接继电器串口
    QSerialPort *serial_frock;     //连接工装串口
    QString serial_port[3];

protected:
    void keyPressEvent(QKeyEvent *event) override;
    QString scannedData ;   //保存条形码

private slots:

    void on_OpenSerialButton_clicked();     //串口开关

    void handleTimeout();                   //超时处理函数，检测主板是否开机
    void handleTimeout_1();                 //超时处理函数，发送都参指令
    void serialCheckHandleTimeout();        //将三个串口连接到正确的槽函数


    void on_fct1pushButton_clicked();
    void on_fct2pushButton_clicked();

    void on_resetpushbutton_clicked();
    void on_periphpushbutton_clicked();

    void on_updetafreTextChanged();

    void on_OpenSerialButton_2_clicked();

private:



    Ui::MainWindow *ui;
    QByteArray data;
    QByteArray frock_buff;
    QByteArray relay_buff;
    QString QR_code;
    int frame_serial = 0;
    int F1_code = 2;    //读参指令代码
    int serial_state_check = 0;
    char send_data[10] = {0};
    int flag_version = 0;
    QTimer *m_pTimer;
    QTimer *m_pTimer_1;
    QTimer *m_pTimer_2;
    QTimer *m_pTimer_3; //发送读参指令

    quint64 Fctsmt_NUM = 0;    //记录smt测试总数
    quint64 Fctsmt_pass = 0;    //记录smt成功总数

    uint8_t serial_frock_flag = 0, serial_relay_flag = 0, serial_mainboard_flag = 0;

    quint8 ReadReference[10] = {
        0x54,0x41,      //帧头
        0x00,0x02,      //正文长度
        0xF1,0x01,      //指令
        //正文内容
        0x00,0x00,      //CRC高低位
        0x4E,0x47       //截止位
    };

    quint8 Read_parameter_CHECK[10] = {
        0x54,0x41,      //帧头
        0x00,0x02,      //正文长度
        0xF1,0x00,      //指令
        //正文内容
        0x00,0x00,      //CRC高低位
        0x4E,0x47       //截止位
    };
    quint8 FCT_CHECK[11] = {
        0x54,0x41,      //帧头
        0x00,0x03,      //正文长度
        0xF2,0x01,      //指令
        0x11,//正文内容
        0x00,0x00,      //CRC高低位
        0x4E,0x47       //截止位+
    };

    quint8 UPDATE_FRE[12] ={
        0x54,0x41,      //帧头
        0x00,0x04,      //正文长度
        0xF2,0x10,      //指令
        0x00,0x00,//正文内容
        0x00,0x00,      //CRC高低位
        0x4E,0x47       //截止位+
    };

    quint8 relay_01_on[8] = {0x01,0x05,0x00,0x00,0xFF,0x00,0x8C,0x3A};
    quint8 relay_01_off[8] = {0x01,0x05,0x00,0x00,0x00,0x00,0xCD,0xCA};


    quint8 relaycontrol_all[13] = {0x01,0x0F,0x00,0x00,0x00,0x20,0x04,
                                   0x03,0x00,0x00,0x00,
                                   0xC4,0xCC};
    quint8 relayRead[8] = {0x01,0x01,0x00,0x00,0x00,0x20,0x3D,0xD2};


    quint8 frock_all[8] = {0x01,0x03,0x00,0x00,0x00,0x10
                           ,0x44,0x44};

    //串口相关
    int count_number = 0;   //主板串口接收数组下标
    quint8 head_num = 0;    //一帧中第一个数据下标
    quint8 tail_num = 0;    //一帧中末尾数据下标
    quint8 serial_head = 0;     //0，没找到头； 1，找到头。一帧数据处理完之后清零
    quint8 serial_tail = 0;
    uchar Read_data[5000] = {0};     //主板串口接收缓存区
    uchar Frock_Read_data[5000] = {0};
    QList<QSerialPortInfo> serialPortInfoList;  //保存有效端口号


    /*
      1  电池电压，16000
      3  电池电流，12000
      4  风机转速，12000
      5  滚刷电流，1800
      6  边刷电流，1800
      7  水泵电流，1500
      8  驱动轮电流-左，700
      9  驱动轮电流-右，700
      10  左下视，1000
      11  中下视，1000
      12  右下视，1000
      13  红外接收灯1，50
      14  红外接收灯2，50
      15  红外接收灯3，50
      16  红外接收灯4，50
      17  陀螺仪，——
      18  驱动轮码盘-左，——
      19  驱动轮码盘-右，——
      20  LDS撞板，——
      21  左撞板，——
      22  右撞板，——
      23  抹布支架检测（霍尔），——
      24  尘盒检测（霍尔），——
      25  WIFI模组，——
      26  LDS模组，——
      27  回冲按键，——
      28  电源按键，——
    */

    //所有功能状态存档
    uchar periph_state[40] = {0};   //1通过，2不通过
    uchar flag_err = 0;     //如果检测失败则重新检测（最多三次）
    uchar receive_succsee = 1;  //接收成功标志位
    char start_mainboard = 0;   //启动主板

    quint8 mcu_state = 0;   //MCU状态标志位
    quint8 ap_state = 0;    //AP状态标志位
    quint8 apactivatedState = 0;    //AP激活状态
    quint8 mcuactivatedState = 0;   //mcu激活状态
    uchar readmcu = 0,readfrock = 0,controlldsboard =0 ;      //是否读完一个设备，读完1，未读完0

    quint16 down_look[3] = {1850,1850,1500};     //下视（左中右）阈值
    quint16 down_look_flag[6] = {0,0,0,0,0,0};

    quint8 version_flag = 0;
    QString ap_version ;    //AP版本字符串
    QString mcu_version ;   //MCU版本字符串
    QString mcu_version_setext = "1.3.84";
    QString ap_version_setext = "1.3.66";

    QString ap_uuid;        //存储ap的uuid
    QString peripvalue = 0;     //外设lineedit显示字符串

    //电机堵转电流范围标准
    //滚刷范围,边刷，左轮，右轮电机堵转电流
    quint16 blockoff_elec[4] = {1500,700,500,500};

    //配置文件路径
    QString Configfilepath = "D:/qt-progect/FCT_SMT/FCT_SMT_1221/config.txt";

    QStringList Testing_result = {" 1"," 2"," 3"," 4","5 "," 6"," 7","8 "," 9"," 10"," 11"," 12"," 13"," 14"," 15"," 16"
                                  ,"17 "," 18"," 19","20 ","21 ","22 "," 23"," 24","25 ","26 "," 27"," 28","29 "," 30",
                                  " 31","32 "," 33","34 ","35 ","36 "
                                  };

    Dataarchiving Myxlsx;       //初始化无参构造函数


};


#endif // MAINWINDOW_H




