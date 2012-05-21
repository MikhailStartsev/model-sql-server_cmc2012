#ifndef SERVER_H
#define	SERVER_H

#include <sstream>
#include <limits>
#include <fstream>
#include <iostream>

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "database.h"
#include "errors.h"

using namespace DataBase;        

namespace SQLServer
{
	/* интерфейсный класс сервера
	 * 
	 * для создания полноценного наследникак нужно переоперделить в своём классе
	 * функцию void onAccept ( int* clientSock ), которая будет исполняться
	 * в отдельном потоке для каждого подключенного клиента.  
	 * Дескриптор сокета клиаента - параметр функции.
	 * */
	class IServer 
	{
		int port; // порт, на которм запускаем сервер
		
	protected:
		int sock_fd; // дескриптор сокета сервера
		sockaddr_in addr; // адрес сервера
		
		fstream log; // файловый поток (~файл) лога
		bool logging; // ключено ли логирование
		
	private:
		IServer(const IServer& serv);
		IServer& operator = (const IServer& serv); // запрещаем копирование   
		
	public:
		IServer(int port = 80, bool logging = true, const string& log_name = "log.txt");
		
		void start(); // запуск сервера (создание сокета, т.п.)
		
		virtual void onAccept(int clientSocket) = 0; // функция-обработчик клиена с дескриптором сокета @clientSocket
		/* промежуточная функция для запуска onAccept в номом потоке
		 * pthread_create требует, чтобы если функция-параметр является 
		 * функцией-членом, то была статической.
		 * 
		 * в @param - указатель на массив из 2 элементов:
		 * 1) IServer* - указатель на вызывающий сервер (его указатель this)
		 * 2) int* - указатель на дескриптор клиентского сокета
		 * 
		 * */
		static void* acceptor(void* param) 
		{
			void **arr = (void**)param;
			IServer* self = (IServer*)arr[0]; // в @param лежит this и int* client_sock
			self->onAccept(*((int*)arr[1]));
		}
		
		~IServer() 
		{
			log.close();
		}
	};
	
	/* сервер баз данных для моделного языка SQL */
	class SQL_Server : public IServer
	{
		DBMS db; // база данных, с которой оперирует сервер
		int bufsz; // размер буфера (для сокетов)
		int welcomeLen; // длина строки приветствия
		
	public:
		SQL_Server(int port = 1433, bool logging = true, const string& log_name = "log.txt");
		
		virtual void onAccept(int clientSocket);
	};
}
#endif	/* SERVER_H */

