//abstract plugin
#include "io.h"
#include "matrix.h"
#include "MyObject.h"

using std::tuple;

class IPlugin
{
public:
  // returns string name of the plugin
	virtual char* stringType() = 0;
    virtual tuple<uint, uint, uint> operator () (const Image &m) const = 0;
    virtual void  Filt(Image &in, Image &dst) const= 0;
    static const int radius = 1;
};


class IFactory
{
public:
	virtual IPlugin* Create() = 0;
};

class IPluginManager
{
public:
  // registers plugins
	virtual void RegisterPlugin(IFactory* factory) = 0;
};


typedef void (*func_point)(IPluginManager *);
