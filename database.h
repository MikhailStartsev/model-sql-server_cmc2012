/*
* 
* @@name - name из текущего класса
* @name - параметр текущей функции
* 
*/

#pragma once

#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <sstream>
#include <stack>
#include <vector>
#include <map>

#include <fstream>
#include <string>
#include <list>
#include <limits>

#include <cstring>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "errors.h"
#include "liker.h"

using namespace std;
using namespace dberrors;

namespace DataBase
{
        /* Классы, относящиеся к трансляции и исполнению только объявлены,
         * написать их вам предстоит самостоятельно.
         * 
         * В некоторых таких классах объявлены прототипы функций или функции
         * с пустым телом. Это сделано для того, чтобы проект компилировался.
         * 
         * В текущем состоянии при запуске проекта запускается сервер, принимает
         * запросы клиентов, отправляет их СУБД, но она отвечает пустой строкой.
         * 
         * Этот ответ отправляется клиенту (в качестве клиена используется 
         * telnet).
         * */
    
	/* тип поля */
	enum FieldType { Long, Text };
	/* расширение для файлов базы даннх */
	static const string dbFileExtention = ".db";
        
        enum TypeOfLex {Lex_Bool, Lex_Num, Lex_Str, Lex_Long, Lex_Text};
        class Lex 
        {
        public:
            TypeOfLex get_type(){}
            long long int get_value(){}
            string get_str_value(){}
        };
        class Poliz {};
        
        
        
	/* вспомогательные функции */
	
	/* перевод строки в верхний регистр */
	string toUpper(const string& str);
	/* проверка, является лексема типа @type - операцией */
	bool isOperation(TypeOfLex type); 
		
        
        
	// DBMS
	
	/* обёртка для класса int, при вызове конструктора с пустым набором 
	* формальных параметров возвращет false при вызову @@operator !
	* 
	* нужна для проверки наличия записи в map<..., IntWrap>, так как 
	* если в map::oiperator[] было передано значения, которого нет среди
	* ключей map, вызовется конструктор по умолчанию для класса 
	* значений map. 
	* 
	* вообще, можно обойтись без этого. у map есть специальная функция 
	* */
	struct IntWrap
	{
		int ind;	
		bool ok;

		IntWrap() : ok(false) {}
		IntWrap(int ind) : ind(ind), ok(true) {}

		bool operator !()
		{
			return !ok;
		}

		operator int() { return ind; }

		IntWrap& operator --() 
		{
			--ind;
			return *this;
		}
	};

	/* класс, описывающий внутреннюю структуру таблицы */
	struct TableStruct
	{
		int numFields; // число полей
		int numRecords; // число записей
		vector<string> names; // имена полей
		vector<FieldType> types; // типы соответствующих полей

		map<string, IntWrap> nameMap; /* ассоциативный массив. ставящий 
						* в соответствие имени поля поля 
						* его номер в массиве имён полей
						* */

		void getTableStructFromFile(fstream& file); // считывание структуры из файла

	};

	/* класс, реализующий мьютекс.
	* 
	* понадобится, чтобы блокировать таблицы при исполнении на них 
	* операций, меняющих их записи*/
	class Mutex
	{
		bool aval;
	public:
		/* блокирока мьютекса. 
		* если мьютекс блокирован, происходит ожидание разблокировки */
		void lock() 
		{ 
			while (!aval)
			{
				sleep(1);
			}
			aval = false;
		}
		/* разблоировка мьютека.
		* разблокировка происходит сразу.
		* суть в том, что если вызвать разблокировку для уже 
		* разблокированного мьютекса, произойдет ошибка.
		* */
		void unlock()
		{
			if (aval)
				throw MutexError();
			aval = true;
		}
		
		
		Mutex() : aval(true) {} // при создании мьютекс не заблокирован
		
	};

	/* интерфейсный класс поля тблицы.
	* 
	* для создания наследника необходимо переопределить методы 
	* @@read и @@write считывания/записи из/в поток 
	* */
	struct ITBField
	{
		FieldType type;
		string name;
		virtual void read(istream& in) = 0;
		virtual void write(ostream& out, bool endline = true) = 0;
	protected: // закрываем доступ к конструктору TBField извне (~абстракнтый класс)
		ITBField() {}
		ITBField(const ITBField&) {}
	};
	
	/* поле типа Long */
	struct TBFieldLong : ITBField
	{
		long long val;
		TBFieldLong() { type = Long; }
		virtual void read(istream& in);
		virtual void write(ostream& out, bool endline = true);
	};

