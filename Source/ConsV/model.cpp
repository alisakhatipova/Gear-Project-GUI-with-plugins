#include "model.h"
#include <QString>
#include <iostream>
#include <math.h>
#include"filter_classes.h"
#include <ctime>
#include <QObject>

#define MinAxisSquare 10
#define INF 100000000

Model::Model(QObject *parent) : QObject(parent)
{

}


void Model::load_pic(QString path){
    image_path = qPrintable(path);
    in = load_image(image_path.c_str());

}

uint Model::Segmentation(Matrix<uint> &Segments, uint n, uint m, Matrix<uint> &Color){
    Matrix<uint> used(n, m);
    queue <tuple<uint, uint>> Coord;
    for (uint i = 0; i < n; ++i)
        for(uint j = 0; j < m; ++j)
            used(i,j) = 0;
    uint tag = 0;
    for (uint k = 0; k < n; ++k)
        for (uint h = 0; h < m; ++h){
            if (!used(k, h)){
                Coord.push(make_tuple(k, h));
                used(k, h) = 1;
                while (!Coord.empty()){
                    uint i, j;
                    tie(i, j) = Coord.front();
                    Coord.pop();
                    Segments(i, j) = tag;
                    if ((i > 0) && (!used(i - 1, j)) && (Color(i,j) == Color(i - 1, j))){
                        Coord.push(make_tuple(i - 1, j));
                        used(i - 1, j) = 1;
                    }
                    if ((i + 1 < n) && (!used(i + 1, j)) && (Color(i,j) == Color(i + 1, j))){
                        Coord.push(make_tuple(i + 1, j));
                        used(i + 1, j) = 1;
                    }
                    if ((j > 0) && (!used(i, j - 1)) && (Color(i,j) == Color(i, j - 1))){
                        Coord.push(make_tuple(i, j - 1));
                        used(i, j - 1) = 1;
                    }
                    if ((j + 1 < m) && (!used(i, j + 1)) && (Color(i,j) == Color(i, j + 1))){
                        Coord.push(make_tuple(i, j + 1));
                        used(i, j + 1) = 1;
                    }
                }
                tag++;
            }
        }
    return tag;
}

tuple<unsigned long long, unsigned long long> Model::GetMinMaxR(tuple<int,int> &Center, Matrix<uint> &Segments, uint n, uint m, uint num){
    unsigned long long MaxR = 0, MinR = -1;
    int x, y;
    tie(y, x) = Center;
    for (uint i = 0; i < n; ++i)
        for (uint j = 0; j < m; ++j){
            unsigned long long dist = (x - i) * (x - i) + (y - j) * (y - j);
            if ((Segments(i,j) == num) && (dist > MaxR) )
                MaxR = dist;
            if ((Segments(i,j) != num) && (dist < MinR) )
                MinR =  dist;
        }
    return make_tuple(MinR, MaxR);
}

uint Model::count_seg(Matrix<uint> &g_color, unsigned long long MinR, unsigned long long MaxR, Matrix<uint> &Segments, uint n, uint m,uint num, tuple <uint, uint> center_in){
    uint h = g_color.n_rows, w = g_color.n_cols;
    uint center_x = h / 2, center_y = w/2, in_x, in_y;
    tie(in_y, in_x) = center_in;
    for (uint i = 0; i < h; ++i)
        for (uint j = 0; j < w; ++j)
            g_color(i, j) = 0;
    for (uint i = 0; i < n; ++i)
        for (uint j = 0; j < m; ++j)
            if ((Segments(i, j) == num) && ((i - in_x) * (i - in_x) + (j - in_y) * (j - in_y) > MinR/4 + MaxR/4 + sqrt(MinR) * sqrt(MaxR)/2)){
                g_color(center_x + (i - in_x), center_y + (j - in_y)) = 1;
            }
    /*Image tryit(h, w);
    for (uint i = 0; i < h; ++i)
        for (uint j = 0; j < w; ++j)
           if (g_color(i, j) == 1)
                tryit(i, j) = make_tuple(255, 255, 255);
           else tryit(i, j) = make_tuple(0,0,0);
    save_image(tryit, "tryit.bmp"); */
    Matrix<uint> Segs(h, w);
    uint size = Segmentation(Segs, h, w, g_color);
    vector<uint> sqr(size);
    for (uint i = 0; i < h; ++i)
        for (uint j = 0; j < w; ++j)
            ++sqr[Segs(i, j)];
    uint kol = size;
    for (uint i = 0; i < size; ++i)
        if (sqr[i] < 10) --kol;
    return kol;

}

