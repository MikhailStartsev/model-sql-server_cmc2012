#include "database.h"
using namespace DataBase;

// <TBField>
void TBFieldLong::read(istream& in)
{
	char sym = in.peek();
	if ((sym < '0' || sym > '9') && sym != '+' && sym != '-')
		throw BadFieldType();
	bool sign = true;
	if (sym == '+' || sym == '-')
	{
		sym = in.get();
		sign = (sym == '+');
		sym = in.peek();
	}
	if (sym < '0' || sym > '9')
		throw BadFieldType();
	in >> val;
	val *= (sign ? 1 : -1);
	in.ignore( numeric_limits<int>::max(), '\n');
}

void TBFieldLong::write(ostream& out, bool endline)
{
	out << val;
	if (endline)
		out << endl;
}

void TBFieldText::read(istream& in)
{
	getline(in, val);
}

void TBFieldText::write(ostream& out, bool endline)
{
	out << val;
	if (endline)
		out << endl;
}

// </TBField>

// <TableStruct>

void TableStruct::getTableStructFromFile(fstream& file)
{
	string s;
	//#Head
	file >> ws;
	
	getline(file, s);
	if (s != "#HEAD")
	{
		file.close();
		throw BadFileFormat();
	}
	file >> s;
	if (s!="$FIELDS")
	{
		file.close();
		throw BadFileFormat();
	}
	int n = -1;
	file >> n;
	if (n < 0)
	{
		file.close();
		throw BadFileFormat();
	}
	numFields = n;
	
	file >> s;
	if (s!="$RECORDS")
	{
		file.close();
		throw BadFileFormat();
	}
	n = -1;
	file >> n;
	if (n < 0)
	{
		file.close();
		throw BadFileFormat();
	}
	numRecords = n;
	
	//#Fields
	file >> ws;
	getline(file, s);
	if (s != "#FIELDS")
	{
		file.close();
		throw BadFileFormat();
	}
	
	for (int i = 0; i < numFields; ++i)
	{
		file >> s;
		if (s != "$NAME:")
		{
			file.close();
			throw BadFileFormat();
		}	
		file.get();
		s = "";
		getline(file, s);
		if (s == "" || !(!nameMap[s]))
		{
			file.close();
			throw BadFileFormat();
		}
		names.push_back(s);
		
		nameMap[s] = i;
		
		file >> s;
		if (s != "$TYPE:")
		{
			file.close();
			throw BadFileFormat();
		}	
		file.get();
		s = "";
		getline(file, s);
		if (s == "LONG")
		{
			types.push_back(Long);
		}
		else
		{
			if (s == "TEXT")
			{
				types.push_back(Text);
			}
			else
				throw BadFileFormat();
		}
		
	}
	
}
// </TableStruct>

// <Table>
void Table::updateFile()
{
	if (tbstr.numRecords < 0)
		return;
	file.close();
	file.open((name + dbFileExtention).c_str(), ofstream::out);
	
	file << "#HEAD\n" << "$FIELDS " << tbstr.numFields << endl;
	file << "$RECORDS " << tbstr.numRecords << "\n\n#FIELDS\n";
	
	for (int i = 0; i < tbstr.numFields; ++i)
	{
		file << "$NAME: " << tbstr.names[i];
		file << "\n$TYPE: ";
		if (tbstr.types[i] == Long)
			file << "LONG\n";
		else
			file << "TEXT\n";
		file << endl;
	}
	
	file << "#RECORDS\n";
	int i = 0;
	
	for (list<ITBField**>::iterator it = records.begin(); it != records.end(); ++it, ++i)
	{
		file << "#ID " << i << endl;
		for (int k = 0; k < tbstr.numFields; ++k)
		{
			(*it)[k]->write(file);
		}
		file << endl;
	}
	file.flush();
}

void Table::close()
{
	updateFile();
	file.close();
	
	for (list<ITBField**>::iterator it = records.begin(); it != records.end(); ++it)
	{
		for (int k = 0; k < tbstr.numFields; ++k)
		{
			delete (*it)[k];
		}
		delete[] (*it);
	}
	
	records.clear();
	tbstr.numRecords = -1;
	
}