	/* поле типа Text */
	struct TBFieldText : ITBField
	{
		string val;

		TBFieldText() { type = Text; }
		virtual void read(istream& in);
		virtual void write(ostream& out, bool endline = true);
	};
	
	/* класс таблицы */
	class Table 
	{
		friend class DBMS;
		
		TableStruct tbstr; // структура таблицы

		fstream file; // файловый поток (~файл таблицы)
		string name; // имя таблицы
		
		list< ITBField** > records; // список массивов указателей на поля (для использования механизма виртуальности)
		list< ITBField** >::iterator currentRecord; // итератор на текущую запись таблицы
		int currentId; // Id текущей записи втаблице


		Mutex mutex; /* мьютекс, отвечающий за блокировку; используется 
				* из DBMS (по-хорошему надо переписать, чтобы
				* таблица сама блокировала себя, когда нужно. 
				* */

	public:
		Table(const string& name) : name(name), currentId(-1) {}

		void open(); //считать таблицу из файла
		void close(); // закрыть таблицу
		void deleteTable(); // удалить таблицу		
		void updateFile(); // сброить таблицу из памяти в файл
		void removeCurrentRecord(); // удаление текущей записи
		void moveToStart() // перейти на первую запись
		{
			currentRecord = records.begin();
			if (currentId != -1)
			{
				currentId = 0;
			}
		}
		void moveNext() // перейти на следующую запись
		{
			if (afterEnd())
				return;
			++currentRecord;
			++currentId;
		}
		void movePrev() // перейти на предыдущую запись
		{
			if (atStart())
				return;
			--currentRecord;
			--currentId;
		}
		void moveToId(int id) // перейти на запись по Id. медленно
		{
			if (id >= tbstr.numRecords || id < 0)
				throw IndexOutOfRange();
			if (id == currentId)
				return;

			if (id > currentId)
				while (id != currentId)
					moveNext();
			else
				while (id != currentId)
					movePrev();
		}

		bool empty() { return tbstr.numRecords == -1; } // проверка на пустоту
		bool atStart() { return currentId == 0; } // проверка на начало таблицы
		bool atEnd() { return currentId == tbstr.numRecords - 1; } // проверка на конец таблицы
		bool afterEnd() { return currentId == tbstr.numRecords; } // проверка на выход за конец

		ITBField* getField(const string& name);  // поле @name текущей записи таблицы
		FieldType getFieldType(const string& name); // его тип
		FieldType getFieldType(unsigned int id); // тип поля с порядковым номером @id

		unsigned int getNumOfFields() { return tbstr.numFields; } // число полей таблицы

		friend ostream& operator << (ostream& out, Table& self) // вывод в поток
		{
			out << self.name << endl << endl;
			
			out << "ID\t\t";
			for (int i=0; i<self.tbstr.numFields; ++i)
				out << self.tbstr.names[i] << "\t\t";

			out << endl << endl;

			if (self.tbstr.numRecords < 0)
				return out;

			self.moveToStart();
			
			int i = 0;

			for (list<ITBField**>::iterator it = self.records.begin(); it != self.records.end(); ++it, ++i)
			{
				out << "#ID " << i << "\t\t";
				for (int k = 0; k < self.tbstr.numFields; ++k)
				{
					(*it)[k]->write(out, false);
					out << "\t\t";
				}
				out << endl;
			}

			return out;
		}
	};

	// класс СУБД
	class DBMS
	{
		vector<Table*> tables; // иассив (вектор) таблиц
		vector<bool> deleted; // удалена ли соответствующая таблица
		map<string, IntWrap> nameMap; // соответствие: имя - номер в векторе
		unsigned totalN; // общее колличество таблиц
		
                vector<Lex> pst; // стек для интерпретации полиза
                
		fstream* log; // указатель на файл лога. по умолчанию - NULL (если выключено логирование)
		bool logging; // включено ли логирование
	public:

		DBMS() : totalN(0), log(NULL), logging(false)
		{
		}
		~DBMS();
		void proceedQuery (const string& query, ostream& out = cout) {} // обработка запроса пользователя
                
                /* Подсчёт выраженитя @expr в контексте текущей записи таблицы.
                 * После завершения работы функции в стеке @@pst должно остаться
                 * только значение выражения. 
                 * */
                void evaluateExpr(const string& tableName, Poliz& expr) {} 
		
                /* функции, выполняющие команды над таблицами возвращают 
		* строку-ответ */
		
