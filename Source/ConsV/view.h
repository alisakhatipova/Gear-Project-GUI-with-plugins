#ifndef VIEW_H
#define VIEW_H

#include <QObject>
#include "io.h"
#include "matrix.h"
#include "MyObject.h"

using std::string;

class View : public QObject
{
    Q_OBJECT
public:
    explicit View(QObject *parent = 0);
    void start(string str1, string str2, string str3);
  signals:
     void got_image(QString path);
     void repair_it();
     void finish();
  public slots:
     void print_log(QString str);
     void print_text(QString str);
     void change_image(Image);
  private:
     string InImgPath, OutImgPath, OutFile;
};

#endif // VIEW_H
