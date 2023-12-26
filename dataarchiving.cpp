#include "dataarchiving.h"

#include <QFile>
#include <QtCore/QTextStream>

#include <QtCore/QFile>
#include <QtCore/QIODevice>



Dataarchiving::Dataarchiving(QWidget *parent)
    : QMainWindow{parent}
{

}

//写入序号，uuid，检测结果
void Dataarchiving::MyxlsxWrite_QR_code( QString QR_code, quint64 Fctnum ,bool state )
{
    QString temp = "A"+QString::number(Fctnum +1);
    QString temp1 = "B"+QString::number(Fctnum +1);
    QString temp2 = "C" + QString::number(Fctnum+1);

    xlsx.write(temp, Fctnum);
    xlsx.write(temp1, QR_code);
    if(state)
        xlsx.write(temp2, "PASS");
    else
        xlsx.write(temp2, "NG");
}



void Dataarchiving::Myxlsx_save()
{

    xlsx.saveAs("../FCT_SMT_1221/Test.xlsx");
}


void Dataarchiving::Myxlsx_config(QString configpath)
{

    QFile configfile(configpath);
    if( !configfile.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "配置文件打开失败！请检测配置文件";
        return;
    }


    QXlsx::Format format1;
    format1.setHorizontalAlignment(QXlsx::Format::AlignHCenter);/*横向居中*/

    QTextStream in(&configfile);

    xlsx.write("A1" , "序号");
    xlsx.write("B1" , "二维码");
    xlsx.write("C1" , "检测结果",format1);

    // 读取文件内容并解析
    while (!in.atEnd())
    {
        QString line = in.readLine();    //逐行读取文件内容
        // 在这里添加解析逻辑，对每一行进行处理
        QStringList values = line.split("，");
       // QString element = values.at(0);
        // 处理元素FXLT20220506836
        if(values.size() != 3)
            continue;

        xlsx.write(values.at(0)+"1" , values.at(1),format1);
    }
}

void Dataarchiving::MyxlsxWrite_parameter( QStringList &List, int num ,QString configpath)
{
    QFile configfile(configpath);
    if( !configfile.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "配置文件打开失败！请检测配置文件";
        return;
    }
    QTextStream in(&configfile);

    int i = 0;
    // 读取文件内容并解析
    while (!in.atEnd())
    {
        QString line = in.readLine();    //逐行读取文件内容
        // 在这里添加解析逻辑，对每一行进行处理
        QStringList values = line.split("，");

        if(values.size() != 3)
            continue;

        // 处理元素
        xlsx.write(values.at(0)+QString::number(num+1) , List.at(i));
        i++;
    }
}
