#include "liker.h"

bool Liker::isLike(const string& str, const string& pattern)
{
	string regex_str = "^" + pattern + "$"; // точное соответствиеы
	
	int n = regex_str.length();
	
	for (int i=0; i<n; ++i)
	{
		if (regex_str[i] == '%')
			regex_str.replace(i, 1, ".*");
		if (regex_str[i] == '_')
			regex_str[i] = '.';
	}
	
	regex_t regex;
	if (regcomp(&regex, regex_str.c_str(),0))
		throw dberrors::CompilationError("Regular expression is wrong-formated.");
	
	if (regexec(&regex, str.c_str(), 0, NULL, 0) == 0 )
		return true;
	
	return false;
	
	
}