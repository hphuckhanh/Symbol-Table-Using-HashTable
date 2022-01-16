#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

class SymbolTable
{
public:
    SymbolTable() {}
    void run(string filename);
	
	struct paraType {
		string type = "";
		string value = "";
	};
	
	struct hItem {
		long long key = 0;
		string name = "";
		string value = "";
		int level = -1;
		int numParam = 0;
		paraType *ptype = NULL;
		string type = "";
	};
	
	struct hash_para {
		string hash_type = "";
		int m = 0;
		int c = 0;
		int c1 = 0;
		int c2 = 0;
	};
	
	hash_para param;
	hItem* table = NULL;
	
	bool is_Valid_name(string name);
	bool isString(string str);
	bool isNumber(const string& str);
	bool isOtherSymbol(string str);
	bool isdeclared (string name, int level);
	int check_value_type(string value);
	long long id_encode(string str, int level);
	
	int linear_hash(long long key, int m, int c, int i);
	int quad_hash(long long key, int m, int c1, int c2, int i);
	int double_hash(long long key, int m, int c, int i);
	int insert(string hash_type, long long key, string name, int level, int numParam);
	int lookup(string name, int level);
	int assign(int index, string name, string value, int level);
	int call(string func, int level);
	int probing(long long key);
	int assign_check(string value, int level);
	void printTable();

};

#endif