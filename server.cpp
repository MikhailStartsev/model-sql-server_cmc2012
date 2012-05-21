#include "server.h"
using namespace SQLServer;

char welcomeMsg[] = "Welcome to SQL server!\n\nYou can now enter commands to manage databases.\n\n";

IServer::IServer(int port, bool logging, const string& log_name) : port(port), logging(logging)
{
	if (logging)
	{
		log.open(log_name.c_str(), fstream::out | fstream::app);
		
		if (log.fail())
			throw dberrors::ServerError("Unable to append to log file '" + log_name + "'. Try starting without logging or change log filename.");
	}
}

void IServer::start()
{
	// запуск "внутренней соктной части" сервера
	
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0)
	{
		throw dberrors::ServerError("Unable to create a socket.");
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sock_fd, (sockaddr*)&addr, sizeof(addr)) != 0)
	{
		throw dberrors::ServerError("Unable to bind the socket.");
	}
	
	if (listen(sock_fd, SOMAXCONN) != 0)
	{
		throw dberrors::ServerError("Unable to start listening the socket.");
	}
	
	time_t t = time(NULL);
	if (logging)
	{
		log << "SERVER STARTED at " << asctime(localtime(&t)) << " on port " << port << endl << endl;
		log.flush();
	}
	cout << "SERVER STARTED at " << asctime(localtime(&t)) << " on port " << port << endl << endl;
	
	
	// работа с клиентами
	
	int client_sock;
	pthread_t client_thread;
	
	while (true)
	{
		client_sock = accept(sock_fd, NULL, NULL);
		if (client_sock >= 0)
		{
			time_t t = time(NULL);
			
			cout << asctime(localtime(&t)) << ": connection accepted from socket " << client_sock << ".\n";
			if (logging)
				log << asctime(localtime(&t)) << ": connection accepted from socket " << client_sock << ".\n";
			void* param[] = {this, &client_sock};
			pthread_create(&client_thread, NULL, &SQLServer::IServer::acceptor, (void*)param);
			
		}
		sleep(1);
	}
}


SQL_Server::SQL_Server(int port, bool logging, const string& log_name) : IServer(port, logging, log_name)
{
	welcomeLen = strlen(welcomeMsg);
	bufsz = 32768;    
	
	if (logging)
		db.setLogFile(log);
	
}

void SQL_Server::onAccept(int sock)
{
	send(sock, welcomeMsg, welcomeLen, 0);
	
	char buf[bufsz];
	
	
	while (true) // работаем с клиентом
	{
		bzero(buf, bufsz);
		if (recv(sock, buf, bufsz-1, 0) <= 0)
		{
			// клиент отключен или ошибка
			break;
		}
		
		time_t t = time(NULL);
		cout << asctime(localtime(&t)) << ": query from socket " << sock << ":" << endl << buf << endl;
		if (logging)
		{
			log << asctime(localtime(&t)) << ": query from socket " << sock << ":" << endl << buf << endl;
			log.flush();
		}
		
		stringstream result;
		db.proceedQuery(buf, result);
		
		
		send(sock, result.str().c_str(), result.str().length(), 0);
		cout << asctime(localtime(&t)) << ": response to " << sock << ":" << endl << result.str() << endl;
		t = time(NULL);
		if (logging)
		{
			log << asctime(localtime(&t)) << ": response to " << sock << ":" << endl << result.str() << endl;
			log.flush();
		}
		
		if (result.str() == "EXIT")
		{
			break;
		}
		
	}
	time_t t = time(NULL);
	
	cout << asctime(localtime(&t)) << ": client " << sock << " disconnected.\n\n";
	if (logging)
	{
		log << asctime(localtime(&t)) << ": client " << sock << " disconnected.\n\n";
		log.flush();
	}
}

