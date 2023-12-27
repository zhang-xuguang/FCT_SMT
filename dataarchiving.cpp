#include "dataarchiving.h"

#include <QFile>
#include <QtCore/QTextStream>

#include <QtCore/QFile>
#include <QtCore/QIODevice>

#include <QAxObject>
#include <QDir>



Dataarchiving::Dataarchiving(QWidget *parent)
    : QMainWindow{parent}
{

}

//写入序号，uuid，检测结果



void Dataarchiving::Myxlsx_save()
{

}


void Dataarchiving::Myxlsx_config( QString configpath)
{

    QFile configfile(configpath);
    if( !configfile.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "配置文件打开失败！请检测配置文件";
        return;
    }

    // 创建一个Excel对象
    QAxObject *excel = new QAxObject("Excel.Application", nullptr);
    if (!excel->isNull()) {
        excel->setProperty("Visible", false);  // 设置Excel应用程序不可见

        // 获取Workbooks对象
        QAxObject *workbooks = excel->querySubObject("Workbooks");
        if (!workbooks->isNull()) {
            // 打开Excel文件
            QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", filePath);
            if (!workbook->isNull()) {
                // 获取第一个工作表
                QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);
                if (!worksheet->isNull()) {

                    // 设置新的值
                    worksheet->querySubObject("Cells(int,int)", 1, 1)->setProperty("Value", "序号");
                    worksheet->querySubObject("Cells(int,int)", 1, 2)->setProperty("Value", "二维码");
                    worksheet->querySubObject("Cells(int,int)", 1, 3)->setProperty("Value", "检测结果");


                    QTextStream in(&configfile);
                    int i = 4;
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
                        worksheet->querySubObject("Cells(int,int)", 1, i)->setProperty("Value", values.at(1));
                        i++;
//                        xlsx.write(values.at(0)+"1" , values.at(1),format1);
                    }

                    // 保存并关闭工作簿
                    workbook->dynamicCall("Save()");
                    workbook->dynamicCall("Close()");
                } else {
                    qDebug() << "Failed to get cell for update";
                }

                // 释放工作簿对象
                delete workbook;
            }
            else {
                qDebug() << "Failed to get the first worksheet";
            }


            // 释放工作簿集合对象
            delete workbooks;

        }
        else {
            qDebug() << "Failed to open workbook:" << filePath;
        }

        // 退出Excel应用程序
        excel->dynamicCall("Quit()");

        // 释放Excel对象
        delete excel;

    }
    else {
        qDebug() << "Failed to create Excel application object";
    }

}

void Dataarchiving::MyxlsxWrite_parameter(int Fctnum , QString QR_code, bool state, QStringList &List)
{

    // 创建一个Excel对象
    QAxObject *excel = new QAxObject("Excel.Application", nullptr);
    if (!excel->isNull()) {
        excel->setProperty("Visible", false);           //设置Excel应用程序不可见

        // 获取Workbooks对象
        QAxObject *workbooks = excel->querySubObject("Workbooks");
        if (!workbooks->isNull()) {
            // 打开Excel文件
            QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", filePath);
            if (!workbook->isNull()) {
                // 获取第一个工作表
                QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);
                if (!worksheet->isNull()) {
                    // 设置新的值
                    worksheet->querySubObject("Cells(int,int)", Fctnum, 1)->setProperty("Value", QString::number(Fctnum-1));
                    worksheet->querySubObject("Cells(int,int)", Fctnum, 2)->setProperty("Value", QR_code);
                    if(state)
                        worksheet->querySubObject("Cells(int,int)", Fctnum, 3)->setProperty("Value", "PASS");
                    else
                        worksheet->querySubObject("Cells(int,int)", Fctnum, 3)->setProperty("Value", "NG");

                    for(int i = 0 ; i < List.size(); i++)
                    {
                        worksheet->querySubObject("Cells(int,int)", Fctnum, i+4)->setProperty("Value", List.at(i));
                    }

                    // 保存并关闭工作簿
                    workbook->dynamicCall("Save()");
                    workbook->dynamicCall("Close()");
                } else {
                    qDebug() << "Failed to get cell for update";
                }

                // 释放工作簿对象
                delete workbook;
            }
            else {
                qDebug() << "Failed to get the first worksheet";
            }


            // 释放工作簿集合对象
            delete workbooks;

        }
        else {
            qDebug() << "Failed to open workbook:" << filePath;
        }

        // 退出Excel应用程序
        excel->dynamicCall("Quit()");

        // 释放Excel对象
        delete excel;

    }
    else {
        qDebug() << "Failed to create Excel application object";
    }

}
