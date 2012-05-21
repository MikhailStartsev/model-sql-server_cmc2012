#pragma once

#include <string>
using namespace std;

namespace dberrors
{
	/* классы для ошибок компиляции/таблицы/ДБМС/сервера */
	
	struct Error 
	{
		string message;
	};
	
	
	struct ServerError : Error
	{
		ServerError(const string& msg = "An error occured at the server application.") 
		{
			message = msg;
		}
	};
	
	
	
	struct CompilationError : Error
	{
		CompilationError(const string& msg = "") 
		{
			message = msg;
		}
	};
	
	struct UndefinedID : Error
	{
		UndefinedID (const string& msg = "") 
		{
			message = msg;
		}
	};
	
	struct TypeMismatch : Error
	{
		TypeMismatch (const string& msg = "") 
		{
			message = msg;
		}
	};
	
	struct DBError : Error
	{
		DBError(const string msg = "Error!") 
		{ 
			message = msg; 
		}
	};
	
	struct PolizError : Error
	{
		PolizError (const string& msg = "") 
		{
			message = msg;
		}
	};
	
	struct MutexError : DBError
	{
		MutexError() { message = "Mutex class is used incorrectly."; }
	};
	
	
	struct BadFileFormat : DBError
	{
		BadFileFormat() { message = "Bad file format. You can check file contents manually."; }
	};
	
	struct ReadWriteError : DBError 
	{
		ReadWriteError() { message = "An error occured while reading or writing table."; }
	};
	struct CantCreateTable : DBError 
	{
		CantCreateTable() { message = "Unable to create table. Try closing unneeded tables to free some memory or a different table name."; }
	};
	struct FieldNotFound : DBError 
	{
		FieldNotFound() { message = "The specified table does not have a requested field."; }
	};
	
	struct TableNotFound : DBError 
	{
		TableNotFound() { message = "The specified table does not exist in the system."; }
	};
	
	struct CantDeleteTable : DBError
	{
		CantDeleteTable() { message = "Unable to delete a table file. Permission denied."; }
	};
	struct BadFieldType : DBError
	{
		BadFieldType() { message = "Field type mismatch."; }
	};
	
	struct IndexOutOfRange : DBError
	{
		IndexOutOfRange() { message = "Index is out of range!"; }
	};
	
	struct TableOpened : DBError
	{
		TableOpened() { message = "The table with specified name is already opened."; }
	};
}