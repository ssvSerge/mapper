#ifndef __OSM_IDX_TOOLS_H__

#define __OSM_IDX_TOOLS_H__

#include <iostream>
#include <string>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <list>
#include <deque>
#include <sstream>
#include <iomanip>
#include <functional> 
#include <fstream>


using namespace std;

#define GEO_COORDINATE_ERROR                    (-1000)
#define GEO_ID_ERROR                            (-1)

#define ARRAY_SIZE(x)                           ( sizeof(x) / sizeof(x[0]) )
#define SET_FALSE(var)                          { var=false; cout << "Error ar: " << __LINE__ << endl; }
#define RET_FALSE                               { cout << "Error at: " << __LINE__ << endl; return false; }


bool LoadConfigFile (string cfg_file_name);

#endif
