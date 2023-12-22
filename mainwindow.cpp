#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <synchapi.h>
#include <QThread>

#include <QCoreApplication>
#include <QMainWindow>
#include <QSerialPort>
#include <QThread>
#include <QTableWidget>

#include <QString>
#include <QDebug>
#include <iostream>

#include <QSlider>
#include <QScrollBar>
#include <QFile>

#include <QCoreApplication>
#include <QInputMethod>
#include <QObject>
#include <QDebug>


#include <QCoreApplication>
#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

#include "dataarchiving.h"

#define TIMER_TIMEOUT   (500)
#define TIMER_TIMEOUT_1   (300)


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("SMT-FCT上位机                                       非淡泊无以明志，非宁静无以致远");

    serial_mainboard = new QSerialPort;       //申请内存，并设置父对象
    serial_frock = new QSerialPort;
    serial_relay = new QSerialPort;

    // 设置窗口属性，使其接受键盘事件
    setFocusPolicy(Qt::StrongFocus);

    ui->updatefrelineEdit->setText("500");

//    this->setMinimumSize(1350, 700);
//    this->setMaximumSize(1350, 700);

    //定时器1  检测主板是否在线
    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));

    //定时器2，讲所有要读的参数，遍历一遍
    m_pTimer_1 = new QTimer(this);
    connect(m_pTimer_1, SIGNAL(timeout()), this, SLOT(handleTimeout_1()));

//    m_pTimer_3 = new QTimer(this);
//    connect(m_pTimer_3, SIGNAL(timeout()), this, SLOT(serialCheckHandleTimeout()));

    //连接串口
    m_pTimer_2 = new QTimer(this);
    connect(m_pTimer_2, SIGNAL(timeout()), this, SLOT(serialCheckHandleTimeout()));
    m_pTimer_2->start(100);

    connect(ui->updatefrelineEdit, &QLineEdit::returnPressed, this, &MainWindow::on_updetafreTextChanged);
    ui->updatefrelineEdit->setMaxLength(4);

    ui->mculable->setFrameShape (QFrame::Box);
    ui->mculable->setStyleSheet("border-width: 1px;border-style: solid;border-color: rgb(0, 0, 0);");
    ui->aplable->setFrameShape (QFrame::Box);
    ui->aplable->setStyleSheet("border-width: 1px;border-style: solid;border-color: rgb(0, 0, 0);");

    ui->serialcontrolboardlable->setFrameShape (QFrame::Box);
    ui->serialfrocklable->setFrameShape (QFrame::Box);
    ui->serialrelaylable->setFrameShape (QFrame::Box);

    //设置版本号
    ui->mcuversionlineEdit_2->setText(mcu_version_setext);
    ui->apversionlineEdit_2->setText(ap_version_setext);
    //按键初始化
//    ui->fct1pushButton->setEnabled(false);
//    ui->fct2pushButton->setEnabled(false);
//    ui->periphpushbutton->setEnabled(false);
    //表格初始化
    Table_init();

    //读取配置文件
    ReadConfigFile();

    qApp->installEventFilter(this);

    Myxlsx.MyxlsxWrite_UUID("13616816816516516568468498496813513846841651089478978994651",10,true);
    Myxlsx.Myxlsx_config(Configfilepath);
    Myxlsx.MyxlsxWrite_parameter(Testing_result,3,Configfilepath);
    Myxlsx.MyxlsxWrite_parameter(Testing_result,5,Configfilepath);
    Myxlsx.Myxlsx_save();

}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 捕获键盘事件
    scannedData += event->text();

    if(event->text() == "\r")
    {
        if(scannedData != ui->shapcodelineedit->text()) //检测到扫码枪内容更新
        {
            //闭合继电器2，提供3.3V电压
            relaycontrol_all[7] = 0x02;
            serialCrc_Send(serial_relay,relaycontrol_all,13);
            start_mainboard = 7;    //设置定时器倒计时时间

        }
        // 在LineEdit中显示扫描到的数据
        ui->shapcodelineedit->setText(scannedData);
        scannedData.clear();
    }

}