void Table::deleteTable()
{
	file.close();
	if (remove((name + dbFileExtention).c_str()))
	{
		// throw CantDeleteTable(); // почему-то всегда кидало иселючение, хотя таблицу удаляло
	}
	
}

void Table::open()
{
	string s;
	file.open((name + dbFileExtention).c_str(), fstream::in);
	
	if (file.fail())
	{
		throw DBError("Table not found: " + name + ".");
	}
	
	tbstr.getTableStructFromFile(file);
	file >> ws;
	
	getline(file, s);
	if (s != "#RECORDS")
	{
		file.close();
		throw BadFileFormat();
	}
	
	for (int i = 0; i < tbstr.numRecords; ++i)
	{
		records.push_back( new ITBField* [tbstr.numFields] );
		
		file >> ws;
		getline(file, s);
		for (int k = 0; k < tbstr.numFields; ++k)
		{
			if (tbstr.types[k] == Long)
			{
				(*records.rbegin())[k] = new TBFieldLong();
			}
			else
			{
				(*records.rbegin())[k] = new TBFieldText();
			}
			
			
			(*records.rbegin())[k]->read(file);
		}
	}
	
	currentId = 0;
	currentRecord = records.begin();
	if (tbstr.numRecords == 0)
		currentId = -1;
}

void Table::removeCurrentRecord()
{
	list<ITBField**>::iterator buf = currentRecord;
	if (empty())
		return;
	if (!atEnd())
	{
		moveNext();
		currentId--;
	}
	else
		movePrev();
	records.erase(buf);
	--tbstr.numRecords;
	
	if (tbstr.numRecords == 0)
		currentId = -1;
}

ITBField* Table::getField(const string& name)
{
	IntWrap i = tbstr.nameMap[name];
	if (!i)
	{
		throw FieldNotFound();
	}
	return (*currentRecord)[i];
}
FieldType Table::getFieldType(const string& name)
{
	IntWrap i = tbstr.nameMap[name];
	if (!i)
	{
		throw FieldNotFound();
	}
	return tbstr.types[i];
}

FieldType Table::getFieldType(unsigned int id)
{
	return tbstr.types[id];
}
// </Table>

// <DBMS>

void DBMS::addTable(const string& name)
{
	IntWrap i = nameMap[name];
	if (!i || deleted[i])
	{
		nameMap[name] = totalN++;
		tables.push_back(new Table(name));
		deleted.push_back(false);
		return;
	}
	throw TableOpened();	
}

void DBMS::openTable(const string& name)
{
	IntWrap i = nameMap[name];
	if (!i)
	{
		throw TableNotFound();
	}
	tables[i]->open();
	deleted[i] = false;
}

void DBMS::closeTable(const string& name)
{
	IntWrap i = nameMap[name];
	if (!i)
	{
		throw TableNotFound();
	}
	if (deleted[i])
		return;
	tables[i]->close();
}

void DBMS::removeTableFromMemory(const string& name)
{
	closeTable(name);
	
	int ind = nameMap[name];
	tables.erase(tables.begin() + ind);
	deleted.erase(deleted.begin() + ind);
	--totalN;
	for (int i=ind;i<totalN;++i)
		--nameMap[ tables[i]->name ];
}

void DBMS::deleteTable(const string& name)
{
	IntWrap i = nameMap[name];
	if (!i)
	{
		throw TableNotFound();
	}
	if (deleted[i])
		throw TableNotFound();
	tables[i]->deleteTable();
	deleted[i] = true;
	removeTableFromMemory(name);
	nameMap[name].ok = false;
}

ITBField* DBMS::getField(const string& tbname, const string& fdname)
{
	IntWrap i = nameMap[tbname];
	if (!i)
	{
		throw TableNotFound();
	}
	return tables[i]->getField(fdname);
}

void DBMS::updateTableFile(const string& name)
{
	IntWrap i = nameMap[name];
	if (!i || deleted[i])
	{
		throw TableNotFound();
	}
	tables[i]->updateFile();
}

DBMS::~DBMS()
{
	for (unsigned int i = 0; i < totalN; ++i)
		if (!deleted[i])
			tables[i]->updateFile();
}

bool DBMS::isTableNameValid (const string& name)
{
	IntWrap i = nameMap[name];
	if (!i || deleted[i])
	{
		return false;
	}
	return true;
}

