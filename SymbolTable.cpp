#include "SymbolTable.h"

enum command 
{
	LINEAR,
	QUADRATIC,
	DOUBLE,
	INSERT,
	ASSIGN,
	CALL,
	LOOKUP,
	PRINT,
	BEGIN,
	END
};

void SymbolTable::run(string filename)
{
    ifstream ifs;
	ifs.open(filename, ifstream::in);
	int k=0;
	int level = 0;
	
	string init;
	getline(ifs, init);
	if(init == "") throw InvalidInstruction(init);
	stringstream con(init);
	string con_arr[5];
	int i = 0;
	while(con.good())
	{
		string tmp;
		getline(con, tmp, ' ');
		con_arr[i] = tmp;
		i++;
	}
	if(con_arr[0] != "LINEAR" && con_arr[0] != "QUADRATIC" && con_arr[0] != "DOUBLE") throw InvalidInstruction(init);
	param.hash_type = con_arr[0];
	param.m = stoi(con_arr[1]);
	table = new hItem[param.m];
	
	if(param.hash_type == "LINEAR")
	{
		if(i != 3) throw InvalidInstruction(init);
		param.c = stoi(con_arr[2]);
	}
	else if(param.hash_type == "QUADRATIC")
	{
		if(i != 4) throw InvalidInstruction(init);
		param.c1 = stoi(con_arr[2]);
		param.c2 = stoi(con_arr[3]);
	}
	else if(con_arr[0] == "DOUBLE")
	{
		if(i != 3) throw InvalidInstruction(init);
		param.c = stoi(con_arr[2]);
	}
	
	while(!ifs.eof())
	{
		string lenh;
		string arr[5];
		
		string input;
		getline(ifs, input);
		
		stringstream line(input);
		if(input == "") {
			if(k > 0) cout << endl;
			throw InvalidInstruction(input);
		}
		int idx = 0;
		while (line.good())
		{
			string tmp;
			getline(line, tmp, ' ');
			arr[idx] = tmp;
			if (arr[0] == "ASSIGN")
			{
				if(idx == 1) break;
			}
			idx++;
		}
		lenh = arr[0];
		if(k == 0 && lenh == "PRINT") continue;
		if(k == 0 && lenh == "BEGIN") {
			level++;
			continue;
		}
		if(k>0 && lenh != "BEGIN" && lenh != "END") cout << endl;
		int code = -1;
		if (lenh == "INSERT")			code = INSERT;	
		else if (lenh == "ASSIGN")		code = ASSIGN;	
		else if (lenh == "CALL")		code = CALL;
		else if (lenh == "LOOKUP")		code = LOOKUP;
		else if (lenh == "PRINT")		code = PRINT;
		else if (lenh == "BEGIN")		code = BEGIN;
		else if (lenh == "END")			code = END;
		else throw InvalidInstruction(input);
		switch (code) 
		{
			case INSERT:
			{

				long long id = 0;
				if(idx != 2 && idx != 3) 
					throw InvalidInstruction(input);
				else
				{
					if(is_Valid_name(arr[1]))
					{
						if(arr[1] == "string" || arr[1] == "number" || arr[1] == "void") throw InvalidInstruction(input);
						else 
						{
							id = id_encode(arr[1], level);
							int ins_result = 0;
							if(idx == 3) 
							{
								if(level > 0) throw InvalidDeclaration(input);
								else ins_result = insert(param.hash_type, id, arr[1], level, stoi(arr[2]));
							}
							else if(idx == 2)
							{
								ins_result = insert(param.hash_type, id, arr[1], level, 0);
							}
							if(ins_result == -1) throw Overflow(input);
							else if(ins_result == -2) throw Redeclared(arr[1]);
							else cout << ins_result;
						}
					}
					else throw InvalidDeclaration(input);
				}
				break;
			}
			case ASSIGN:
			{
				if(idx != 1 || !is_Valid_name(arr[1])) throw InvalidInstruction(input);
				else
				{
					string str = arr[0] + " " + arr[1] + " ";
					int t = str.length();
					string value = input.substr(t);
					if(value.find('(') != std::string::npos)
					{
						int kq = assign_check(value, level);
						if(kq == -1) throw TypeCannotBeInferred(input);
						else if(kq == -2) throw TypeMismatch(input);
						else if(kq == -3) throw Undeclared(input);
						else if(kq == -4) throw InvalidInstruction(input);
					}
					int look = lookup(arr[1], level);
					if(look == -1) 
						throw Undeclared(input);
					else
					{
						if(table[look].numParam > 0) throw TypeMismatch(input);
						int result = assign(look, arr[1], value, level);
						if(result == -1) throw TypeCannotBeInferred(input);
						else if(result == -2) throw TypeMismatch(input);
						else if(result == -3) throw Undeclared(input);
						else if(result == -4) throw InvalidInstruction(input);
						else cout << result;
					}
				}
				break;
			}
			case CALL:
			{
				int res = -10;
				if(idx != 2) throw InvalidInstruction(input);
				else 
				{
					string str = arr[0] + " ";
					int t = str.length();
					string func = input.substr(t);
					res = call(func, level);
				}
				if(res == -1)
					throw TypeCannotBeInferred(input);
				else if(res == -2)
					throw TypeMismatch(input);
				else if(res == -3)
					throw Undeclared(input);
				else if(res == -4) 
					throw InvalidInstruction(input);
				else
					cout << res;
				break;
			}
			case LOOKUP:
			{
				int res = -2;
				if(idx != 2) throw InvalidInstruction(input);
				else res = lookup(arr[1], 1);
				
				if(res == -1) throw Undeclared(input);
				else cout << res;
				break;
			}
			case PRINT:
			{
				if(idx != 1) throw InvalidInstruction(input);
				printTable();
				break;
			}
			case BEGIN:
			{
				if(idx != 1) throw InvalidInstruction(input);
				level++;
				break;
			}
			case END:
			{
				if(idx != 1) throw InvalidInstruction(input);
				if(level==0 || level<0) throw UnknownBlock();
				level--;
				break;
			}
		}
		k++;
	}
	if (level > 0) {
		cout << endl;
		throw UnclosedBlock(level);
	} 
	else if (level < 0) 
	{
		cout << endl;
		throw UnknownBlock();
	}
		
	delete [] table;
}