//定时器连接三个串口（按先后顺序）
void MainWindow::serialCheckHandleTimeout()
{

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        info.portName();
    }

    //启动软件的时候获取可用的串口信息
    serialPortInfoList = QSerialPortInfo::availablePorts();         //每次进定时器刷新串口信息

    int i = 0;
    if (!serialPortInfoList.isEmpty()) {
        for(QList<QSerialPortInfo>::iterator it = serialPortInfoList.begin();
             it != serialPortInfoList.end(); ++it)
        {
            ++i;
            const QSerialPortInfo &serialPortInfo = *it;

            QString portName = serialPortInfo.portName();


            if(!serial_frock->isOpen())
            {
                serial_port[0] = portName;
                serial_frock->setPort(*it);                      // 在对象中设置串口
                if(serial_frock->open(QIODevice::ReadWrite))      // 以读写方式打开串口
                {
                    //ui->PortBox->addItem(info.portName());       // 添加计算机中的端口
                    //serial_frock->close();                   // 关闭
                    serial_frock->setReadBufferSize(1024);

                    // 参数配置
                    serial_frock->setBaudRate(QSerialPort::Baud9600);
                    // 校验，校验默认选择无
                    serial_frock->setParity(QSerialPort::NoParity);
                    // 数据位，数据位默认选择8位
                    serial_frock->setDataBits(QSerialPort::Data8);
                    // 停止位，停止位默认选择1位
                    serial_frock->setStopBits(QSerialPort::OneStop);
                    // 控制流，默认选择无
                    serial_frock->setFlowControl(QSerialPort::NoFlowControl);

                    //m_pTimer->start(TIMER_TIMEOUT);
                    connect(serial_frock,&QSerialPort::readyRead,this,&MainWindow::forckDataReceived);      // 接收数据
                    ui->serialfrocklable->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");

                    qDebug() << "串口1打开成功";
                }
                else
                {
                    ui->serialfrocklable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
                    qDebug() << "串口1打开失败，请重试";
                }
            }


            if(!serial_relay->isOpen())
            {
                if(serial_port[0] != portName)
                {
                    serial_port[1] = portName;
                    serial_relay->setPort(*it);                      // 在对象中设置串口
                    if(serial_relay->open(QIODevice::ReadWrite))      // 以读写方式打开串口
                    {
                        //ui->PortBox->addItem(info.portName());       // 添加计算机中的端口
                        //serial_device->close();                       // 关闭

                        serial_relay->setReadBufferSize(1024);

                        // 参数配置
                        serial_relay->setBaudRate(QSerialPort::Baud9600);
                        // 校验，校验默认选择无
                        serial_relay->setParity(QSerialPort::NoParity);
                        // 数据位，数据位默认选择8位
                        serial_relay->setDataBits(QSerialPort::Data8);
                        // 停止位，停止位默认选择1位
                        serial_relay->setStopBits(QSerialPort::OneStop);
                        // 控制流，默认选择无
                        serial_relay->setFlowControl(QSerialPort::NoFlowControl);

                        connect(serial_relay,&QSerialPort::readyRead,this,&MainWindow::relayDataReceived);      // 接收数据
                        ui->serialrelaylable->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");
                        qDebug() << "串口2打开成功";

                    }
                    else
                    {
                        ui->serialrelaylable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
                        qDebug() << "串口2打开失败，请重试";
                    }
                }
            }


            if( portName != serial_port[0] && portName != serial_port[1]  )
            {

                serial_mainboard->setPort(*it);
                if(!serial_mainboard->isOpen())
                {                    // 在对象中设置串口
                    if(serial_mainboard->open(QIODevice::ReadWrite))      // 以读写方式打开串口
                    {
                        //ui->PortBox->addItem(info.portName());       // 添加计算机中的端口
                        //serial_mainboard->close();                       // 关闭

                        serial_mainboard->setReadBufferSize(2048);

                        // 参数配置
                        serial_mainboard->setBaudRate(QSerialPort::Baud115200);
                        // 校验，校验默认选择无
                        serial_mainboard->setParity(QSerialPort::NoParity);
                        // 数据位，数据位默认选择8位
                        serial_mainboard->setDataBits(QSerialPort::Data8);
                        // 停止位，停止位默认选择1位
                        serial_mainboard->setStopBits(QSerialPort::OneStop);
                        // 控制流，默认选择无
                        serial_mainboard->setFlowControl(QSerialPort::NoFlowControl);

                        connect(serial_mainboard,&QSerialPort::readyRead,this,&MainWindow::MaincontrolDataReceived);      // 接收数据

                        qDebug() << "串口3打开成功";
                        ui->serialcontrolboardlable->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");
                        m_pTimer->start(TIMER_TIMEOUT);

                        m_pTimer_2->stop();
                    }
                    else
                    {
                        ui->serialcontrolboardlable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
                        qDebug() << "串口3打开失败，请重试";
                    }
                }
            }

        }

        return ;
    }

}


void MainWindow::handleTimeout()        //检测MCU和AP状态，并检测所有功能状态
{
    //MCU在线状态
    ReadReference[5] = 0x01;
    serialCrc_Send(serial_mainboard,ReadReference,10);

    //AP在线状态
    ReadReference[5] = 0x02;
    serialCrc_Send(serial_mainboard,ReadReference,10);

    //MCU激活状态
    ReadReference[5] = 0x63;
    serialCrc_Send(serial_mainboard,ReadReference,10);

    //AP激活状态
    ReadReference[5] = 0x6B;
    serialCrc_Send(serial_mainboard,ReadReference,10);

    qDebug() << "handleTimeout" ;

    //判断所有检测项目是否通过
    int flag = 1;
    for(int i = 1; i <= 34; i++)
    {
        if( i != 21 && i != 22 )
        {
            flag &= periph_state[i];
        }
    }

    if(flag == 1)      //所有检测通过
    {
        ui->alltestslable->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");
        ui->alltestslable->setText("PASS");
        FCT_CHECK[6] = 0x13;

        serialCrc_Send(serial_mainboard,FCT_CHECK,11);
        serialCrc_Send(serial_mainboard,FCT_CHECK,11);
        serialCrc_Send(serial_mainboard,FCT_CHECK,11);

        FCT_CHECK[6] = 0x12;
        serialCrc_Send(serial_mainboard,FCT_CHECK,11);
    }
    else
    {
        ui->alltestslable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
    }

    if(flag_version == 0)
    {
        Read_parameter_CHECK[5] = 0x62;
        serialCrc_Send(serial_mainboard,Read_parameter_CHECK,10);        //检测版本

        ReadReference[5] = 0x68;
        serialCrc_Send(serial_mainboard,ReadReference,10);       //读取UUID
        //更新参数
        flag_version = 1;
    }

    if(mcu_state == 1 && start_mainboard == 7)
    {
        //断开3.3V供电,并开启主板15.5V电压
        relaycontrol_all[7] = 0x01;
        serialCrc_Send(serial_relay,relaycontrol_all,13);
        mcu_state = 0;

    }


    if(start_mainboard == 6)
    {
        qDebug() << "352";
        relaycontrol_all[7] |= 0x04;    //闭合继电器3,启动机器
        serialCrc_Send(serial_relay,relaycontrol_all,13);
    }

    if(start_mainboard > 0)
        start_mainboard--;

    if(mcu_state == 1 && start_mainboard == 0 && relaycontrol_all[7] != 0x01)
    {
        qDebug() << "361";
        relaycontrol_all[7] = 0x01;    //断开继电器3，启动成功
        serialCrc_Send(serial_relay,relaycontrol_all,13);

    }

    //判断扫地机的MCU和AP是否在线
    if(ap_state == 1 && mcu_state == 1)
    {
        if(flag_version == 0)
        {
            Read_parameter_CHECK[5] = 0x62;
            serialCrc_Send(serial_mainboard,Read_parameter_CHECK,10);        //检测版本

            ReadReference[5] = 0x68;
            serialCrc_Send(serial_mainboard,ReadReference,10);       //读取UUID
            //更新参数
            flag_version = 1;
        }

        ui->fct1pushButton->setEnabled(true);
        ui->fct2pushButton->setEnabled(true);
        ui->periphpushbutton->setEnabled(true);
    }
    else
    {
        if(mcu_state == 0)
        {
            ui->mculable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
            ui->mculable->setText("MCU离线");
        }
        if(ap_state == 0)
        {
            ui->aplable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
            ui->aplable->setText("AP离线");
        }

//        ui->fct1pushButton->setEnabled(false);
//        ui->fct2pushButton->setEnabled(false);
//        ui->periphpushbutton->setEnabled(false);
    }


    //判断串口是否连上机器(定时器持续检测)
    if(serial_state_check == 0)
    {
        //串口断开
        mcu_state = 0;
        ui->mculable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
        ui->mculable->setText("MCU离线");

        ap_state = 0;
        ui->aplable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
        ui->aplable->setText("AP离线");

        //复位界面
        on_resetpushbutton_clicked();
    }
    serial_state_check --;

}

