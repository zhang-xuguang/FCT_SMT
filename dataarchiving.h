#ifndef DATAARCHIVING_H
#define DATAARCHIVING_H


#include "ui_mainwindow.h"

#include <QFile>




class Dataarchiving : public QMainWindow
{
    Q_OBJECT
public:
    explicit Dataarchiving(QWidget *parent = nullptr);

    //写入AP的UUID,参数（UUID，测试机器数量，通过/不通过(0/1））

    //写入机器各项功能测试的通过情况
    void MyxlsxWrite_State(const char * periph_state);

    //写入机器各项功能的具体参数
    void MyxlsxWrite_parameter(int Fctnum , QString QR_code, bool state, QStringList &List);

    void Myxlsx_save();

    void Myxlsx_config(QString configpath);



signals:

private:
    // 要处理的Excel文件路径
    QString filePath = "D:/qt-progect/FCT_SMT/FCT_SMT_1221/file.xlsx";


};

#endif // DATAARCHIVING_H