bool SymbolTable::is_Valid_name(string name)
{
	string first = name.substr(0,1);
	if(first.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos ) return 1;
	if(name.find_first_not_of("a'bcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890") != std::string::npos){
		return false;
	}
	return true;
}

bool SymbolTable::isString(string str)
{
	string ss = str.substr(0,2);
	if(str.find("'") == 0){
		if(str.find("'", 1) == (str.length()-1)){
			 if (str.empty())
			 {
				return false;
			 }
			 else if(str.find_first_not_of("a'bcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 ") != std::string::npos)
			 {
				return false;
			 }
			 else
			 {
				return true;
			 }
		} else return false;
	} else return false;
}

bool SymbolTable::isNumber(const string& str)
{
    for (char const &c : str) {
        if (isdigit(c) == 0) return false;
    }
    return true;
}

bool SymbolTable::isOtherSymbol(string str)
{	
	int rlt=false;
	if (str.find("'") == std::string::npos){
		if(str.find_first_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890") == std::string::npos){
			rlt = true;
		} else rlt = false;
	}
	return rlt;
}

int SymbolTable::check_value_type(string value)
{
	int type = 0;
	if(isNumber(value) == 1) type = 1;
	else if(isString(value) == 1) type = 2;
	else if(isOtherSymbol(value) == 1) type = 3;
	return type;
} 

bool SymbolTable::isdeclared (string name, int level)
{
	int res = false;
	for(int i = 0; i < param.m; i++)
	{
		if(table[i].name == name && table[i].level == level) res = true;
	}
	return res;
}

long long SymbolTable::id_encode(string str, int level)
{
	stringstream t;
	t << level;
	string s;
	char* name = &(str)[0];
	for(int i=0; i < (int)strlen(name); i++)
	{
		long long num = (long long)(name[i]) - 48;
		t << num;
	}
	t >> s;
	return stoll(s, nullptr, 10);
}

int SymbolTable::linear_hash(long long key, int m, int c, int i)
{
	int h1 = key % m;
	int h = (h1 + c*i) % m;
	return h;
}

int SymbolTable::quad_hash(long long key, int m, int c1, int c2, int i)
{
	int h1 = key % m;
	int h = (h1 + c1*i + c2*i*i) % m;
	return h;
}

int SymbolTable::double_hash(long long key, int m, int c, int i)
{
	int h1 = key % m;
	int h2 = 1 + (key % (m-2));
	int h = (h1 + c*i*h2) % m;
	return h;
}