void MainWindow::handleTimeout_1()      //发送读参数指令
{
    //    if(F1_code == 0x0C)
    //    {
    //        //翻页
    //        //跳转到表格指定位置
    //        ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    //        ui->tableWidget->verticalScrollBar()->setSingleStep(2);
    //        ui->tableWidget->verticalScrollBar()->setSliderPosition(10*30);//跳转行到指定位置
    //        ui->tableWidget->viewport()->installEventFilter(this);//对此对象安装事件过滤器
    //        ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);//设置滚动模式按照像素滑动，做表格滑动时使用
    //    }

    //    if(F1_code == 0x12)
    //    {
    //        //跳转到表格指定位置
    //        ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    //        ui->tableWidget->verticalScrollBar()->setSingleStep(2);
    //        ui->tableWidget->verticalScrollBar()->setSliderPosition(18*30);//跳转行到指定位置
    //        ui->tableWidget->viewport()->installEventFilter(this);//对此对象安装事件过滤器
    //        ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);//设置滚动模式按照像素滑动，做表格滑动时使用
    //    }

    qDebug() << "handleTimeout_1" << flag_err << receive_succsee;

    if(readmcu == 0)    //还未读完mcu, 读完mcu的全部内容则标志位置1
    {
        if( flag_err == 0 && receive_succsee == 1)
        {
            F1_code++;
            receive_succsee = 0;
            if(F1_code == 4)
                F1_code = 5;
            if(F1_code == 0x0F)
                F1_code = 0x11;
            if(F1_code == 0x12)
                F1_code = 0x13;
        }

        Read_parameter_CHECK[5] = F1_code;
        serialCrc_Send(serial_mainboard,Read_parameter_CHECK,10);
    }

    //读玩mcu后开始读电压采集器
    if(readmcu == 1 && readfrock == 0 )
    {
        //发送读电压盒子指令，收到所有反馈则测试完成
        serialCrc_Send(serial_frock,frock_all,8);
        qDebug() << "serialCrc_Send(serial_frock,frock_all,8);";
        return ;
    }

    if(readmcu == 1 && readfrock == 1 && controlldsboard == 0)
    {
        //将LDS模组和wifi模组的状态写入表格
        if(periph_state[21] == 1)
        {
            ui->tableWidget->setItem(21,2,new QTableWidgetItem("1"));
            ui->tableWidget->item(21,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[21] = "1";
            oneTestresule(true,21);

        }
        else
        {
            ui->tableWidget->setItem(21,2,new QTableWidgetItem("0"));
            ui->tableWidget->item(21,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            oneTestresule(false,21);
        }

        //将LDS模组和wifi模组的状态写入表格
        if(periph_state[22] == 1)
        {
            ui->tableWidget->setItem(22,2,new QTableWidgetItem("1"));
            ui->tableWidget->item(22,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[22] = "1";
            oneTestresule(true,22);

        }
        else
        {
            ui->tableWidget->setItem(22,2,new QTableWidgetItem("0"));
            ui->tableWidget->item(22,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            oneTestresule(false,22);
        }

        //将LDS模组和wifi模组的状态写入表格
        if(periph_state[23] == 1)
        {
            ui->tableWidget->setItem(23,2,new QTableWidgetItem("1"));
            ui->tableWidget->item(23,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[23] = "1";
            oneTestresule(true,23);

        }
        else
        {
            ui->tableWidget->setItem(23,2,new QTableWidgetItem("0"));
            ui->tableWidget->item(23,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            oneTestresule(false,23);
        }

        controlldsboard = 1;
        //return;
        //m_pTimer_1->stop();     //关闭定时器
    }

    if(controlldsboard != 0 && controlldsboard < 8)    //继电器控制撞板
    {

        if(controlldsboard == 2)
        {
            qDebug() << "534";
            relaycontrol_all[7] = 0x09;    //闭合继电器4，启动成功
            serialCrc_Send(serial_relay,relaycontrol_all,13);

        }
        if(controlldsboard == 7)
        {
            qDebug() << "540";
            relaycontrol_all[7] = 0x01;    //闭合继电器4，启动成功
            serialCrc_Send(serial_relay,relaycontrol_all,13);
            m_pTimer_1->stop();
        }
        controlldsboard++;
    }

}


void MainWindow::SerialPortInit()
{


}

//串口开关
void MainWindow::on_OpenSerialButton_clicked()
{

    if(serial_mainboard->isOpen())    //如果串口打开了，先给他关闭
    {
        m_pTimer->stop();
        serial_mainboard->clear();
        serial_mainboard->close();

        //关闭状态，按钮显示
        ui->OpenSerialButton->setText("打开串口");

        //关闭状态，允许用户操作
        //ui->PortBox->setDisabled(false);
        // 关闭状态，颜色为绿色

        ui->OpenSerialButton->setStyleSheet("color: green;");
    }
    else  //如果串口关闭了，先给他打开
    {
        SerialPortInit();

        //当前选择的串口名字
        //serial_mainboard->setPortName(ui->PortBox->currentText());
        //用ReadWrite 模式尝试打开串口，无法接收数据时，发出警告
        if(!serial_mainboard->open(QIODevice::ReadWrite))
        {
            QMessageBox::warning(this,tr("提示"),tr("串口打开失败！"),QMessageBox::Ok);
            return;
        }
        else
        {

            // 打开状态，按钮显示“关闭串口”
            ui->OpenSerialButton->setText("关闭串口");

            // 打开状态，颜色为红色
            ui->OpenSerialButton->setStyleSheet("color: red;");

            //on_updetafreTextChanged();      //连上串口的时候，开始刷新扫地机各项参数
        }
    }
}

quint16 MainWindow::CRC_Compute(const quint8 *puchMsg, quint16 usDataLen)
{

    uint16_t crc16 = 0xffff;
    uint8_t uchCRCHi = 0xFF;
    uint8_t uchCRCLo = 0xFF;
        for (int i = 0; i < usDataLen; i++)
        {
            crc16 ^= puchMsg[i];

            for (int j = 0; j < 8; j++)
            {
                if ((crc16 & 0x01) == 1)
                {
                    crc16 = (crc16 >> 1) ^ 0xa001;
                }
                else
                {
                    crc16 = crc16 >> 1;
                }
            }
        }
        uchCRCHi = crc16 & 0xFF;
        uchCRCLo = crc16 >> 8;
        return ((uchCRCHi << 8) | (uchCRCLo));

}

typedef struct charToShort{
    char L;
    char H;
}CTS;

typedef union {
    CTS cts;
    short val;
}CTS_CS;

CTS_CS sts_Cs;
ushort val_test;


void MainWindow::MaincontrolDataReceived()
{

        serial_state_check = 1;

        //Sleep();
        data = serial_mainboard->readAll();        //读取数据
        qDebug() << " serial_mainboard接收帧长 " << data.size();

        for(int i = 0; i < data.size(); i++)
        {
        Read_data[count_number] = data.at(i);
        count_number++;
        }

        /*
        1、数据一次性接收
        2、数据多次接收
        */

         int j = 0; //便利接收缓冲区 Read_data
         int k = 0;

         if(!data.isEmpty())         //非空
         {

            for(j = 0; j < count_number; j++)
            {
                if(Read_data[j] == (char)0x54 && Read_data[j+1] == (char)0x41 ) //找头
                {
                    qDebug() << "帧头1" << j;
                    serial_head = 1;
                    frame_serial = j;
                    head_num = j+2;   //记录上一次的帧头位置
                    qDebug() << "帧头1" << head_num;

                }

                if(Read_data[j] == (char)0x4E && Read_data[j+1] == (char)0x47 &&  serial_head == 1   )
                     //找尾
                {

                    qDebug() << "帧尾1" << j;
                    qDebug() << "head_num" << head_num;

                    if(CRC_Compute(&Read_data[frame_serial],4+(Read_data[frame_serial+2] << 8 | Read_data[frame_serial+3] )) == (Read_data[j-2]<< 8| Read_data[j-1] ))
                    {

                    qDebug() << "帧尾1_CRC";
                    serial_tail = 1;

                    tail_num = j+2;

                    if(Read_data[head_num+2] == 0xF1 )
                    {
                        switch(Read_data[head_num+3])
                        {
                        case 1 :
                            if(Read_data[head_num+4] == 0x01)
                            {
                                ui->mculable->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");
                                ui->mculable->setText("MCU在线");
                                mcu_state = 1;      //MCU启动成功
                            }
                            else
                            {
                                ui->mculable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
                                ui->mculable->setText("MCU离线");
                                mcu_state = 0;      //启动失败
                            }
                            break;

                        case 2:
                            if(Read_data[head_num+4] == 0x01)
                            {
                                ui->aplable->setStyleSheet("QLabel{background-color:rgb(0,255,0);}");
                                ui->aplable->setText("AP在线");
                                ap_state = 1;      //AP启动成功
                            }
                            else
                            {
                                ui->aplable->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
                                ui->aplable->setText("AP离线");
                                ap_state = 0;      //AP启动失败
                            }
                            break;

                        case 3:     //电池电压
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                            receive_succsee = 1;        //接收成功
                            ui->tableWidget->setItem(0,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(0,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[0] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 1500)
                            {
                                oneTestresule(true,0);
                            }
                            else
                            {
                                oneTestresule(false,0);
                            }
                            break;

                        case 4:     //充电桩电压
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);


                            break;

                        case 5:     //电池电流（short）
                            receive_succsee = 1;
                            if((Read_data[head_num+4] & 0x10) == 0x10)    //负数
                            {
                                peripvalue = "-" + QString::number(~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF);
                                ui->tableWidget->setItem(1,2,new QTableWidgetItem(peripvalue));
                                ui->tableWidget->item(1,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[1] = peripvalue;
                                if((~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF) > 700)
                                {
                                    oneTestresule(true,1);
                                }
                                else
                                {
                                    oneTestresule(false,1);
                                }
                            }
                            else        //正数
                            {
                                peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);

                            }

                            break;


                        case 6:     //风机转速
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                            receive_succsee = 1;
                            ui->tableWidget->setItem(2,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(2,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[2] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 500)
                            {
                               oneTestresule(true,2);
                            }
                            else
                            {
                               oneTestresule(false,2);
                            }

                            break;

                        case 7:     //滚刷电流
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                            ui->tableWidget->setItem(3,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(3,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[3] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 0)
                            {
                               oneTestresule(true,3);
                            }
                            else
                            {
                                oneTestresule(false,3);
                            }

                            break;

                        case 8:     //边刷电流
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                            ui->tableWidget->setItem(4,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(4,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[4] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 0)
                            {
                                oneTestresule(true,4);
                            }
                            else
                            {
                                oneTestresule(false,4);
                            }

                            break;

                        case 9:     //水泵电流
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                            ui->tableWidget->setItem(5,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(5,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[5] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 0)
                            {
                                oneTestresule(true,5);
                            }
                            else
                            {
                                oneTestresule(false,5);
                            }

                            break;

                        case 0x0A:      //驱动轮电流-左
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);

                            ui->tableWidget->setItem(6,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(6,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[6] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 0)
                            {
                              oneTestresule(true,6);
                            }
                            else
                            {
                                oneTestresule(false,6);
                            }


                            break;

                        case 0x0B:      //驱动轮电流-右
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);

                            ui->tableWidget->setItem(7,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(7,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[7] = peripvalue;
                            if((Read_data[head_num+4] << 8 | Read_data[head_num+5]) >= 0)
                            {
                                oneTestresule(true,7);
                            }
                            else
                            {
                                oneTestresule(false,7);
                            }


                            break;

                        case 0x0C:      //下视-左
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);

                            ui->tableWidget->setItem(8,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(8,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[8] = peripvalue;

                            if( ((Read_data[head_num+4] << 8 | Read_data[head_num+5]) < down_look[0])) //判断阈值
                            {
                                oneTestresule(true, 8);
                            }
                            else
                            {
                                oneTestresule(false, 8);
                            }

                            break;

                        case 0x0D:      //下视-中
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                            ui->tableWidget->setItem(9,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(9,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[9] = peripvalue;
                            if( ((Read_data[head_num+4] << 8 | Read_data[head_num+5]) < down_look[1]) ) //判断阈值
                            {
                                oneTestresule(true, 9);
                            }
                            else
                            {
                               oneTestresule(false, 9);

                            }
                            break;

                        case 0x0E:      //下视-右
                            receive_succsee = 1;
                            peripvalue = QString::number(Read_data[head_num+4]<<8 | Read_data[head_num+5]);

                            ui->tableWidget->setItem(10,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(10,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[10] = peripvalue;
                            if( ((Read_data[head_num+4] << 8 | Read_data[head_num+5]) < down_look[2]) ) //判断阈值
                            {
                                oneTestresule(true, 10);
                            }
                            else
                            {
                                oneTestresule(false, 10);
                            }

                            break;

                        case 0x0F:      //电源按键（0未触发，1触发）
                            peripvalue = QString::number(Read_data[head_num+4]);

                            break;

                        case 0x10:      //回冲按键（0未触发，1触发）
                            peripvalue = QString::number(Read_data[head_num+4]);

                            break;

                        case 0x11:      //红外接收灯
                            qDebug() << "0x11receive";
                            receive_succsee = 1;

                            peripvalue = QString::number(Read_data[head_num+4]);
                            ui->tableWidget->setItem(11,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(11,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[11] = peripvalue;

                            peripvalue = QString::number(Read_data[head_num+5]);
                            ui->tableWidget->setItem(12,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(12,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[12] = peripvalue;

                            peripvalue = QString::number(Read_data[head_num+6]);
                            ui->tableWidget->setItem(13,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(13,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[13] = peripvalue;

                            peripvalue = QString::number(Read_data[head_num+7]);
                            ui->tableWidget->setItem(14,2,new QTableWidgetItem(peripvalue));
                            ui->tableWidget->item(14,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[14] = peripvalue;

                            if(Read_data[head_num+4] != 0)
                            {
                                oneTestresule(true, 11);
                            }
                            else
                            {
                                oneTestresule(false, 11);
                            }

                            if(Read_data[head_num+5] != 0)
                            {
                                oneTestresule(true, 12);
                            }
                            else
                            {
                                oneTestresule(false, 12);
                            }

                            if(Read_data[head_num+6] != 0)
                            {
                                oneTestresule(true, 13);
                            }
                            else
                            {
                                oneTestresule(false, 13);
                            }

                            if(Read_data[head_num+7] != 0)
                            {
                                oneTestresule(true, 14);
                            }
                            else
                            {
                                oneTestresule(false, 14);
                            }
                            break;


                        case 0x12:      //电池容量
                            peripvalue = QString::number(Read_data[head_num+4])+"%";

                            break;

                        case 0x13:      //驱动轮码盘-左

                            receive_succsee = 1;
                            if((Read_data[head_num+4] & 0x10) == 0x10)    //负数
                            {
                                peripvalue = "-" + QString::number(~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF);
                                ui->tableWidget->setItem(15,2,new QTableWidgetItem(peripvalue));
                                ui->tableWidget->item(15,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[15] = peripvalue;
                                if((~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF) > 700)
                                {
                                    oneTestresule(true,15);
                                }
                                else
                                {
                                    oneTestresule(false,15);
                                }
                            }
                            else        //正数
                            {
                                peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                                ui->tableWidget->setItem(15,2,new QTableWidgetItem(peripvalue));
                                ui->tableWidget->item(15,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[15] = peripvalue;
                                if(((Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF) > 700)
                                {
                                    oneTestresule(true,15);
                                }
                                else
                                {
                                    oneTestresule(false,15);
                                }

                            }

                            break;

                        case 0x14:      //驱动轮码盘-右
                            receive_succsee = 1;
                            if((Read_data[head_num+4] & 0x10) == 0x10)    //负数
                            {

                                peripvalue = "-" + QString::number(~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF);
                                ui->tableWidget->setItem(16,2,new QTableWidgetItem(peripvalue));
                                ui->tableWidget->item(16,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[16] = peripvalue;
                                if((~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF) > 700)
                                {

                                    readmcu = 1;
                                    oneTestresule(true,16);
                                }
                                else
                                {
                                    qDebug() << "0x14驱动轮"<<"-"<<~(Read_data[head_num+4] << 8 | Read_data[head_num+5]) ;
                                    oneTestresule(false,16);
                                }
                            }
                            else        //正数
                            {
                                peripvalue = QString::number(Read_data[head_num+4] << 8 | Read_data[head_num+5]);
                                ui->tableWidget->setItem(16,2,new QTableWidgetItem(peripvalue));
                                ui->tableWidget->item(16,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[16] = peripvalue;
                                if(((Read_data[head_num+4] << 8 | Read_data[head_num+5]) & 0xeFFF) > 700)
                                {
                                    readmcu = 1;
                                    oneTestresule(true,16);
                                }
                                else
                                {
                                    oneTestresule(false,16);
                                }

                            }
                            break;


                        case 0x61:
                            ap_version = QString::number(Read_data[head_num+4]) + "." +
                                         QString::number(Read_data[head_num+5]) + "." +
                                         QString::number(Read_data[head_num+6]);

                            break;

                        case 0x62:
                            receive_succsee = 1;
                            qDebug() << "0x62" <<"123456789";
                            mcu_version = QString::number(Read_data[head_num+4]) + "." +
                                          QString::number(Read_data[head_num+5]) + "." +
                                          QString::number(Read_data[head_num+6]);

                            ui->mcuversionlineEdit->setText(mcu_version);
                            F1_code = 2;
                            //JudegVersion();

                            break;

                        case 0x68:

                            for(k = head_num+4; k <= head_num+4+20; k++)
                            {
                                printf("[%d] %x", k, Read_data[k]);
                            }

                            break;

                        case 0x63:
                            mcuactivatedState = Read_data[head_num+4];

                            if(Read_data[head_num+4] == 1)

                            break;

                        case 0x6B:
                            apactivatedState = Read_data[head_num+4];

                            if(Read_data[head_num+4] == 1)

                            break;
                        }
                    }

                    if(Read_data[head_num+2] == 0xF3)
                    {


                        if(Read_data[head_num+3] == 0x0D)
                        {
                            if(Read_data[head_num+4] == 0x01 )   //电源按键
                            {
                            ui->tableWidget->setItem(30,2,new QTableWidgetItem("1"));
                            ui->tableWidget->item(30,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            Testing_result[30] = "1";
                            oneTestresule(true,30);

                            }
                            else
                            {
                            ui->tableWidget->setItem(30,2,new QTableWidgetItem("0"));
                            ui->tableWidget->item(30,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                            oneTestresule(false,30);
                            }
                        }

                        if(Read_data[head_num+3] == 0x0E)
                        {
                            if(Read_data[head_num+4] == 0x01 &&  periph_state[14] != 2)   //陀螺仪
                            {
                                periph_state[23] = 1;

                            }
                            else
                            {
                                periph_state[23] = 2;

                            }
                        }

                        if(Read_data[head_num+3] == 0x13)
                        {
                            if(Read_data[head_num+4] == 0x01 &&  periph_state[19] != 2)   //驱动轮码盘-左
                            {
                                periph_state[19] = 1;

                            }
                            else
                            {
                                periph_state[19] = 2;

                            }
                        }

                        if(Read_data[head_num+3] == 0x17)
                        {
                            if(Read_data[head_num+4] == 0x01)   //LDS撞板
                            {
                                periph_state[23] = 1;

                                ui->tableWidget->setItem(24,2,new QTableWidgetItem("1"));
                                ui->tableWidget->item(24,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[24] = "1";
                                oneTestresule(true,24);

                            }
                            else
                            {
                                ui->tableWidget->setItem(24,2,new QTableWidgetItem("0"));
                                ui->tableWidget->item(24,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                oneTestresule(false,24);
                            }
                        }

                        if(Read_data[head_num+3] == 0x18)
                        {
                            if(Read_data[head_num+4] == 0x01)   //左撞板
                            {
                                ui->tableWidget->setItem(25,2,new QTableWidgetItem("1"));
                                ui->tableWidget->item(25,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[25] = "1";
                                oneTestresule(true,25);

                            }
                            else
                            {
                                ui->tableWidget->setItem(25,2,new QTableWidgetItem("0"));
                                ui->tableWidget->item(25,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                oneTestresule(false,25);
                            }
                        }

                        if(Read_data[head_num+3] == 0x19)
                        {
                            if(Read_data[head_num+4] == 0x01)   //右撞板
                            {
                                ui->tableWidget->setItem(26,2,new QTableWidgetItem("1"));
                                ui->tableWidget->item(26,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[26] = "1";
                                oneTestresule(true,26);

                            }
                            else
                            {
                                ui->tableWidget->setItem(26,2,new QTableWidgetItem("0"));
                                ui->tableWidget->item(26,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                oneTestresule(false,26);
                            }
                        }

                        if(Read_data[head_num+3] == 0x1A)
                        {
                            if(Read_data[head_num+4] == 0x01)   //抹布支架检测（霍尔）
                            {
                                ui->tableWidget->setItem(27,2,new QTableWidgetItem("1"));
                                ui->tableWidget->item(27,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[27] = "1";
                                oneTestresule(true,27);

                            }
                            else
                            {
                                ui->tableWidget->setItem(27,2,new QTableWidgetItem("0"));
                                ui->tableWidget->item(27,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                oneTestresule(false,27);
                            }
                        }

                        if(Read_data[head_num+3] == 0x1B)
                        {
                            if(Read_data[head_num+4] == 0x01 )   //尘盒检测（霍尔）
                            {
                                ui->tableWidget->setItem(28,2,new QTableWidgetItem("1"));
                                ui->tableWidget->item(28,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[28] = "1";
                                oneTestresule(true,28);

                            }
                            else
                            {
                                ui->tableWidget->setItem(28,2,new QTableWidgetItem("0"));
                                ui->tableWidget->item(28,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                oneTestresule(false,28);
                            }
                        }
                        if(Read_data[head_num+3] == 0x1C)
                        {
                            if(Read_data[head_num+4] == 0x01 && periph_state[28] != 2)   //WiFi 模组
                            {
                                periph_state[21] = 1;

                            }
                            else
                            {
                                periph_state[21] = 2;

                            }
                        }
                        if(Read_data[head_num+3] == 0x1D)
                        {

                            if(Read_data[head_num+4] == 0x01 && periph_state[29] != 2)   //LDS 模组
                            {
                                periph_state[22] = 1;

                            }
                            else
                            {
                                periph_state[22] = 2;

                            }
                        }
                        if(Read_data[head_num+3] == 0x1E)
                        {
                            if(Read_data[head_num+4] == 0x01)   //回充按键
                            {
                                ui->tableWidget->setItem(29,2,new QTableWidgetItem("1"));
                                ui->tableWidget->item(29,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                Testing_result[29] = "1";
                                oneTestresule(true,29);

                            }
                            else
                            {
                                ui->tableWidget->setItem(29,2,new QTableWidgetItem("0"));
                                ui->tableWidget->item(29,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                                oneTestresule(false,29);
                            }
                        }

                    }
                }
                }
            }

        }

        frame_serial = 0;
        head_num = 0;
        count_number = 0;
        serial_head = 0;
        serial_tail = 0;
        data.clear();

}



void MainWindow::forckDataReceived()
{

    serial_frock_flag = 1;

    frock_buff.append(serial_frock->readAll());
    qDebug() << "serial_frock接收帧长"  << frock_buff.size();

    if(frock_buff.size() >= 37)
    {

        for(int i= 0; i < frock_buff.size(); i++)
        {
            Read_data[i] = frock_buff.at(i);
            printf("%x ",Read_data[i]);
        }
        std::cout << std::endl;

        //判断crc校验是否正确
        if(CRC_Compute(Read_data,frock_buff.size() - 2) == (Read_data[35] << 8 | Read_data[36]))
        {
            //电池电压
            receive_succsee = 1;

            peripvalue = QString::number(Read_data[3] << 8 | Read_data[4]);
            ui->tableWidget->setItem(17,2,new QTableWidgetItem(peripvalue));
            ui->tableWidget->item(17,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[17] = peripvalue;
            if((Read_data[4] << 8 | Read_data[5]) >= 1600)
            {
                oneTestresule(true,17);
            }
            else
            {
                oneTestresule(false,17);
            }


            //LDS电压
            peripvalue = QString::number(Read_data[7] << 8 | Read_data[8]);
            ui->tableWidget->setItem(18,2,new QTableWidgetItem(peripvalue));
            ui->tableWidget->item(18,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[18] = peripvalue;
            if((Read_data[4] << 8 | Read_data[5]) >= 1600)
            {
                oneTestresule(true,18);
            }
            else
            {
                oneTestresule(false,18);
            }


            peripvalue = QString::number(Read_data[9] << 8 | Read_data[10]);
            ui->tableWidget->setItem(19,2,new QTableWidgetItem(peripvalue));
            ui->tableWidget->item(19,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[19] = peripvalue;
            if((Read_data[4] << 8 | Read_data[5]) >= 1600)
            {
                oneTestresule(true,19);
            }
            else
            {
                oneTestresule(false,19);
            }


            peripvalue = QString::number(Read_data[11] << 8 | Read_data[12]);
            ui->tableWidget->setItem(20,2,new QTableWidgetItem(peripvalue));
            ui->tableWidget->item(20,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
            Testing_result[20] = peripvalue;
            if((Read_data[4] << 8 | Read_data[5]) >= 1600)
            {
                oneTestresule(true,20);
            }
            else
            {
                oneTestresule(false,20);
            }

            readfrock = 1;

        }


        qDebug()<<frock_buff.toHex(' ');
        frock_buff.clear();
    }


}

void MainWindow::relayDataReceived()
{
    serial_relay_flag = 1;
    relay_buff.append(serial_relay->readAll());

    relay_buff.clear();

}

void MainWindow::oneTestresule(bool result, int row)
{
        if(result)
        {

            periph_state[row] = 1;
            ui->tableWidget->setItem(row,4,new QTableWidgetItem("PASS"));
            for(int a = 0; a < 5; a++)
            {
                ui->tableWidget->item(row,a)->setBackground(QColor(0,255,0));
            }
        }
        else
        {
            flag_err++;
            if( flag_err == 3  || row >=17)
            {
                if(row == 16)
                    readmcu = 1;
                periph_state[row] = 0;
                flag_err = 0;
                ui->tableWidget->setItem(row,4,new QTableWidgetItem("NG"));
                ui->tableWidget->item(row,4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
                for(int a = 0; a < 5; a++)
                {
                    ui->tableWidget->item(row,a)->setBackground(QColor(255,0,0));
                }
            }
        }
}




MainWindow::~MainWindow()
{



    delete ui;
}


//FCT开始
void MainWindow::on_fct1pushButton_clicked()
{
    FCT_CHECK[6] = 0x01;
    serialCrc_Send(serial_mainboard,FCT_CHECK,11);


}

void MainWindow::serialCrc_Send(QSerialPort *serial,quint8 *puchMsg, quint16 usDataLen)
{
    if(serial->portName() == serial_mainboard->portName())
    {
        puchMsg[usDataLen-4] = CRC_Compute(puchMsg, usDataLen-4) >> 8;
        puchMsg[usDataLen-3] = CRC_Compute(puchMsg, usDataLen-4);
    }
    else
    {
        puchMsg[usDataLen-2] = CRC_Compute(puchMsg, usDataLen-2) >> 8;
        puchMsg[usDataLen-1] = CRC_Compute(puchMsg, usDataLen-2);
    }
    for(int i = 0; i < usDataLen; i++)
    {
        //qDebug("0x%x ",puchMsg[i]);
        //std::cout <<  puchMsg[i];
        printf("%x  ", puchMsg[i]);

    }

    std::cout << std::endl;

    serial->write((const char*)puchMsg,usDataLen);
//    serial->flush();
//    if (serial->waitForBytesWritten(3000)) {
//        qDebug() << "数据发送完成";
//    } else {
//        qDebug() << "等待数据发送超时：" << serial->errorString();
//    }

}

void MainWindow::on_fct2pushButton_clicked()
{

    FCT_CHECK[6] = 0x02;
    serialCrc_Send(serial_mainboard,FCT_CHECK,11);

    relaycontrol_all[7] = 0x00;
    relaycontrol_all[8] = 0x00;
    serialCrc_Send(serial_relay,relaycontrol_all,13);

//    //发送FCT结束指令
//    FCT_CHECK[6] = 0x12;
//    serialCrc_Send(serial_mainboard,FCT_CHECK,11);

//    //发送更新频率
//    UPDATE_FRE[6] = 0;
//    UPDATE_FRE[7] = 0;
//    serialCrc_Send(UPDATE_FRE,12);

}


void MainWindow::on_updetafreTextChanged()
{
    QString text = ui->updatefrelineEdit->text();
    quint16 updatefre = text.toInt();
    UPDATE_FRE[6] = updatefre>>8;
    UPDATE_FRE[7] = updatefre & 0xff;

    serialCrc_Send(serial_mainboard,UPDATE_FRE,12);
}


void MainWindow::on_resetpushbutton_clicked()
{

    mcu_state = 0;
    ap_state = 0;
    serial_head = 0;
    serial_tail = 0;

    count_number = 0;//数组下标
    head_num = 0;    //一帧中第一个数据下标
    tail_num = 0;    //一帧中末尾数据下标

    flag_version = 0;
    QString ap_version ;    //AP版本字符串
    QString mcu_version ;   //MCU版本字符串
    QString peripvalue = 0;     //外设lineedit显示字符串

    //start_mainboard

    memset(Read_data,0,5000);
    memset(periph_state,0,40);

    ui->mculable->setFrameShape (QFrame::Box);
    ui->mculable->setStyleSheet("border-width: 1px;border-style: solid;border-color: rgb(0, 0, 0);");

    ui->aplable->setFrameShape (QFrame::Box);
    ui->aplable->setStyleSheet("border-width: 1px;border-style: solid;border-color: rgb(0, 0, 0);");




    ui->mcuversionlineEdit->setText("");

    relaycontrol_all[7] = 0x00;
    relaycontrol_all[8] = 0x00;
    serialCrc_Send(serial_relay,relaycontrol_all,13);


//    ui->fct1pushButton->setEnabled(false);
//    ui->fct2pushButton->setEnabled(false);

}


void MainWindow::on_periphpushbutton_clicked()
{
    qDebug() << "on_periphpushbutton_clicked";
    m_pTimer_1->start(TIMER_TIMEOUT_1);
}




//定时器更新
void MainWindow::updata_UI()
{
    for(int i = 0; i <= 40; i++)
    {
        if(periph_state[i] == 1)
        {

        }
    }
}


void MainWindow::JudegVersion()
{

    if(version_flag == 0)
    {

        version_flag = 100;
        if(mcu_version !=ui->mcuversionlineEdit_2->text() )
        {
            QMessageBox::warning(this, tr("版本错误"),  tr("版本错误，请更新版本"),
                          QMessageBox::Discard);
            //QMessageBox::warning(this, tr("提示"), text, tr("是"),tr("否"));

        }
        if(ap_version != ui->apversionlineEdit_2->text())
        {

        }
    }
}

void MainWindow::Table_init()
{
    // 创建字体对象并设置字体大小
    QFont font;
    font.setPointSize(16);  // 设置字体大小为14


    // 将字体应用于整个表格
    ui->tableWidget->setFont(font);

    ui->tableWidget->setColumnCount(5);//设置列数
    ui->tableWidget->setRowCount(32);//设置行数
    ui->tableWidget->setWindowTitle("tableWidget");

    int targetRow = 2;  // 设置第3行的颜色（行索引从0开始）
    QStringList m_Header;
    m_Header<<QString("序号")<<QString("测试项目")<<QString("测试数据")<<QString("测试标准")<<QString("结果");
    ui->tableWidget->setHorizontalHeaderLabels(m_Header);//添加横向表头
    //ui->tableWidget->verticalHeader()->setVisible(true);//纵向表头可视化
    ui->tableWidget->horizontalHeader()->setVisible(true);//横向表头可视化
    ui->tableWidget->verticalHeader()->setHidden(true);     //隐藏纵列序号
    //ui->tableWidget->setShowGrid(false);//隐藏栅格
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置编辑方式：禁止编辑表格
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//设置表格选择方式：设置表格为整行选中
    //ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectColumns);//设置表格选择方式：设置表格为整列选中
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//选择目标方式
    ui->tableWidget->setStyleSheet("selection-background-color:pink");//设置选中颜色：粉色


    //编辑单元格内容
    ui->tableWidget->setItem(1,2,new QTableWidgetItem("你好"));
    QString num;
    for(int i = 0; i < 32; i++)
    {
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(QString("%1").arg(i+1)));
    }


    //设置水平表头为拉伸模式
   // ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);



    ui->tableWidget->setColumnWidth(0, 200);  // 设置第一列宽度为150
    ui->tableWidget->setColumnWidth(1, 251);  // 设置第一列宽度为150
    ui->tableWidget->setColumnWidth(2, 200);  // 设置第一列宽度为150
    ui->tableWidget->setColumnWidth(3, 200);  // 设置第一列宽度为150
    ui->tableWidget->setColumnWidth(4, 200);  // 设置第一列宽度为150
   // ui->tableWidget->item(1,2 )->setBackground(QColor(255,0,0));    //设置单元格背景
    // ui->tableWidget->setStyleSheet("background-color: lightgray;");
    //    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {

    //        QTableWidgetItem *item = ui->tableWidget->item(targetRow,col );
    //        qDebug() << "col" << col;
    //            //item->setBackground(Qt::red);
    //            item ->setBackground(QColor(255,0,0));
    //    }

    //让tableWidget内容中的每个元素居中
    for (int i=0;i<32;i++)
    {
        ui->tableWidget->item(i,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //单元格必须有内容
    }

}


//读取配置文件
void MainWindow::ReadConfigFile()
{
    QFile configpath(Configfilepath);
    if( !configpath.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "配置文件打开失败！请检测配置文件";
        return;
    }

     //创建QTextStream对象，并绑定到文件
    QTextStream Qstream(&configpath);

    int i = 0;
    // 读取文件内容并解析
    while (!Qstream.atEnd())
    {
        QString line = Qstream.readLine();    //逐行读取文件内容
        // 在这里添加解析逻辑，对每一行进行处理
        QStringList values = line.split("，");
        //Myxlsx.
        for (int j = 1; j < values.size(); ++j) {
            QString element = values.at(j);
            // 处理元素
            ui->tableWidget->setItem(i,(j==1?1:3),new QTableWidgetItem(element));
            ui->tableWidget->item(i,(j==1?1:3))->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);        //设置单元格居中
        }
        i++;
    }

    configpath.close();
}

void MainWindow::on_OpenSerialButton_2_clicked()
{

    relaycontrol_all[7] = 0x09;    //闭合继电器4，启动成功
    serialCrc_Send(serial_relay,relaycontrol_all,13);
}

