#ifndef _INIWRAPPER_H_
#define _INIWRAPPER_H_

#include <string>
#include "iniparser.h"

using std::string;

inline int getint(dictionary* ini, const char* sec, const char* name, int def)
{
	string tag = string(sec) + ":" + string(name);
	return iniparser_getint(ini, const_cast<char*>(tag.c_str()), def);
}

inline bool getbool(dictionary* ini, const char* sec, const char* name, bool def)
{
	string tag = string(sec) + ":" + string(name);
	return bool(iniparser_getboolean(ini, const_cast<char*>(tag.c_str()), def));
}

//inline float getfloat(dictionary* ini, const string& sec, const string& name, int def)
//{
//	string tag = sec + ":" + name;
//	return iniparser_getdouble(ini, tag.c_str(), def);
//}

inline string getstring(dictionary* ini, const char* sec, const char* name, const char* def)
{
	string tag = string(sec) + ":" + string(name);
	char* tmp = iniparser_getstring(ini, const_cast<char*>(tag.c_str()), NULL);
	if (tmp)
		return tmp;
	else
		return def;
}
#endif

