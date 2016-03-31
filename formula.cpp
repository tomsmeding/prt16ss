#include "formula.h"

using namespace std;

Token::Token(tokentype_t type)
	:type(type){}

Token::Token(tokentype_t type,string value)
	:type(type),value(value){}


//either returns error message or the list of parsed tokens
Either<string,vector<Token>> tokeniseFormula(const string &formula){
	size_t i,len=formula.size();
	vector<Token> tokens;
	for(i=0;i<len;i++){
		if(formula[i]=='"'||formula[i]=='\''){
			const char quote=formula[i];
			Token token(TT_STRING);
			for(i++;i<len;i++){
				if(formula[i]==quote)break;
				if(formula[i]=='\\'){
					if(i>=len-2)return string("String niet afgesloten met een quote");
					token.value+='\\';
					token.value+=formula[i+1];
					i+=2;
				} else {
					token.value+=formula[i];
				}
			}
			if(i==len)return string("String niet afgesloten met een quote");
			tokens.push_back(move(token));
		} else if(isdigit(formula[i])){
			const char *iptr=&formula[i];
			char *endptr;
			strtod(iptr,&endptr);
			int nparsed=endptr-iptr;
			//Because the string passed to strtod always starts with a digit,
			//its conversion should always at least succeed.
			tokens.emplace_back(TT_NUMBER,formula.substr(i,nparsed));
			i+=nparsed-1; //set i on the last character parsed
		} else if(strchr("+-*/%():",formula[i])!=nullptr){
			tokens.emplace_back(TT_SYMBOL,string(1,formula[i]));
		} else if(isupper(formula[i])){
			int starti=i;
			for(i++;i<len;i++){
				if(!isupper(formula[i]))break;
			}
			if(i!=len&&isdigit(formula[i])){
				for(i++;i<len;i++){
					if(!isdigit(formula[i]))break;
				}
				tokens.emplace_back(TT_ADDRESS,formula.substr(starti,i-starti));
			} else {
				tokens.emplace_back(TT_NAME,formula.substr(starti,i-starti));
			}
			i--;
		}
	}
	return tokens;
}

//returns either an error message or a parsed AST
Either<string,ASTNode*> parseFormula(const string &formula){
	const Either<string,vector<Token>> &mtokens=tokeniseFormula(formula);
	if(mtokens.isLeft())return mtokens.fromLeft();
	const vector<Token> &tokens=mtokens.fromRight();

	size_t i,ntokens=tokens.size();
	for(i=0;i<ntokens;i++){
		switch(tokens[i].type){
		case TT_STRING:
			;
		}
	}

	return nullptr;
}
