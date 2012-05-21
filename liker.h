#pragma once

#include <string>
#include <sys/types.h>
#include <regex.h>
#include "errors.h"


using namespace std;

namespace Liker
{
	/* проверка на соответствие строки шаблону */
	bool isLike(const string& str, const string& pattern);
}