#include "formula.h"
//#include <iostream>
#include <unordered_map>
#include <climits>

using namespace std;

ASTNode::ASTNode(astnodetype_t type,double numval)
	:type(type),numval(numval),addrval(0,0),rangeval(CellAddress(0,0),CellAddress(0,0)){}

ASTNode::ASTNode(astnodetype_t type,string strval)
	:type(type),strval(strval),addrval(0,0),rangeval(CellAddress(0,0),CellAddress(0,0)){}

ASTNode::ASTNode(astnodetype_t type,CellAddress addrval)
	:type(type),addrval(addrval),rangeval(CellAddress(0,0),CellAddress(0,0)){}

ASTNode::ASTNode(astnodetype_t type,CellRange rangeval)
	:type(type),addrval(0,0),rangeval(rangeval){}

ASTNode::~ASTNode(){
	for(ASTNode *node : children){
		delete node;
	}
}

/*void ASTNode::print(int tablevel) const {
	cout<<string(tablevel*2,' ')<<'(';
	switch(type){
		case AN_FUNCTION: cout<<"FUNCTION "<<strval; break;
		case AN_STRING: cout<<"STRING "<<strval; break;
		case AN_NUMBER: cout<<"NUMBER "<<numval; break;
		case AN_ADDRESS: cout<<"ADDRESS "<<addrval.toRepresentation(); break;
		case AN_RANGE: cout<<"RANGE "<<rangeval.toRepresentation(); break;
		case AN_OPERATOR: cout<<"OPERATOR "<<strval; break;
		default: cout<<"???"; break;
	}
	if(children.size()){
		cout<<endl;
		for(ASTNode *node : children){
			node->print(tablevel+1);
		}
		cout<<string(tablevel*2,' ')<<')'<<endl;
	} else cout<<')'<<endl;
}*/



Token::Token(tokentype_t type)
	:type(type){}

Token::Token(tokentype_t type,string value)
	:type(type),value(value){}



//assumes that the char under the cursor is an uppercase character
Maybe<Token> tryTokeniseNameAddressRange(const string &formula,int &cursor){
	int len=formula.size();
	int start=cursor;
	for(cursor++;cursor<len;cursor++){ //first letters
		if(!isupper(formula[cursor]))break;
	}
	if(cursor==len||!isdigit(formula[cursor])){
		return Token(TT_NAME,formula.substr(start,cursor-start));
	}

	for(cursor++;cursor<len;cursor++){ //first numbers
		if(!isdigit(formula[cursor]))break;
	}
	if(cursor==len||formula[cursor]!=':'){ //the colon
		return Token(TT_ADDRESS,formula.substr(start,cursor-start));
	}

	for(cursor++;cursor<len;cursor++){ //second letters
		if(!isupper(formula[cursor]))break;
	}
	if(cursor==len||!isdigit(formula[cursor])){
		return Nothing(); //AA11:AA, missing numbers of second address
	}

	for(cursor++;cursor<len;cursor++){ //second numbers
		if(!isdigit(formula[cursor]))break;
	}
	return Token(TT_RANGE,formula.substr(start,cursor-start));
}