		/* SELECT из таблицы с именем @tbname полей из списка @fdList 
		* при условии истинности выражения @whereCl */
		const string Select (const string& tbname, vector<string>& fdList, Poliz& whereCl);
		/* INSERT в таблицу с именем @tbname записи, чьи значения 
		* полей содержатся в @valList */
		const string Insert (const string& tbname, vector<Lex>& valList);
		/* UPDATE для таблицы с именем @tbname присвоением полю 
		* с именем @fdname значения выражения @expr при условии 
		* истинности выражения @whereCl */
		const string Update (const string& tbname, const string& fdname, Poliz& expr, Poliz& whereCl);
		/* CREATE таблицы с именем @tbname, поля которой описываются в @desceiption */
		const string Create (const string& tbname, vector<Lex> description);
		/* DELETE записей из таблицы с именем @tbname при условии истинности выражения @whereCl */
		const string Delete (const string& tbname, Poliz& whereCl);

		void setLogFile(fstream& file) // установка файла-лога
		{
			logging = true;
			log = &file;
		}
		
		bool isTableNameValid (const string& name); // проверка существования таблицы @name
		
		/* работа с таблицами в целом
		* ВАЖНО: закрытие таблицы выгружает из памяти ВСЕ записи 
		* таблицы!
		* 
		* То есть удалять таблицы из массива таблиц имеет смысл только 
		* при их "файловом удалении". При закрытии таблицы в памяти 
		* остайтся, пр большому счёту, только структура таблицы.
		* */

		void addTable(const string& name); // добавляет в СУБД таблицу, но не открывает её
		void openTable(const string& name); // открывает уже находящуюся в СУБД таблицу
		void closeTable(const string& name); // закрывает таблицу, но запись в СУБД остаётся
		void removeTableFromMemory(const string& name); /* дорогостоящая операция, мало толку
								* убирет запись о таблице из @tables
								* !! приводит к перестроению @nameMap !!
								* */
		
		void deleteTable(const string& name); // удаляет файл таблицы и запись о ней из системы
		void updateTableFile (const string& name); // обновление файла таблицы @name
		
		// Работа с таблицами по-отдельности
		void addRecordToTable(const string& name, ITBField** vec); // добавление записи в таблицу

		void removeCurrentRecordFromTable(const string& name) // удаление текущей записи из таблицы @name
		{
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}
			tables[i]->removeCurrentRecord();
		}
		void moveNext(const string& name) // переход к следующей записе в таблице @name
		{
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}
			tables[i]->moveNext();
		}
		void movePrev(const string& name) // переход к предыдущей записе в таблице @name
		{
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}

			tables[i]->movePrev();
		}
		void moveToId(const string& name, unsigned int id) // переход к записе с порядковым новером @id в таблице @name
		{
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}

			tables[i]->moveToId(id);
		}
                
                void moveToStart(const string& name)
                {
                        IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}

			tables[i]->moveToStart();
                }
                
		bool afterTableEnd(const string& name) // проверка таблицы @name на выход за конец
		{ 
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}

			return tables[i]->afterEnd();
		}

		ITBField* getField(const string& tbname, const string& fdname); // поле @fdname текущей записи таблицы @tbname
		FieldType getFieldType (const string& tbname, const string& fdname); // его тип

		FieldType getFieldType(const string& tbname, unsigned int id); // тип поля с индексом @id в таблице @tbname
		unsigned int getNumOfFields(const string& tbname); // число полей таблицы @tbname

		TableStruct getTableStruct(const string& tbname) // структура таблицы @tbname
		{
			IntWrap i = nameMap[tbname];
			if (!i)
			{
				throw TableNotFound();
			}
			return tables[i]->tbstr;
		}
		
		void lockTable (const string& name) // блокировка таблицы @name
		{
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}
			
			tables[i]->mutex.lock();
			
			time_t t = time(NULL);
			if (logging)
			{
			*log << asctime(localtime(&t)) << " : table " << name << " locked." << endl;
			log->flush();
			}
			cout << asctime(localtime(&t)) << " : table " << name << " locked." << endl;
			
		}
		void unlockTable (const string& name) // разблокировка таблицы @name
		{
			IntWrap i = nameMap[name];
			if (!i)
			{
				throw TableNotFound();
			}
			tables[i]->mutex.unlock();
			
			time_t t = time(NULL);
			if (logging)
			{
			*log << asctime(localtime(&t)) << " : table " << name << " unlocked." << endl;
			log->flush();
			}
			cout << asctime(localtime(&t)) << " : table " << name << " unlocked." << endl;
		}
	};

}

#endif