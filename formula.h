#pragma once

#include "celladdress.h"
#include "maybe.h"
#include "either.h"
#include <string>
#include <vector>

using namespace std;

class CellArray;

class Formula{
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
		const double numval=0;
		const string strval;
		const CellAddress addrval=CellAddress(0,0);
		const CellRange rangeval=CellRange(CellAddress(0,0),CellAddress(0,0));
		vector<ASTNode*> children;

		ASTNode(astnodetype_t type,double numval);
		ASTNode(astnodetype_t type,string strval);
		ASTNode(astnodetype_t type,CellAddress addrval);
		ASTNode(astnodetype_t type,CellRange rangeval);

		~ASTNode();
	};

	class Token;


	ASTNode *root;

	Formula(ASTNode *root);

	static Maybe<Token> tryTokeniseNameAddressRange(const string &formula,int &cursor);
	static Either<string,vector<Token>> tokeniseFormula(const string &formula);
	static Either<string,ASTNode*> parseExpression(const vector<Token> &tokens);

	void collectDependencies(ASTNode *node,vector<CellAddress> &deps) const;
	Either<double,string> evaluateSubtree(ASTNode *node,const CellArray &cells) const;

public:
	~Formula();

	static Either<string,Formula*> parse(const string &s);

	vector<CellAddress> getDependencies() const;

	string evaluate(const CellArray &cells) const;
};
