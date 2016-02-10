#include "view.h"
#include <iostream>
#include <QString>
#include <fstream>
#include <QObject>
#include <QCoreApplication>

using std::string;
using std::cout;
using std::ofstream;
View::View(QObject *parent) : QObject(parent)
{

}

void View::print_log(QString str){
    std::cout << qPrintable(str);
}

void View::print_text(QString str){
     ofstream fout(OutFile);
     fout << qPrintable(str);
     //emit finish();
     //QCoreApplication::exit(0);
}
void View::start(string str1, string str2, string str3){
     InImgPath = str1;
     OutImgPath = str2;
     OutFile = str3;
     emit got_image(InImgPath.c_str());
     emit repair_it();
}

void View::change_image(Image dst_image){
     save_image(dst_image, OutImgPath.c_str());
}
