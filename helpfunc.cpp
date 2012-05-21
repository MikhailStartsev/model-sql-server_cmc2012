#include "database.h"

string DataBase::toUpper(const string& str)
{
	string result = str;
	int n = result.length();
	for (int i=0; i<n; ++i)
		result[i] = toupper(result[i]);
	
	return result;
}