int SymbolTable::insert(string hash_type, long long key, string name, int level, int numParam)
{
	int idx = -1;
	int run = 0;
	int first = -1;
	int result = -3;
	if(isdeclared(name, level)) return -2;
	if(hash_type == "LINEAR")
	{
		idx = linear_hash(key, param.m, param.c, run);
		first = idx;
		while (idx < param.m)
		{
			if(table[idx].level == -1)
			{
				table[idx].key = key;
				table[idx].name = name;
				table[idx].level = level;
				table[idx].numParam = numParam;
				if(numParam > 0)
				{
					table[idx].ptype = new paraType[numParam];
				}
				break;
			} 
			else 
			{
				run++;
				idx = linear_hash(key, param.m, param.c, run);
				if(idx == first) break;
			}
		}
		if(idx > param.m || (idx == first && run > 0)) result = -1;
		else result = run;
	}
	else if(hash_type == "QUADRATIC")
	{
		idx = quad_hash(key, param.m, param.c1, param.c2, run);
		first = idx;
		while (idx < param.m)
		{
			if(table[idx].level == -1)
			{
				table[idx].key = key;
				table[idx].name = name;
				table[idx].level = level;
				table[idx].numParam = numParam;
				if(numParam > 0)
				{
					table[idx].ptype = new paraType[numParam];
				}
				break;
			} 
			else 
			{
				run++;
				idx = quad_hash(key, param.m, param.c1, param.c2, run);
				if(idx == first) break;
			}
		}
		if(idx > param.m || (idx == first && run > 0)) result = -1;
		else result = run;
	}
	else if(hash_type == "DOUBLE")
	{
		idx = double_hash(key, param.m, param.c, run);
		first = idx;
		while (idx < param.m)
		{
			if(table[idx].level == -1)
			{
				table[idx].key = key;
				table[idx].name = name;
				table[idx].level = level;
				table[idx].numParam = numParam;
				if(numParam > 0)
				{
					table[idx].ptype = new paraType[numParam];
				}
				break;
			} 
			else 
			{
				run++;
				idx = double_hash(key, param.m, param.c, run);
				if(idx == first) break;
			}
		}
		if(idx > param.m || (idx == first && run > 0)) result = -1;
		else result = run;
	}
	return result;
}

int SymbolTable::lookup(string name, int level)
{
	int res = -1;
	while(level >= 0)
	{
		for(int i = 0; i < param.m; i++)
		{
			if(table[i].name == name && table[i].level == level)
			{
				res = i;
				break;
			}
		}
		if(res != -1) break;
		else level--;
	}		
	return res;
}

void SymbolTable::printTable()
{
	int i = 0;
	for(i=0; i < param.m; i++)
	{
		if(table[i].name != "")
		{
			cout << i << " " << table[i].name << "//" << table[i].level;
			break;
		}
	}
	int tmp = i+1;
	for(i = tmp; i < param.m; i++)
	{
		if(table[i].name != "")
		{
			cout << ";" << i << " " << table[i].name << "//" << table[i].level;
		}
	}
}