//either returns error message or the list of parsed tokens
Either<string,vector<Token>> tokeniseFormula(const string &formula){
	int i,len=formula.size();
	vector<Token> tokens;
	for(i=0;i<len;i++){
		if(isspace(formula[i]))continue;
		else if(formula[i]=='"'||formula[i]=='\''){
			const char quote=formula[i];
			Token token(TT_STRING);
			for(i++;i<len;i++){
				if(formula[i]==quote)break;
				if(formula[i]=='\\'){
					if(i>=len-2)return string("String niet afgesloten met een quote");
					token.value+=formula[i+1];
					i++;
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
			//its conversion should always at least succeed (or be out of range,
			//which doesn't really matter)
			tokens.emplace_back(TT_NUMBER,formula.substr(i,nparsed));
			i+=nparsed-1; //set i on the last character parsed
		} else if(strchr("+-*/%^()",formula[i])!=nullptr){
			tokens.emplace_back(TT_SYMBOL,string(1,formula[i]));
		} else if(isupper(formula[i])){
			Maybe<Token> mtoken=tryTokeniseNameAddressRange(formula,i);
			i--;
			if(mtoken.isNothing()){
				return string("Ongeldig adres of range in formule");
			}
			tokens.push_back(mtoken.fromJust());
		} else {
			return string("Ongeldig teken '")+formula[i]+"' in formule";
		}
	}
	return tokens;
}


unordered_map<string,int> precedencemap={
	{"(",INT_MIN},
	{")",INT_MIN},
	{"+",1},
	{"-",1},
	{"*",2},
	{"/",2},
	{"%",2},
	{"^",3}
};
unordered_map<string,bool> leftassocmap={
	{"(",false}, //these should be false to correctly handle '((' cases
	{")",false},

	{"+",true},
	{"-",true}, //make all operators of the same precedence, the same
	{"*",true}, //associativity!
	{"/",true},
	{"%",true},
	{"^",false}
};

Either<string,ASTNode*> parseExpression(const vector<Token> &tokens){
	vector<ASTNode*> nodestack;
	vector<string> opstack;
	ASTNode *node;

	const int len=tokens.size();

	for(int i=0;i<len;i++){
		switch(tokens[i].type){
			case TT_STRING:
				nodestack.push_back(new ASTNode(AN_STRING,tokens[i].value));
				break;
			case TT_NUMBER:
				nodestack.push_back(new ASTNode(AN_NUMBER,strtod(tokens[i].value.data(),nullptr)));
				break;
			case TT_ADDRESS:
				nodestack.push_back(new ASTNode(AN_ADDRESS,CellAddress::fromRepresentation(tokens[i].value).fromJust()));
				break;
			case TT_RANGE:
				nodestack.push_back(new ASTNode(AN_RANGE,CellRange::fromRepresentation(tokens[i].value).fromJust()));
				break;
			case TT_NAME:
				if(i>len-4||
				   tokens[i+1].type!=TT_SYMBOL||
				   tokens[i+1].value!="("||
				   (tokens[i+2].type!=TT_ADDRESS&&tokens[i+2].type!=TT_RANGE)||
				   tokens[i+3].type!=TT_SYMBOL||
				   tokens[i+3].value!=")"){
					for(ASTNode *node : nodestack)delete node;
					return string("Unterminated function call");
				}
				node=new ASTNode(AN_FUNCTION,tokens[i].value);
				if(tokens[i+2].type==TT_ADDRESS){
					node->children.push_back(new ASTNode(AN_ADDRESS,CellAddress::fromRepresentation(tokens[i+2].value).fromJust()));
				} else {
					node->children.push_back(new ASTNode(AN_RANGE,CellRange::fromRepresentation(tokens[i+2].value).fromJust()));
				}
				nodestack.push_back(node);
				i+=3;
				break;
			case TT_SYMBOL:
				const int prec=precedencemap[tokens[i].value];
				const bool leftassoc=leftassocmap[tokens[i].value];
				while(opstack.size()){
					const int otherprec=precedencemap[opstack.back()];
					// both left-assoc: also pop equal-prec operators
					// both right-assoc: only pop higher-prec operators
					if(otherprec>prec||(leftassoc&&otherprec==prec)){
						if(nodestack.size()<2){
							for(ASTNode *node : nodestack)delete node;
							return "Not enough arguments to operator "+opstack.back()+"?";
						}
						node=new ASTNode(AN_OPERATOR,opstack.back());
						node->children.push_back(nodestack[nodestack.size()-2]);
						node->children.push_back(nodestack.back());
						nodestack.pop_back();
						nodestack.pop_back();
						nodestack.push_back(node);
						opstack.pop_back();
					} else break;
				}
				if(tokens[i].value==")"){
					if(opstack.size()==0){
						for(ASTNode *node : nodestack)delete node;
						return string("Excess closing parenthesis");
					}
					//because of their precedence, we can now assume opstack.back()=="("
					opstack.pop_back();
				} else {
					opstack.push_back(tokens[i].value);
				}
				break;
		}
	}
	while(opstack.size()){
		node=new ASTNode(AN_OPERATOR,opstack.back());
		node->children.push_back(nodestack[nodestack.size()-2]);
		node->children.push_back(nodestack.back());
		nodestack.pop_back();
		nodestack.pop_back();
		nodestack.push_back(node);
		opstack.pop_back();
	}
	if(nodestack.size()==1){
		return nodestack.front();
	}
	for(ASTNode *node : nodestack)delete node;
	return to_string(nodestack.size())+" values?";
}



/*int main(){
	string line;
	while(cin){
		cout<<"f> ";
		getline(cin,line);
		if(line.size()==0)continue;
		Either<string,vector<Token>> res=tokeniseFormula(line);
		if(res.isLeft()){
			cout<<'"'<<res.fromLeft()<<'"'<<endl;
		} else {
			const vector<Token> &tokens=res.fromRight();
			for(const Token &token : tokens){
				cout<<tokenstring[token.type]<<"(\""<<token.value<<"\") ";
			}
			cout<<endl;

			Either<string,ASTNode*> mparsed=parseExpression(tokens);
			if(mparsed.isLeft()){
				cout<<'"'<<mparsed.fromLeft()<<'"'<<endl;
			} else {
				ASTNode *parsed=mparsed.fromRight();
				parsed->print();
				delete parsed;
			}
		}
	}
}*/
