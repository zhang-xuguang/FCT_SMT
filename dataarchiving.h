#ifndef DATAARCHIVING_H
#define DATAARCHIVING_H


#include "ui_mainwindow.h"

#include <QFile>

#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"


class Dataarchiving : public QMainWindow
{
    Q_OBJECT
public:
    explicit Dataarchiving(QWidget *parent = nullptr);

    //写入AP的UUID,参数（UUID，测试机器数量，通过/不通过(0/1））
    void MyxlsxWrite_QR_code( QString uuid, quint64 Fctnum ,bool state );

    //写入机器各项功能测试的通过情况
    void MyxlsxWrite_State(const char * periph_state);

    //写入机器各项功能的具体参数
    void MyxlsxWrite_parameter( QStringList &List, int num ,QString configpath);

    void Myxlsx_save();

    void Myxlsx_config(QString configpath);


signals:


private:
    QXlsx::Document xlsx;

};

#endif // DATAARCHIVING_H