FieldType DBMS::getFieldType(const string& tbname, const string& fdname)
{
	IntWrap i = nameMap[tbname];
	if (!i || deleted[i])
	{
		throw TableNotFound();
	}
	return tables[i]->getFieldType(fdname);
}

FieldType DBMS::getFieldType(const string& tbname, unsigned int id)
{
	IntWrap i = nameMap[tbname];
	if (!i || deleted[i])
	{
		throw TableNotFound();
	}
	return tables[i]->getFieldType(id);
}

unsigned int DBMS::getNumOfFields(const string& tbname)
{
	IntWrap i = nameMap[tbname];
	if (!i || deleted[i])
	{
		throw TableNotFound();
	}
	return tables[i]->getNumOfFields();
}





const string DBMS::Select(const string& tbname, vector<string>& fdList, Poliz& whereCl)
{
	stringstream result;
	
	if (fdList.size() == 1 && fdList[0] == "*")
	{
		fdList = getTableStruct(tbname).names;
	}
	
	
	
	Table* tab = new Table("from "+tbname);
	tab->tbstr.names = fdList;
	tab->tbstr.numFields = fdList.size();
	tab->tbstr.numRecords = 0;
	
	if (getTableStruct(tbname).numRecords == 0)
	{
		result << *tab << endl << "OK." << endl;
		return result.str();
	}
	moveToStart(tbname);
	
	IntWrap ind;
	
	for (int i = 0; i < tab->tbstr.numFields; ++i)
	{
		tab->tbstr.types.push_back( getFieldType(tbname, fdList[i]) );
		tab->tbstr.nameMap[fdList[i]] = i;
	}
	
	Lex res;
	stringstream buf;
	ITBField* buf_fd;
	
	
	while (!afterTableEnd(tbname))
	{
		evaluateExpr(tbname, whereCl);
		res = pst[0];
		if (res.get_type() != Lex_Bool)
		{
			throw TypeMismatch("WHERE-clause expected a boolean expression as it's argument.");
		}
		
		if (res.get_value()) // подходит под where-клаузу
		{
			tab->records.push_back( new ITBField* [tab->tbstr.numFields] );
			
			for (int k = 0; k < tab->tbstr.numFields; ++k)
			{
				if (tab->tbstr.types[k] == Long)
				{
					(*(tab->records.rbegin()))[k] = new TBFieldLong();
				}
				else
				{
					(*(tab->records.rbegin()))[k] = new TBFieldText();
				}
				
				IntWrap ii = tab->tbstr.nameMap[fdList[k]];
				if (!ii)
				{
					throw FieldNotFound();
				}
				
				buf_fd = getField(tbname, tab->tbstr.names[ii]);
				
				buf_fd->write(buf, false);
				buf << endl;
				
				(*(tab->records.rbegin()))[k]->read(buf);
				
			}
			
			++(tab->tbstr.numRecords);
			
			if (tab->currentId == -1)
			{
				tab->currentId = 0;
				tab->currentRecord = tab->records.begin();
			}
		}
		
		moveNext(tbname);
	}
	
	result << *tab << endl << "OK.\n\n" << endl;
	
	moveToStart(tbname);
	
	return result.str();
}

const string DBMS::Insert(const string& tbname, vector<Lex>& valList)
{
	// типы точно верные, проверено в Ins_Sent
	IntWrap i = nameMap[tbname];
	
	if (!i)
	{
		throw TableNotFound();
	}
	
	ITBField** tbf = new ITBField*[getNumOfFields(tbname)]; //это тоже относительно критическая секция. но путсь нет :)
	int n = valList.size();
	
	stringstream buf;
	
	for (int i = 0; i < n; ++i)
	{
		if (valList[i].get_type() == Lex_Str)
		{
			tbf[i] = new TBFieldText();
			buf << valList[i].get_str_value() << endl;
			tbf[i]->read(buf);
		}
		else
		{
			tbf[i] = new TBFieldLong();
			buf << valList[i].get_value() << endl;
			tbf[i]->read(buf);
		}
	}
	
	lockTable(tbname);
	addRecordToTable(tbname, tbf);
	unlockTable(tbname);
	return "OK.\n\n";
}

