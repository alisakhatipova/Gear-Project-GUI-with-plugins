#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <vector>
#include <tuple>
#include <memory>
#include <queue>
#include <math.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;
using std::vector;
using std::tuple;
using std::tie;
using std::make_tuple;
using std::shared_ptr;
using std::queue;
#include "io.h"
#include "matrix.h"
#include "MyObject.h"

class Model : public QObject
{
   Q_OBJECT
public:
   explicit Model(QObject *parent = 0);
//   void add_viewer(Window &window);
signals:
   void log(QString str);
   void text(QString str);
   void img(Image);
public slots:
   void load_pic(QString Path);
   void repair_all();
private:
   Image in, dst_image;
   string image_path;
   tuple <uint, uint> distance_transform(Matrix<double> &g_func, Matrix<uint> &Segments, uint n, uint m, uint num, tuple <uint, uint> center_in);
   bool dist_betw_cogs(unsigned long long MinR, unsigned long long MaxR, Matrix<uint> &Segments, uint num, tuple <uint, uint> center_in);
   uint count_seg(Matrix<uint> &g_color, unsigned long long MinR, unsigned long long MaxR, Matrix<uint> &Segments, uint n, uint m,uint num, tuple <uint, uint> center_in);
   tuple<unsigned long long, unsigned long long> GetMinMaxR(tuple<int,int> &Center, Matrix<uint> &Segments, uint n, uint m, uint num);
   uint Segmentation(Matrix<uint> &Segments, uint n, uint m, Matrix<uint> &Color);
   void sent_time(time_t seconds, QString ev);
};

#endif // MODEL_H