int SymbolTable::assign(int index, string name, string value, int level)
{
	int res = -5;
	int kq = 0;
	kq = kq + probing(table[index].key);
	if(table[index].type == "")
	{
		if(isNumber(value))
		{
			table[index].type = "number";
			table[index].value = value;
			res = 1;
		}
		else if(isString(value))
		{
			table[index].type = "string";
			table[index].value = value;
			res = 1;
		}
		else if(isOtherSymbol(value))
		{
			if(value == "string" || value == "number" || value == "void") return -4;
			int sym2 = -1;
			sym2 = lookup(value, level);
			if(sym2 != -1)
			{
				if(table[sym2].numParam > 0) return -4;
				kq = kq + probing(table[sym2].key);
				if(table[sym2].type == "" && table[index].type == "") 
					return -1;
				else if(table[sym2].type == table[index].type)
				{
					table[index].value == table[sym2].value;
					res = 1;
				}
				else if(table[index].type == "")
				{
					table[index].type = table[sym2].type;
					table[index].value = table[sym2].value;
					res = 1;
				}	
			}				
			else return -3;
		}
		else
		{
			int f = value.find('(');
			if(value.find('(') == std::string::npos || value.find(')') == std::string::npos) return -4;
			string sym = value.substr(0,f);
			string tmp = value.substr(f);
			string para = value.substr(f+1, tmp.find(')')-1);
			string a[100];
			stringstream sv(para);
			int v = 0;
			while (sv.good()) {
				if(v > 99) break;
				string substr;
				getline(sv, substr, ',');
				if(substr == "string" || substr == "number" || substr == "void") return -4;
				a[v] = substr;
				v++;
			}
			if(a[0] == "") return -2;
			if(v == 0) return -4;
			int vitri = -3;
			vitri = lookup(sym, level);
			kq = kq + probing(table[vitri].key);
			if(vitri == -3) return -3;
			else 
			{
				if(table[vitri].numParam == 0 || table[vitri].numParam != v) return -2;
				for(int i=0; i < v; i++)
				{
					if(table[vitri].ptype[i].type == "")
					{
						if(isNumber(a[i]))
						{
							table[vitri].ptype[i].type = "number";
							table[vitri].ptype[i].value = a[i];
							res = 1;
						}
						else if(isString(a[i]))
						{
							table[vitri].ptype[i].type = "string";
							table[vitri].ptype[i].value = a[i];
							res = 1;
						}
						else if(isOtherSymbol(a[i]))
						{
							int symbol = -1;
							symbol = lookup(a[i], level);
							if(symbol != -1)
							{
								if(table[symbol].numParam > 0) return -2;
								kq = kq + probing(table[symbol].key);
								if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
									return -1;
								else if(table[symbol].type == table[vitri].ptype[i].type)
								{
									table[vitri].ptype[i].value == table[symbol].value;
									res = 1;
								}
								else if(table[vitri].ptype[i].type == "")
								{
									table[vitri].ptype[i].type = table[symbol].type;
									table[vitri].ptype[i].value = table[symbol].value;
									res = 1;
								}	
							}				
							else return -3;
						}
						else return -2;
					}
					else
					{
						if(table[vitri].ptype[i].type == "number")
						{
							if(isNumber(a[i]))
							{
								table[vitri].ptype[i].value = a[i];
								res = 1;
							}
							else if(isString(a[i])){
								return -2;
							}
							else if(isOtherSymbol(a[i]))
							{
								int symbol = -1;
								symbol = lookup(a[i], level);
								if(symbol != -1)
								{
									if(table[symbol].numParam > 0) return -2;
									kq = kq + probing(table[symbol].key);
									if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
										return -1;
									else if(table[symbol].type == table[vitri].ptype[i].type)
									{
										table[vitri].ptype[i].value == table[symbol].value;
										res = 1;
									}
									else if(table[vitri].ptype[i].type == "")
									{
										table[vitri].ptype[i].type = table[symbol].type;
										table[vitri].ptype[i].value = table[symbol].value;
										res = 1;
									}	
								}				
								else return -3;
							}
							else return -2;
						}
						else if(table[vitri].ptype[i].type == "string")
						{
							if(isString(a[i]))
							{
								table[vitri].ptype[i].value = a[i];
								res = 1;
							}
							else if(isNumber(a[i]))
							{
								return -2;
							}
							else if(isOtherSymbol(a[i]))
							{
								int symbol = -1;
								symbol = lookup(a[i], level);
								if(symbol != -1)
								{
									if(table[symbol].numParam > 0) return -2;
									kq = kq + probing(table[symbol].key);
									if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
										return -1;
									else if(table[symbol].type == table[vitri].ptype[i].type)
									{
										table[vitri].ptype[i].value == table[symbol].value;
										res = 1;
									}
									else if(table[vitri].ptype[i].type == "")
									{
										table[vitri].ptype[i].type = table[symbol].type;
										table[vitri].ptype[i].value = table[symbol].value;
										res = 1;
									}	
								}				
								else return -3;
							}
							else return -2;
						}
					}
				}
				if(res == 1)
				{
					if(table[index].type == "" && table[vitri].type != "")
					{
						table[index].type = table[vitri].type;
					}
					else if(table[index].type != "" && table[vitri].type == "")
					{
						table[vitri].type = table[index].type;
					}
					else if(table[index].type == "" && table[vitri].type == "")
						return -1;
				}
			}
		}
	}
	else 
	{
		if(table[index].type == "number")
		{
			if(isNumber(value))
			{
				table[index].value = value;
				res = 1;
			}
			else if(isString(value))
			{
				return -2;
			}
			else if(isOtherSymbol(value))
			{
				int sym2 = -1;
				sym2 = lookup(value, level);
				if(sym2 != -1)
				{
					if(table[sym2].numParam > 0) return -4;
					kq = kq + probing(table[sym2].key);
					if(table[sym2].type == "" && table[index].type == "") 
						return -1;
					else if(table[sym2].type == table[index].type)
					{
						table[index].value == table[sym2].value;
						res = 1;
					}
					else if(table[index].type == "")
					{
						table[index].type = table[sym2].type;
						table[index].value = table[sym2].value;
						res = 1;
					}	
				}				
				else return -3;
			}
			else
			{
				if(value.find('(') == std::string::npos || value.find(')') == std::string::npos) return -4;
				int f = value.find('(');
				string sym = value.substr(0,f);
				string tmp = value.substr(f);
				string para = value.substr(f+1, tmp.find(')')-1);
				string a[100];
				stringstream sv(para);
				int v = 0;
				while (sv.good()) {
					if(v > 99) break;
					string substr;
					getline(sv, substr, ',');
					if(substr == "string" || substr == "number" || substr == "void") return -4;
					a[v] = substr;
					v++;
				}
				if(a[0] == "") return -2;
				if(v == 0) return -4;
				int vitri = -3;
				vitri = lookup(sym, level);
				kq = kq + probing(table[vitri].key);
				if(vitri == -3) return -3;
				else 
				{
					if(table[vitri].numParam == 0 || table[vitri].numParam != v) return -2;
					for(int i=0; i < v; i++)
					{
						if(table[vitri].ptype[i].type == "")
						{
							if(isNumber(a[i]))
							{
								table[vitri].ptype[i].type = "number";
								table[vitri].ptype[i].value = a[i];
								res = 1;
							}
							else if(isString(a[i]))
							{
								table[vitri].ptype[i].type = "string";
								table[vitri].ptype[i].value = a[i];
								res = 1;
							}
							else if(isOtherSymbol(a[i]))
							{
								int symbol = -1;
								symbol = lookup(a[i], level);
								if(symbol != -1)
								{
									if(table[symbol].numParam > 0) return -2;
									kq = kq + probing(table[symbol].key);
									if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
										return -1;
									else if(table[symbol].type == table[vitri].ptype[i].type)
									{
										table[vitri].ptype[i].value == table[symbol].value;
										res = 1;
									}
									else if(table[vitri].ptype[i].type == "")
									{
										table[vitri].ptype[i].type = table[symbol].type;
										table[vitri].ptype[i].value = table[symbol].value;
										res = 1;
									}	
								}				
								else return -3;
							}
							else return -2;
						}
						else
						{
							if(table[vitri].ptype[i].type == "number")
							{
								if(isNumber(a[i]))
								{
									table[vitri].ptype[i].value = a[i];
									res = 1;
								}else return -2;
							}
							else if(table[vitri].ptype[i].type == "string")
							{
								if(isString(a[i]))
								{
									table[vitri].ptype[i].value = a[i];
									res = 1;
								} else return -2;
							}
						}
					}
				}
			}
		}
		else if(table[index].type == "string")
		{
			if(isString(value))
			{
				table[index].value = value;
				res = 1;
			}
			else if(isNumber(value))
			{
				return -2;
			}
			else if(isOtherSymbol(value))
			{
				int sym2 = -1;
				sym2 = lookup(value, level);
				if(sym2 != -1)
				{
					if(table[sym2].numParam > 0) return -4;
					kq = kq + probing(table[sym2].key);
					if(table[sym2].type == "" && table[index].type == "") 
						return -1;
					else if(table[sym2].type == table[index].type)
					{
						table[index].value == table[sym2].value;
						res = 1;
					}
					else if(table[index].type == "")
					{
						table[index].type = table[sym2].type;
						table[index].value = table[sym2].value;
						res = 1;
					}	
				}				
				else return -3;
			}
			else
			{
				if(value.find('(') == std::string::npos || value.find(')') == std::string::npos) return -4;
				int f = value.find('(');
				string sym = value.substr(0,f);
				string tmp = value.substr(f);
				string para = value.substr(f+1, tmp.find(')')-1);
				string a[100];
				stringstream sv(para);
				int v = 0;
				while (sv.good()) {
					if(v > 99) break;
					string substr;
					getline(sv, substr, ',');
					if(substr == "string" || substr == "number" || substr == "void") return -4;
					a[v] = substr;
					v++;
				}
				if(a[0] == "") return -2;
				int vitri = -3;
				vitri = lookup(sym, level);
				kq = kq + probing(table[vitri].key);
				if(vitri == -3) res = -3;
				else 
				{
					if(table[vitri].numParam == 0 || table[vitri].numParam != v) return -2;
					for(int i=0; i < v; i++)
					{
						if(table[vitri].ptype[i].type == "")
						{
							if(isNumber(a[i]))
							{
								table[vitri].ptype[i].type = "number";
								table[vitri].ptype[i].value = a[i];
								res = 1;
							}
							else if(isString(a[i]))
							{
								table[vitri].ptype[i].type = "string";
								table[vitri].ptype[i].value = a[i];
								res = 1;
							}
							else if(isOtherSymbol(a[i]))
							{
								int symbol = -1;
								symbol = lookup(a[i], level);
								if(symbol != -1)
								{
									if(table[symbol].numParam > 0) return -2;
									kq = kq + probing(table[symbol].key);
									if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
										return -1;
									else if(table[symbol].type == table[vitri].ptype[i].type)
									{
										table[vitri].ptype[i].value == table[symbol].value;
										res = 1;
									}
									else if(table[vitri].ptype[i].type == "")
									{
										table[vitri].ptype[i].type = table[symbol].type;
										table[vitri].ptype[i].value = table[symbol].value;
										res = 1;
									}	
								}				
								else return -3;
							}
							else return -2;
						}
						else
						{
							if(table[vitri].ptype[i].type == "number")
							{
								if(isNumber(a[i]))
								{
									table[vitri].ptype[i].value = a[i];
									res = 1;
								} else return -2;
							}
							else if(table[vitri].ptype[i].type == "string")
							{
								if(isString(a[i]))
								{
									table[vitri].ptype[i].value = a[i];
									res = 1;
								} else return -2;
							}
						}
					}
				}
			}
		}
	}
	return kq;
}
	
	
int SymbolTable::probing (long long key)
{
	int idx = -1;
	int run = 0;
	int first = -1;
	if(param.hash_type == "LINEAR")
	{
		idx = linear_hash(key, param.m, param.c, run);
		first = idx;
		while (idx < param.m)
		{
			if(table[idx].key == key)
			{
				break;
			} 
			else 
			{
				run++;
				idx = linear_hash(key, param.m, param.c, run);
				if(idx == first) break;
			}
		}
	}
	else if(param.hash_type == "QUADRATIC")
	{
		idx = quad_hash(key, param.m, param.c1, param.c2, run);
		first = idx;
		while (idx < param.m)
		{
			if(table[idx].key == key)
			{
				break;
			} 
			else 
			{
				run++;
				idx = quad_hash(key, param.m, param.c1, param.c2, run);
				if(idx == first) break;
			}
		}
	}
	else if(param.hash_type == "DOUBLE")
	{
		idx = double_hash(key, param.m, param.c, run);
		first = idx;
		while (idx < param.m)
		{
			if(table[idx].key == key)
			{
				break;
			} 
			else 
			{
				run++;
				idx = double_hash(key, param.m, param.c, run);
				if(idx == first) break;
			}
		}
	}
	return run;
}