bool Model::dist_betw_cogs(unsigned long long MinR, unsigned long long MaxR, Matrix<uint> &Segments, uint num, tuple <uint, uint> center_in){
    // использован алгоритм Брезенхэма
    int radius = (sqrt(MaxR) + sqrt(MinR))/2;
    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;
    int x0, y0;
    tie(y0, x0) = center_in;
    vector <uint> quat1, quat2, quat3, quat4;
    while(y >= 0) {
        if (Segments(x0 + x, y0 + y) == num)
            quat1.push_back(1);
        else
            quat1.push_back(0);
        if (Segments(x0 + x, y0 - y) == num)
            quat2.push_back(1);
        else
            quat2.push_back(0);
        if (Segments(x0 - x, y0 + y) == num)
            quat4.push_back(1);
        else
            quat4.push_back(0);
        if (Segments(x0 - x, y0 - y) == num)
            quat3.push_back(1);
        else
            quat3.push_back(0);
        error = 2 * (delta + y) - 1;
        if(delta < 0 && error <= 0) {
            ++x;
            delta += 2 * x + 1;
            continue;
        }
        error = 2 * (delta - x) - 1;
        if(delta > 0 && error > 0) {
            --y;
            delta += 1 - 2 * y;
            continue;
        }
        ++x;
        delta += 2 * (x - y);
        --y;
    }
    uint size = quat1.size();
    for (int i = size - 1; i >= 0; --i)
        quat1.push_back(quat2[i]);
    for (uint i = 0; i < size; ++i)
        quat1.push_back(quat3[i]);
    for (int i = size - 1; i >= 0; --i)
        quat1.push_back(quat4[i]);
    uint len_black = 0;
    while (quat1.back() == 0){
        ++len_black;
        quat1.pop_back();
    }
    size = quat1.size();
    uint j = 0;
    vector<uint>blacks;
    while (j < size){
        if (quat1[j] == 0)
            ++len_black;
        else {
            if (len_black > 3){
                blacks.push_back(len_black);
                len_black = 0;
            }
        }
        ++j;
    }
    size = blacks.size();
    for (uint i = 0; i < size - 1; ++i)
        if (abs(blacks[i] - blacks[i  + 1]) > 25)
            return true;
    return false;
}

tuple <uint, uint> Model::distance_transform(Matrix<double> &g_func, Matrix<uint> &Segments, uint n, uint m, uint num, tuple <uint, uint> center_in) {
    uint h = g_func.n_rows, w = g_func.n_cols;
    uint center_x = h / 2, center_y = w/2, in_x, in_y;
    tie(in_y, in_x) = center_in;
    for (uint i = 0; i < h; ++i)
        for (uint j = 0; j < w; ++j)
            g_func(i, j) = 0;
    for (uint i = 0; i < n; ++i)
        for (uint j = 0; j < m; ++j)
            if ((Segments(i, j) == num))
                g_func(center_x + (i - in_x), center_y + (j - in_y)) = INF;
    for (uint i = 0; i < h; ++i){
        uint k = 0;
        vector<double> z;
        vector<uint> v;
        v.resize(w + 2);
        z.resize(w + 2);
        v[0] = 0;
        z[0] = -INF;
        z[1] = INF;
        uint q = 1;
        while (q < w){
            double s = ((g_func(i, q) + q * q) - (g_func(i, v[k]) + v[k] * v[k]))/(2 * q - 2 * v[k]);
            if (s <= z[k]){
                k--;
                continue;
            }
            else{
                k++;
                v[k] = q;
                z[k] = s;
                z[k + 1] = INF;
            }
            ++q;
        }
        k = 0;
        for (q = 0; q < w; ++q){
            while(z[k + 1] < q)
                k++;
            g_func(i, q) = (q - v[k]) * (q - v[k]) + g_func(i, v[k]);
        }
    }

    for (uint i = 0; i < w; ++i){
        uint k = 0;
        vector<double> z;
        vector<uint> v;
        v.resize(h + 2);
        z.resize(h + 2);
        v[0] = 0;
        z[0] = -INF;
        z[1] = INF;
        uint q = 1;
        while (q < h){
            double s = ((g_func(q, i) + q * q) - (g_func(v[k], i) + v[k] * v[k]))/(2 * q - 2 * v[k]);
            if (s <= z[k]){
                k--;
                continue;
            }
            else{
                k++;
                v[k] = q;
                z[k] = s;
                z[k + 1] = INF;
            }
            ++q;
        }
        k = 0;
        for (q = 0; q < h; ++q){
            while(z[k + 1] < q)
                k++;
            g_func(q, i) = (q - v[k]) * (q - v[k]) + g_func(v[k], i);
        }
    }
        double max_dist = 0, imax = 0, jmax = 0;

        for (uint i = 0; i < h; ++i)
            for (uint j = 0; j < w; ++j)
                if (g_func(i, j) > max_dist){
                    max_dist = g_func(i, j);
                    imax = i;
                    jmax = j;
                }
       // tryit(imax, jmax) = make_tuple(255, 0, 0);
        //  save_image(tryit, "tryit.bmp");
        return make_tuple(jmax - center_y + in_y, imax - center_x + in_x);

}

