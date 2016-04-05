#pragma once

#include "celladdress.h"
#include "maybe.h"
#include "either.h"
#include <string>
#include <vector>

using namespace std;

/*
A wrapper for a formula, with useful functions for parsing and evaluating.
Used extensively (obviously) by CellValueFormula.
*/

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

		ASTNode(astnodetype_t type,double numval) noexcept;
		ASTNode(astnodetype_t type,string strval) noexcept;
		ASTNode(astnodetype_t type,CellAddress addrval) noexcept;
		ASTNode(astnodetype_t type,CellRange rangeval) noexcept;

		~ASTNode() noexcept;
	};

	class Token;
	class Partialresult;


	ASTNode *root;

	Formula(ASTNode *root) noexcept;

	//Parsing and tokenisation sub functions
	static Maybe<Token> tryTokeniseNameAddressRange(const string &formula,int &cursor) noexcept;
	static Either<string,vector<Token>> tokeniseFormula(const string &formula) noexcept;
	static Either<string,ASTNode*> parseExpression(const vector<Token> &tokens) noexcept;

	//evaluation and dep getting sub functions
	void collectDependencies(ASTNode *node,vector<CellAddress> &deps) const noexcept;
	Partialresult evaluateSubtree(ASTNode *node,const CellArray &cells) const noexcept;

public:
	~Formula() noexcept;

	//Maybe construct a Formla
	static Either<string,Formula*> parse(const string &s) noexcept;

	vector<CellAddress> getDependencies() const noexcept;

	//returns Nothing if an error in dependencies
	Maybe<string> evaluate(const CellArray &cells) const noexcept;
};
