#include "dataarchiving.h"

#include <QFile>
#include <QtCore/QTextStream>

#include <QtCore/QFile>
#include <QtCore/QIODevice>



Dataarchiving::Dataarchiving(QWidget *parent)
    : QMainWindow{parent}
{

//    QXlsx::Format format1;
//    format1.setHorizontalAlignment(QXlsx::Format::AlignHCenter);/*横向居中*/

   // //    xlsx.write(1, 2, "Hello Qt!");
//    xlsx.write(2, 2, QStringLiteral("中文"));
//    //xlsx.saveAs("Text.xlsx");

  //  QXlsx::Format format1;

    //format1.setFontColor(QColor(Qt::red));   //文字为红色
   // format1.setPatternBackgroundColor(QColor("#00B050"));  //背景颜色

    //format1.setPatternForegroundColor(Qt::red);
   // format1.setFontSize(30);  // 设置字体大小
//    format1.setHorizontalAlignment(QXlsx::Format::AlignHCenter);  //横向居中
//   // format1.setBorderStyle(QXlsx::Format::BorderDashDotDot);  //边框样式
    //xlsx.write("A2", "Hello Qt!", format1);//写入文字，按照刚才设置的样式
//    xlsx.write(2, 1, 12345, format1);//写入文字，按照刚才设置的样式

//    xlsx.write("A3","12345");
//    //第2行第1列
//    xlsx.saveAs("Text.xlsx");
}


void Dataarchiving::MyxlsxWrite_UUID( QString uuid, quint64 Fctnum ,bool state )
{
    QString temp = "A"+QString::number(Fctnum + 1);
    QString temp1 = "B"+QString::number(Fctnum + 1);
    QString temp2 = "C" + QString::number(Fctnum+1);

    xlsx.write(temp, Fctnum);
    xlsx.write(temp1, uuid);
    if(state)
        xlsx.write(temp2, "PASS");
    else
        xlsx.write(temp2, "NG");
}



void Dataarchiving::Myxlsx_save()
{
    xlsx.saveAs("Text1.xlsx");
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
    xlsx.write("B1" , "UUID");
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
        xlsx.write(values.at(0)+QString::number(num) , List.at(i));
        i++;
    }
}