void Model::sent_time(time_t seconds, QString ev){
    char buffer[80];
    char format[] = "%d, %Y %I:%M:%S";
    tm* timeinfo = localtime(&seconds);
    strftime(buffer, 80, format, timeinfo);
    QString event = ev + " " + buffer + "\n";
    emit log(event);
}

void Model::repair_all(){
    uint AxisCenter_x = 0, AxisCenter_y = 0, AxisSeg = 0, BackgrSeg = 0;
    Image img1 = in.unary_map(Binarization());
    time_t seconds = time(NULL);
    sent_time(seconds, "Image binarized");
    Image img2 = img1.unary_map(MedianFilter());
    seconds = time(NULL);
    sent_time(seconds, "Median filter applied");
   // Image dst_image;
    uint n = img2.n_rows, m = img2.n_cols;
    //create binary matrix
    Matrix<uint> Color(n, m);
    for (uint i = 0; i < n; ++i)
        for (uint j = 0; j < m; ++j){
            uint r, g, b;
            tie (r, g, b) = img2(i, j);
            if (r == 255)
                Color(i, j) = 1; //white
            else Color(i, j) = 0; //black
        }
    //create segmentation
    Matrix<uint> Segments(n, m);
    uint num_of_seg = Segmentation(Segments, n, m, Color);
    seconds = time(NULL);
    sent_time(seconds, "Connected components marked");
    vector<uint> Sqr(num_of_seg);
    vector<tuple<int, int>> Mass_center(num_of_seg);
    for (uint i = 0; i < num_of_seg; ++i){
        Sqr[i] = 0;
        Mass_center[i] = make_tuple(0, 0);
    }
    for (uint i = 0; i < n; ++i)
        for (uint j = 0; j < m; ++j){
            Sqr[Segments(i,j)]++;
            uint k, h;
            tie(k, h) = Mass_center[Segments(i,j)];
            Mass_center[Segments(i,j)] = make_tuple(k + i, h + j);
        }
    uint MaxAxisSquare = 1500, MaxGearSquare = 0, MaxSquare = 0;
    Image in_1 = in.deep_copy();
    for (uint i = 0; i < num_of_seg; ++i){
        if ((Sqr[i] > MinAxisSquare) && (Sqr[i] < MaxAxisSquare))
            MaxAxisSquare = Sqr[i];
        if (Sqr[i] > MaxSquare)
            MaxSquare = Sqr[i];
    }
    for (uint i = 0; i < num_of_seg; ++i)
        if ((Sqr[i] > MaxGearSquare) && (Sqr[i] < MaxSquare))
            MaxGearSquare = Sqr[i];

    auto object_array = vector<shared_ptr<IObject>>();
    for (uint i = 0; i < num_of_seg; ++i){
        uint k, h;
        tie(k, h) = Mass_center[i];
        Mass_center[i] = make_tuple(round(h * 1.f/Sqr[i]), round(k * 1.f/Sqr[i]));
        if (Sqr[i] == MaxAxisSquare){
            object_array.push_back(shared_ptr<IObject>(new Axis(Mass_center[i])));
            AxisSeg = i;
            tie(AxisCenter_y, AxisCenter_x) = Mass_center[i];
            seconds = time(NULL);
            sent_time(seconds, "Axis found");
        }
        else if ((Sqr[i] > MaxAxisSquare) && (Sqr[i] <= MaxGearSquare)){
            unsigned long long MinR, MaxR;
            tie(MinR, MaxR) = GetMinMaxR(Mass_center[i], Segments, n, m, i);
            Matrix<uint> Gear_seg(round(sqrt(MaxR)) * 2 + 40, round(sqrt(MaxR)) * 2 + 40);
            uint cogs_num = count_seg(Gear_seg, MinR, MaxR, Segments, n, m, i, Mass_center[i]);
            Matrix<double> Gear_dist(round(sqrt(MaxR)) * 3 + 40, round(sqrt(MaxR)) * 3 + 40);
            tuple <uint, uint> new_center = distance_transform(Gear_dist, Segments, n, m, i, Mass_center[i]);
            uint newx, newy, oldx, oldy;
            tie(newx, newy) = new_center;
            tie(oldx, oldy) = Mass_center[i];
            bool is_broken = false;
            if ((abs(newx - oldx) > 2)|| (abs(newy - oldy) > 2)){
                is_broken = true;
                seconds = time(NULL);
                sent_time(seconds, "Broken gear found");
                Mass_center[i] = new_center;
                tie(MinR, MaxR) = GetMinMaxR(Mass_center[i], Segments, n, m, i);
                Matrix<uint> Gear_seg_1(round(sqrt(MaxR)) * 2 + 40, round(sqrt(MaxR)) * 2 + 40);
                cogs_num = count_seg(Gear_seg_1, MinR, MaxR, Segments, n, m, i, Mass_center[i]);
                tie(AxisCenter_y, AxisCenter_x) = Mass_center[i];
                AxisSeg = i;
                for (uint q = 0; q < n; ++q)
                    for (uint j = 0; j < m; ++j){
                        if (Segments(q, j) == i)
                            in_1(q, j) = make_tuple(0, 0, 0);
                    }
            }
            object_array.push_back(shared_ptr<IObject>(new Gear(Mass_center[i], sqrt(MinR), sqrt(MaxR), is_broken, cogs_num - 1)));
        }
        else
            BackgrSeg = i;

    }

    if ((AxisCenter_x == 0) && (AxisCenter_y == 0))
         for (const auto &obj : object_array)
            if (obj->IsGear()){
                double minrad = static_cast<Gear*>(obj.get())->min_r;
                double maxrad = static_cast<Gear*>(obj.get())->max_r;
                tuple<int, int> Cent = obj->location;
                uint numb = obj->num_in_seg_array;
                bool br = dist_betw_cogs(minrad*minrad, maxrad*maxrad, Segments,numb, Cent) ;
                if (br){
                    static_cast<Gear*>(obj.get())->is_broken = true;
                    tie(AxisCenter_y, AxisCenter_x) = Cent;
                    AxisSeg = numb;
                    for (uint q = 0; q < n; ++q)
                        for (uint j = 0; j < m; ++j){
                            if (Segments(q, j) == numb)
                                in_1(q, j) = make_tuple(0, 0, 0);
                        }
                }
            }
    seconds = time(NULL);
    sent_time(seconds, "All cogs counted");
    // check three gears
    vector<string> Patterns = {"1", "2", "3"};
    int result_idx = 0;
    uint max_sqr_gear = 0;
    for (int i = 0; i < 4; ++i) image_path.pop_back(); //delete ".bmp"
    for (int idx = 0; idx < 3; ++idx){
        string ImPath = image_path + "_" + Patterns[idx] + ".bmp";
        Image GearImg = load_image(ImPath.c_str());
        Image GearImg2 = GearImg.unary_map(Binarization());
        uint h = GearImg.n_rows, w = GearImg.n_cols, sum_x = 0, sum_y = 0, sqr_gear = 0;
        for (uint i = 0; i < h; ++i)
            for (uint j = 0; j < w; ++j){
                int r, g, b;
                tie (r, g, b) = GearImg2(i, j);
                if (r == 255){
                    sum_x += i; sum_y +=j; sqr_gear++;
                }
            }
        sum_x = round((sum_x * 1.f)/sqr_gear); sum_y= round((sum_y * 1.f)/sqr_gear);
        Image Var = in_1.deep_copy();
        int gear_is_ok = 1;
        for (uint i = 0; i < h; ++i){
            if (!gear_is_ok)
                break;
            for (uint j = 0; j < w; ++j){
                int r, g, b;
                tie (r, g, b) = GearImg2(i, j);
                if (r == 255){
                    int coord_x = AxisCenter_x + (i - sum_x), coord_y = AxisCenter_y + (j - sum_y);
                    if ((coord_x >= 0) && (abs(coord_x) < (long long)n) && (coord_y >= 0) && (abs(coord_y) < (long long)m) &&((Segments(coord_x, coord_y) == AxisSeg) || (Segments(coord_x, coord_y) == BackgrSeg)))
                        Var(coord_x, coord_y) = make_tuple(0, 255, 0);
                    else {
                        gear_is_ok = 0;
                        break;
                    }
                }
            }
        }
        if ((gear_is_ok) && (sqr_gear > max_sqr_gear)) {
            max_sqr_gear = sqr_gear;
            dst_image = Var.deep_copy();
            result_idx = idx + 1;
        }
    }
    seconds = time(NULL);
    sent_time(seconds, "Mechanism repaired");
    //save_image(dst_image, "qwer.bmp");
    emit img(dst_image);
    string result = std::to_string(result_idx);
    result += "\n" + std::to_string(object_array.size()) + "\n";
    for (const auto &obj : object_array)
        result += obj->Write();
    emit (text(result.c_str()));
}

/* void Model::add_viewer(Window &window){
    QObject::connect(&window, SIGNAL( got_image(QString)), this, SLOT(load_pic(QString)));
    QObject::connect(window.repair_button, SIGNAL(clicked()), this, SLOT(repair_all()));
    QObject::connect(this, SIGNAL(log(QString)), &window, SLOT(print_log(QString)));
    QObject::connect(this, SIGNAL(text(QString)), &window, SLOT(print_text(QString)));
    QObject::connect(this, SIGNAL(img(QString)), &window, SLOT(change_image(QString)));
} */