int SymbolTable::call(string func, int level)
{
	int res = 0;
	if(func.find('(') == std::string::npos || func.find(')') == std::string::npos) return -4;
	int f = func.find('(');
	string sym = func.substr(0,f);
	string tmp = func.substr(f);
	string para = func.substr(f+1, tmp.find(')')-1);
	string a[100];
	stringstream sv(para);
	int v = 0;
	while (sv.good()) {
		if(v > 99) break;
		string substr;
		getline(sv, substr, ',');
		if(substr == "string" || substr == "number" || substr == "void") return -4;
		a[v] = substr;
		v++;
	}
	int vitri = -3;
	vitri = lookup(sym, level);
	if(vitri == -3) res = -3;
	else 
	{
		if(table[vitri].numParam == 0 || table[vitri].numParam != v) return -2;
		res = res + probing(table[vitri].key);
		for(int i=0; i < v; i++)
		{
			if(table[vitri].ptype[i].type == "")
			{
				if(isNumber(a[i]))
				{
					table[vitri].ptype[i].type = "number";
					table[vitri].ptype[i].value = a[i];
				}
				else if(isString(a[i]))
				{
					table[vitri].ptype[i].type = "string";
					table[vitri].ptype[i].value = a[i];
				}
				else if(isOtherSymbol(a[i]))
				{
					int symbol = -1;
					symbol = lookup(a[i], level);
					if(symbol != -1)
					{
						if(table[symbol].numParam > 0) return -4;
						if(table[symbol].numParam != 0) res = -2;
						else {
							res = res + probing(table[symbol].key);
							if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
								res = -1;
							else if(table[symbol].type == table[vitri].ptype[i].type)
							{
								table[vitri].ptype[i].value == table[symbol].value;
							}
							else if(table[vitri].ptype[i].type == "")
							{
								table[vitri].ptype[i].type = table[symbol].type;
								table[vitri].ptype[i].value = table[symbol].value;
							}
						}						
					}				
					else res = -3;
				}
				else return -2;
			}
			else
			{
				if(table[vitri].ptype[i].type == "number")
				{
					if(isNumber(a[i]))
					{
						table[vitri].ptype[i].value = a[i];
					} else res = -2;
				}
				else if(table[vitri].ptype[i].type == "string")
				{
					if(isString(a[i]))
					{
						table[vitri].ptype[i].value = a[i];
					}else res = -2;
				}
			}
		}
	}
	if(res == 0 || res > 0) table[vitri].type = "void";;
	return res;
}


