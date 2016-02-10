//.dll
#include <vector>
#include <tuple>
#include <algorithm>
#include "API.h"
#include "io.h"
#include "matrix.h"
#include "MyObject.h"
using std::tuple;
using std::vector;
extern "C" {
class MedianF: public IPlugin
{
public:
	virtual tuple<uint, uint, uint> operator () (const Image &m) const
	{
        return std::make_tuple(255, 255, 255);
	}
    virtual void Filt(Image &in, Image &dst) const{
        int radius = 1;
        uint n_rows = in.n_rows;
        uint n_cols = in.n_cols;
        const auto size = 2 * radius + 1;
        const auto start_i = radius;
        const auto end_i = n_rows - radius;
        const auto start_j = radius;
        const auto end_j = n_cols - radius;
        for (uint i = start_i; i < end_i; ++i) {
            for (uint j = start_j; j < end_j; ++j) {
                auto neighbourhood = in.submatrix(i - radius, j - radius, size, size);
                uint r, g, b;
                vector<uint> rr, gg, bb;
                for (uint k = 0; k < size; ++k) {
                    for (uint h = 0; h < size; ++h) {
                        std::tie(r, g, b) = neighbourhood(k, h);
                        rr.push_back(r);
                        gg.push_back(g);
                        bb.push_back(b);
                    }
                }
                std::sort(rr.begin(), rr.end());
                std::sort(gg.begin(), gg.end());
                std::sort(bb.begin(), bb.end());
                dst(i, j) = std::make_tuple(rr[5], gg[5], bb[5]);
            }
        }
    }

	virtual char * stringType()
	{
		char * ret = (char*)("Median Filter");
		return ret;
	}

};


class GaussianF: public IPlugin
{
public:
	virtual tuple<uint, uint, uint> operator () (const Image &m) const
	{
		return std::make_tuple(0, 0, 0);
	}
    virtual void Filt(Image &in, Image &dst) const{
        int radius = 2;
        uint n_rows = in.n_rows;
        uint n_cols = in.n_cols;
        const auto size = 2 * radius + 1;
        const auto start_i = radius;
        const auto end_i = n_rows - radius;
        const auto start_j = radius;
        const auto end_j = n_cols - radius;
        for (uint i = start_i; i < end_i; ++i) {
            for (uint j = start_j; j < end_j; ++j) {
                auto neighbourhood = in.submatrix(i - radius, j - radius, size, size);
                uint r, g, b, sumr = 0, sumg = 0, sumb = 0;
                double smth[5][5]={
                    {0.003, 0.013, 0.022, 0.013, 0.003},
                    {0.013, 0.059, 0.097, 0.059, 0.013},
                     {0.022, 0.097, 0.159, 0.097, 0.022},
                    {0.013, 0.059, 0.097, 0.059, 0.013},
                    {0.003, 0.013, 0.022, 0.013, 0.003}
                };
                for (uint k = 0; k <  size; ++k) {
                    for (uint h = 0; h < size; ++h) {
                        std::tie(r, g, b) = neighbourhood(k, h);
                        sumr += r*smth[k][h];
                        sumg += g*smth[k][h];
                        sumb += b*smth[k][h];
                    }
                }
                dst(i, j) = std::make_tuple(sumr, sumg, sumb);
            }
        }
    }
	virtual char * stringType()
	{
		char *ret = (char*)("Gaussian Filter");
		return ret;
	}
};

class MedianFactory: public IFactory
{
	vector<IPlugin*> pInstances;
public:
	virtual IPlugin* Create()
	{
		IPlugin* ret = new  MedianF;
		pInstances.push_back(ret);
		return ret;
	}
	/*~MedianFactory()
	{
		for (int i=0; i<pInstances.size(); i++)
			delete pInstances[i];
	} */
};


class GaussianFactory: public IFactory
{
	vector<IPlugin*> pInstances;
public:
	virtual IPlugin* Create()
	{
		IPlugin* ret = new GaussianF;
		pInstances.push_back(ret);
		return ret;
	}
	/*~GaussianFactory()
	{
		for (int i=0; i<pInstances.size(); i++)
			delete pInstances[i];
	} */
};

// singleton factories producing plugins
MedianFactory Factory1;
GaussianFactory Factory2;

// dll entry point. Registers all plugins realized in the dll
void registerPlugins(IPluginManager * pluginManager)
{
	if (pluginManager != 0)
	{
		pluginManager->RegisterPlugin(&Factory1);
		pluginManager->RegisterPlugin(&Factory2);
	}
}

}
// function that called then dll is attached and detached.
// Allows performing pre- and post processing.
/*BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch( ul_reason_for_call ) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
    //do nothing
    break;
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    //release memory in destructor
    ;
  }
  return TRUE;
} */
