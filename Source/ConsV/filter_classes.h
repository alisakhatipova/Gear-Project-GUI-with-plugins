#ifndef FILTER_CLASSES
#define FILTER_CLASSES

using std::cerr;
using std::tuple;
using std::tie;
using std::make_tuple;

class Binarization
{
public:
    tuple<uint, uint, uint> operator () (const Image &m) const
    {
        uint r, g, b;
        tie(r, g, b) = m(0, 0);
        if (0.299 * r + 0.587 * g + 0.114 * b > 55.0)
            return make_tuple(255, 255, 255);
        else
            return make_tuple(0, 0, 0);
    }
    static const int radius = 0;
};


class MedianFilter
{
public:
    tuple<uint, uint, uint> operator () (const Image &m) const
    {
        uint size = 2 * radius + 1;
        uint r, g, b, num_b = 0, num_w = 0;
        for (uint i = 0; i < size; ++i) {
            for (uint j = 0; j < size; ++j) {
                tie(r, g, b) = m(i, j);
                if (r == 255)
                    num_w++;
                else
                    num_b++;
            }
        }
        if (num_w > num_b)
                return make_tuple(255, 255, 255);
        else
            return make_tuple(0, 0, 0);

    }
    static const int radius = 1;
};

#endif // FILTER_CLASSES

