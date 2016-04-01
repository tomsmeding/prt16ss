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
};

enum tokentype_t{
	TT_STRING,
	TT_NUMBER,
	TT_ADDRESS,
	TT_RANGE,
	TT_NAME,
	TT_SYMBOL
};

class Token{
public:
	tokentype_t type;
	string value;

	Token(tokentype_t type);
	Token(tokentype_t type,string value);
};