int SymbolTable::assign_check(string value, int level)
{
			int res = -5;
			if(value.find('(') == std::string::npos || value.find(')') == std::string::npos) return -4;
			int f = value.find('(');
			string sym = value.substr(0,f);
			string tmp = value.substr(f);
			string para = value.substr(f+1, tmp.find(')')-1);
			string a[100];
			stringstream sv(para);
			int v = 0;
			while (sv.good()) {
				if(v > 99) break;
				string substr;
				getline(sv, substr, ',');
				if(substr == "string" || substr == "number" || substr == "void") return -4;
				a[v] = substr;
				v++;
			}
			if(a[0] == "") return -2;
			if(v == 0) return -4;
			int vitri = -3;
			vitri = lookup(sym, level);
			if(vitri == -3) return -3;
			else 
			{
				if(table[vitri].numParam == 0 || table[vitri].numParam != v) return -2;
				for(int i=0; i < v; i++)
				{
					if(table[vitri].ptype[i].type == "")
					{
						if(isNumber(a[i]))
						{
							res = 1;
						}
						else if(isString(a[i]))
						{
							res = 1;
						}
						else if(isOtherSymbol(a[i]))
						{
							int symbol = -1;
							symbol = lookup(a[i], level);
							if(symbol != -1)
							{
								if(table[symbol].numParam > 0) return -2;
								if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
									return -1;
								else if(table[symbol].type == table[vitri].ptype[i].type)
								{
									res = 1;
								}
								else if(table[vitri].ptype[i].type == "")
								{
									res = 1;
								}	
							}				
							else return -3;
						}
						else return -2;
					}
					else
					{
						if(table[vitri].ptype[i].type == "number")
						{
							if(isNumber(a[i]))
							{
								res = 1;
							}
							else if(isString(a[i])){
								return -2;
							}
							else if(isOtherSymbol(a[i]))
							{
								int symbol = -1;
								symbol = lookup(a[i], level);
								if(symbol != -1)
								{
									if(table[symbol].numParam > 0) return -2;
									if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
										return -1;
									else if(table[symbol].type == table[vitri].ptype[i].type)
									{
										res = 1;
									}
									else if(table[vitri].ptype[i].type == "")
									{
										res = 1;
									}	
								}				
								else return -3;
							}
							else return -2;
						}
						else if(table[vitri].ptype[i].type == "string")
						{
							if(isString(a[i]))
							{
								res = 1;
							}
							else if(isNumber(a[i]))
							{
								return -2;
							}
							else if(isOtherSymbol(a[i]))
							{
								int symbol = -1;
								symbol = lookup(a[i], level);
								if(symbol != -1)
								{
									if(table[symbol].numParam > 0) return -2;
									if(table[symbol].type == "" && table[vitri].ptype[i].type == "") 
										return -1;
									else if(table[symbol].type == table[vitri].ptype[i].type)
									{
										res = 1;
									}
									else if(table[vitri].ptype[i].type == "")
									{
										res = 1;
									}	
								}				
								else return -3;
							}
							else return -2;
						}
					}
				}
				if(table[vitri].type == "void") return -2;
			}
	return res;
}