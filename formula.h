#pragma once

#include "celladdress.h"
#include "either.h"
#include <string>
#include <vector>
#include <cstring>
#include <cctype>

using namespace std;

enum astnodetype_t{
	AN_FUNCTION,
	AN_STRING,
	AN_NUMBER,
	AN_ADDRESS,
	AN_RANGE,
	AN_OPERATOR
};

class ASTNode{
public:
	astnodetype_t type;
	double numval;
	string strval;
	CellAddress addrval;
	CellRange rangeval;
	vector<ASTNode*> children;

	ASTNode(astnodetype_t type,double numval);
	ASTNode(astnodetype_t type,string strval);
	ASTNode(astnodetype_t type,CellAddress addrval);
	ASTNode(astnodetype_t type,CellRange rangeval);

	~ASTNode();

	void print(int tablevel=0) const;
};

enum tokentype_t{
	TT_STRING,
	TT_NUMBER,
	TT_ADDRESS,
	TT_RANGE,
	TT_NAME,
	TT_SYMBOL
};

#include <unordered_map>

unordered_map<int,string> tokenstring={
	{TT_STRING,"TT_STRING"},
	{TT_NUMBER,"TT_NUMBER"},
	{TT_ADDRESS,"TT_ADDRESS"},
	{TT_RANGE,"TT_RANGE"},
	{TT_NAME,"TT_NAME"},
	{TT_SYMBOL,"TT_SYMBOL"}
};

class Token{
public:
	tokentype_t type;
	string value;

	Token(tokentype_t type);
	Token(tokentype_t type,string value);
};
