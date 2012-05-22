#include "server.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <string>

using namespace SQLServer;
using namespace std;

int main(int argc, char* argv[])
{
	int port = 1433;
	
	bool nextPort = false;
	bool portSet = false;
	
	bool nextLog = false;
	bool logSet = false;
	bool logging = false;
	string logname = "log.txt";
	char usage[] = "Usege: sqlserver [options]\n"
	"Options:\n"
	"-h | --help - show this help message\n"
	"-p <port> | --port <port> - start server at port <port>\n"
	"-l | --log - enable server logging; default log file is log.txt\n"
	"-lf <filename> | --logfile <filename> | -o <filename>| --out <filename> - set log filename as <filename>; automatically enables logging\n";
	
	if (argc >= 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
	{
		cout << usage;
		return 0;
	}
	
	for (int i = 1; i < argc; ++i)
	{        
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			if (nextLog)
			{
				cout << "Wrong usage.\n" << usage;
				return 1;
			}
			continue;
		}
		if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--log"))
		{
			if (nextLog)
			{
				cout << "Wrong usage.\n" << usage;
				return 1;
			}
			logging = true;
			continue;
		}
		if (!strcmp(argv[i], "-lf") || !strcmp(argv[i], "--logfile") ||
                        !strcmp(argv[i], "-o") || !strcmp(argv[i], "--out"))
		{
			if (logSet)
			{
				cout << "Log filename cannot be set twice.\n";
				return 1;
			}
			if (nextLog)
			{
				cout << "Wrong usage.\n" << usage;
				return 1;
			}
			nextLog = true;
			logging = true;
			continue;
		}
		
		if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
		{
			if (portSet)
			{
				cout << "Port cannot be set twice.\n";
				return 1;
			}
			
			if (nextPort)
			{
				cout << "Wrong usage.\n" << usage;
				return 1;
			}
			
			port = -1;
			nextPort = true;
			continue;
		}
		
		if (nextLog)
		{
			logname = argv[i];
			nextLog = false;
			logSet = true;
			continue;
		}
		else if (nextPort)
		{
			port = atoi(argv[i]);
			nextPort = false;
			continue;
		}
		
		else
		{
			cout << "Wrong usage.\n" << usage;
			return 1;
		}
	}
	
	if (nextLog || nextPort)
	{
		cout << "Wrong usage.\n" << usage;
		return 1;
	}
	
	if (port < 0 || port > (1<<16))
	{
		cout << "Wrong port value '" << port << "'. <port> should be a positive integer <= " << (1<<16) << ".\n";
		return 1;
	}
	
	try
	{
		SQL_Server server(port, logging, logname);
		server.start();
	}
	catch (Error& ex)
	{
		cout << ex.message << endl;
	}
	
	return 0;
}