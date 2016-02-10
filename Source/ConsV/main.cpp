#include <QCoreApplication>
#include <QString>
#include <QObject>
#include "view.h"
#include "model.h"
#include <QTimer>
#include <dlfcn.h>
#include <string>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
using namespace std;
#include "API.h"
#include "io.h"
#include "matrix.h"
#include "MyObject.h"

class PluginManager: public IPluginManager
{
    vector <IPlugin*> Plugins;
public:
    virtual void RegisterPlugin(IFactory* factory)
    {
        IPlugin* plugin = factory->Create();
        if (plugin)
            Plugins.push_back(plugin);
    }
   vector<IPlugin*> &GetPlugins()
   {
        return Plugins;
   }
};

PluginManager manager;

int findDLLs(const char *in_pluginsDir, vector<string> &out_file_names)
{
  DIR *dir;
  struct dirent *entry;
  dir = opendir(in_pluginsDir);
  while ((entry = readdir(dir)) != NULL){
        string st = entry->d_name;
        size_t found = st.find_last_of('.');
        string subs = st.substr(found + 1);
        if (!subs.compare("so")){
            cout<<entry->d_name<<endl;
            out_file_names.push_back(entry->d_name);
        }
  }
  closedir(dir);
  return out_file_names.size();
}

int LoadDLLs(const char *in_pluginsDir, const vector<string> &in_file_names,
  void **out_pLib, vector<string> &out_lib_names)
{
  auto files_num = in_file_names.size();
  char *buffer = new char[strlen(in_pluginsDir) + 1000];
  int k=0;
    for (size_t i=0; i<files_num; i++)
    {
        strcpy(buffer,in_pluginsDir);
        buffer = strcat(buffer, "/");
        buffer = strcat(buffer, in_file_names[i].c_str());
        out_pLib[k] = dlopen(buffer, RTLD_LAZY);
        if (out_pLib[k] != NULL)
        {
            out_lib_names.push_back(buffer);
            k++;
        }
        else
        {
            cerr << "Error loading .so" << endl;
        }
    }
  delete [] buffer;
  return k;
}

void LoadPlugins(void **in_pLib, const vector<string> &in_pLibNames)
{
  auto fLength = in_pLibNames.size();
    for (uint i=0; i<fLength; i++)//free libraries
    {
        func_point function = (func_point)dlsym(in_pLib[i], "registerPlugins");
        if (function == NULL)
            cout << "Library " << in_pLibNames[i] << " is not a plugin" << endl;
        else
        {
            cout << "Library " << in_pLibNames[i] << " is a plugin" << endl;
            function(&manager);
        }
    }
}

void UserOperation(vector<IPlugin*> &plugList, char *img_in, char *img_out)
{
    if (plugList.size() > 0)
    {
        cout << "Choose plugins" << endl;
        for (uint i=0; i<plugList.size(); i++)
            cout << '[' << i << ']' <<  (plugList[i])->stringType() << endl;

        int pluginIndex = -1;
        while (pluginIndex>=plugList.size()|| pluginIndex < 0)
        {
            cin >> pluginIndex;
            if (pluginIndex>=plugList.size()|| pluginIndex < 0)
            {
                cerr << "Invalid input: out of range " << "0-" << plugList.size() << endl;
            }
        }
        Image in = load_image(img_in);
        Image dst = in.deep_copy();
        IPlugin *pl = plugList[pluginIndex];
        pl->Filt(in, dst);
        save_image(dst, img_out);
        cout << "Image is saved" << endl;
    }

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    View view;
    Model model;
    QObject::connect(&view, SIGNAL(got_image(QString)), &model, SLOT(load_pic(QString)));
    QObject::connect(&view, SIGNAL(repair_it()), &model, SLOT(repair_all()));
    QObject::connect(&model, SIGNAL(log(QString)), &view, SLOT(print_log(QString)));
    QObject::connect(&model, SIGNAL(text(QString)), &view, SLOT(print_text(QString)));
    QObject::connect(&model, SIGNAL(img(Image)), &view, SLOT(change_image(Image)));
   // QObject::connect(&view, SIGNAL(finish()), &a, SLOT(quit()));
    if (argc == 5){
        char pluginsDir[] = "plugins";
        char *tmpStr = new char [strlen(pluginsDir) + 1000];

        cout << "File name is:\t" << pluginsDir << endl;

        vector<string> files;
        vector<string> pLibNames;

      // find all dlls in a folder specified by the pluginDir
      int fLength = findDLLs(pluginsDir, files);
        cout << fLength << " .dll files was found in dir " << pluginsDir <<endl;

        // array of dll handles
      //  void **pLib = new (void*)[fLength];
      //  void **pLib = operator new(sizeof(void*) * fLength);
       void **pLib = (void**)malloc(sizeof(void*)*fLength);
      fLength = LoadDLLs(pluginsDir, files, pLib, pLibNames);
      LoadPlugins(pLib, pLibNames);
      UserOperation(manager.GetPlugins(), argv[1], argv[2]);
        for (int i=0; i<fLength; i++)//free libraries
        {
            int success = dlclose(pLib[i]);
            if (success)
                cerr << "FreeLibrary error";
        }
        free(pLib);
        delete[] tmpStr;
        return 0;
    }
    else{
    view.start(argv[1], argv[2], argv[3]);
    QTimer::singleShot(1, &a, SLOT(quit())); //correct it
    return a.exec();
    }
}