void DBMS::addRecordToTable(const string& tbname, ITBField** vec)
{
	IntWrap i = nameMap[tbname];
	
	if (!i)
	{
		throw TableNotFound();
	}
	
	tables[i]->records.push_back(vec);
	tables[i]->tbstr.numRecords++;
	
	if (tables[i]->currentId == -1)
	{
		tables[i]->currentId = 0;
		tables[i]->currentRecord = tables[i]->records.begin();
	}
}

const string DBMS::Update(const string& tbname, const string& fdname, Poliz& expr, Poliz& whereCl)
{	
	int affR = 0;
	
	lockTable(tbname);
	
	if (getTableStruct(tbname).numRecords == 0)
		return "OK.\n0 affected rows.\n\n";
	moveToStart(tbname);
	
	Lex res;
	stringstream buf;
	ITBField* fdp;
	
	IntWrap ind = nameMap[tbname];
	if (!ind || deleted[ind])
		throw TableNotFound();
	
	while (!afterTableEnd(tbname))
	{
		evaluateExpr(tbname, whereCl);
		res = pst[0];
		if (res.get_type() != Lex_Bool)
		{
			throw TypeMismatch("WHERE-clause expected a boolean expression as it's argument.");
		}
		
		if (res.get_value()) // подходит под where-клаузу
		{
			++affR;
			pst.clear();
			evaluateExpr(tbname, expr);
			res = pst[0];
			
			fdp = getField(tbname, fdname);
			
			if (getFieldType(tbname, fdname) == Long)
			{
				if (res.get_type() != Lex_Num)
				{
					throw TypeMismatch("Type mismatch in UPDATE: LONG expression expected.");
				}
				buf << res.get_value() << endl;
				
				fdp->read(buf);
			}
			else
			{
				if (res.get_type() != Lex_Str)
				{
					throw TypeMismatch("Type mismatch in UPDATE: LONG expression expected.");
				}
				buf << res.get_str_value() << endl;
				
				fdp->read(buf);
			}
		}
		
		moveNext(tbname);
	}
	
	moveToStart(tbname);
	
	unlockTable(tbname);
	
	stringstream s;
	s << "OK.\n" << affR << " affected rows.\n\n";
	
	return s.str();
}

const string DBMS::Create (const string& tbname, vector<Lex> des)
{
	fstream f((tbname + dbFileExtention).c_str(), fstream::in);
	if (!f.fail())
	{
		throw TableOpened();
	}
	f.close();
	
	addTable(tbname);
	
	lockTable(tbname);
	
	IntWrap ind = nameMap[tbname];
	if (!ind || deleted[ind])
	{
		throw DBError("WTF, dude?");
	}
	
	Table* tb = tables[ind];
	int n = des.size()/2;
	tb->tbstr.numFields = n;
	tb->tbstr.numRecords = 0;
	
	Lex type, name;
	
	for (int i=0; i<n; i++)
	{
		type = des[2*i];
		name = des[2*i+1];
		
		tb->tbstr.names.push_back(name.get_str_value());
		tb->tbstr.types.push_back( (type.get_type() == Lex_Long) ? Long : Text );
		tb->tbstr.nameMap[tb->tbstr.names[i]] = i;
	}
	
	updateTableFile(tbname);
	
	unlockTable(tbname);
	
	return "OK.\n\n";
}

const string DBMS::Delete (const string& tbname, Poliz& whereCl)
{
	int affR = 0;
	
	if (getTableStruct(tbname).numRecords == 0)
		return "OK. \n0 affected rows.\n\n";
	
	lockTable(tbname);
	
	moveToStart(tbname);
	
	Lex res;
	
	IntWrap ind = nameMap[tbname];
	if (!ind || deleted[ind])
		throw TableNotFound();
	
	while (!afterTableEnd(tbname))
	{
		evaluateExpr(tbname, whereCl);
		res = pst[0];
		if (res.get_type() != Lex_Bool)
		{
			throw TypeMismatch("WHERE-clause expected a boolean expression as it's argument.");
		}
		
		if (res.get_value()) // подходит под where-клаузу
		{
			tables[ind]->removeCurrentRecord();	
			++affR;
		}
		else
			moveNext(tbname);
	}
	
	moveToStart(tbname);
	
	unlockTable(tbname);
	
	stringstream s;
	s << "OK.\n" << affR << " affected rows.\n\n";
	
	return s.str();
}
// </DBMS>