#include "formula.h"
#include "cell.h"
#include <sstream>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <cmath>
#include <cctype>
#include <climits>

using namespace std;

Formula::ASTNode::ASTNode(astnodetype_t type,double numval)
	:type(type),numval(numval){}

Formula::ASTNode::ASTNode(astnodetype_t type,string strval)
	:type(type),strval(strval){}

Formula::ASTNode::ASTNode(astnodetype_t type,CellAddress addrval)
	:type(type),addrval(addrval){}

Formula::ASTNode::ASTNode(astnodetype_t type,CellRange rangeval)
	:type(type),rangeval(rangeval){}

Formula::ASTNode::~ASTNode(){
	for(ASTNode *node : children){
		delete node;
	}
}



enum tokentype_t{
	TT_STRING,
	TT_NUMBER,
	TT_ADDRESS,
	TT_RANGE,
	TT_NAME,
	TT_SYMBOL
};

class Formula::Token{
public:
	tokentype_t type;
	string value;

	Token(tokentype_t type);
	Token(tokentype_t type,string value);
};

Formula::Token::Token(tokentype_t type)
	:type(type){}

Formula::Token::Token(tokentype_t type,string value)
	:type(type),value(value){}



const unordered_map<string,function<double(const CellArray&,CellRange)>> functionmap={
	{"SUM",[](const CellArray &cells,CellRange range) -> double {
		double res=0;
		const char *startp;
		char *endp;
		for(const Cell &cell : cells.range(range)){
			string dispstr=cell.getDisplayString();
			startp=dispstr.data();
			double item=strtod(startp,&endp);
			if(endp-startp==(ptrdiff_t)dispstr.size()){
				res+=item; //if the value is not a number, it just isn't added
			}
		}
		return res;
	}},
	{"AVG",[](const CellArray &cells,CellRange range) -> double {
		double sum=functionmap.at("SUM")(cells,range);
		if(isnan(sum))return sum;
		return sum/range.size();
	}},
	{"COUNT",[](const CellArray &cells,CellRange range) -> double {
		int count=0;
		for(const Cell &cell : cells.range(range)){
			count+=cell.getDisplayString().size()!=0;
		}
		return count;
	}}
};



//assumes that the char under the cursor is an uppercase character
Maybe<Formula::Token> Formula::tryTokeniseNameAddressRange(const string &formula,int &cursor){
	int len=formula.size();
	int start=cursor;
	for(cursor++;cursor<len;cursor++){ //first letters
		if(!isupper(formula[cursor]))break;
	}
	if(cursor==len||!isdigit(formula[cursor])){
		return Formula::Token(TT_NAME,formula.substr(start,cursor-start));
	}

	for(cursor++;cursor<len;cursor++){ //first numbers
		if(!isdigit(formula[cursor]))break;
	}
	if(cursor==len||formula[cursor]!=':'){ //the colon
		return Formula::Token(TT_ADDRESS,formula.substr(start,cursor-start));
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
	return Formula::Token(TT_RANGE,formula.substr(start,cursor-start));
}

//either returns error message or the list of parsed tokens
Either<string,vector<Formula::Token>> Formula::tokeniseFormula(const string &formula){
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

Either<string,Formula::ASTNode*> Formula::parseExpression(const vector<Token> &tokens){
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
				//nodestack.push_back(new ASTNode(AN_RANGE,CellRange::fromRepresentation(tokens[i].value).fromJust()));
				for(ASTNode *node : nodestack)delete node;
				return string("Range outside of function call");
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
				if(functionmap.find(tokens[i].value)==functionmap.end()){
					for(ASTNode *node : nodestack)delete node;
					return "Unknown function "+tokens[i].value;
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
				if(tokens[i].value=="("){
					opstack.push_back("(");
				} else {
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



Formula::Formula(ASTNode *root)
	:root(root){}

Formula::~Formula(){
	delete root;
}

Either<string,Formula*> Formula::parse(const string &s){
	Either<string,vector<Token>> mtokens=tokeniseFormula(s);
	if(mtokens.isLeft())return mtokens.fromLeft();
	Either<string,ASTNode*> mtree=parseExpression(mtokens.fromRight());
	if(mtree.isLeft())return mtree.fromLeft();
	return new Formula(mtree.fromRight());
}

void Formula::collectDependencies(ASTNode *node,vector<CellAddress> &deps) const {
	unsigned int x,y;
	unsigned int fromx,fromy,tox,toy;
	switch(node->type){
		case AN_FUNCTION:
		case AN_OPERATOR:
			for(ASTNode *child : node->children){
				collectDependencies(child,deps);
			}
			break;
		case AN_ADDRESS:
			deps.push_back(node->addrval);
			break;
		case AN_RANGE:
			fromx=node->rangeval.from.column;
			fromy=node->rangeval.from.row;
			tox=node->rangeval.to.column;
			toy=node->rangeval.to.row;
			deps.reserve(deps.size()+(tox-fromx+1)*(toy-fromy+1));
			for(y=fromy;y<=toy;y++){
				for(x=fromx;x<=tox;x++){
					deps.emplace_back(y,x);
				}
			}
			break;
		default:
			break;
	}
}

vector<CellAddress> Formula::getDependencies() const {
	vector<CellAddress> deps;
	collectDependencies(root,deps);
	return deps;
}

double modulo(double a,double b){
	b=abs(b);
	return a<0?a+floor(-a/b)*b:a-floor(a/b)*b;
}

Either<double,string> Formula::evaluateSubtree(ASTNode *node,const CellArray &cells) const {
	switch(node->type){
		case AN_STRING:
			return node->strval;
		case AN_NUMBER:
			return node->numval;
		case AN_ADDRESS:
			if(node->addrval.row>=cells.height()||
			   node->addrval.column>=cells.width()){
				return string(); //yet non-existent cells are empty
			}
			return cells[node->addrval].getDisplayString();
		case AN_RANGE:
			//should not happen, since ranges are only parsed in function calls
			return string("?range?");
		case AN_FUNCTION:
			if(node->children.size()!=1||
			   (node->children[0]->type!=AN_ADDRESS&&node->children[0]->type!=AN_RANGE)){
				//should not happen, since the parser enforces the function(addr/range) structure
				return string("?functionarg?");
			}
			try {
				if(node->children[0]->type==AN_ADDRESS){
					return functionmap.at(node->strval)(
						cells,
						CellRange(node->children[0]->addrval,node->children[0]->addrval)
					);
				} else {
					return functionmap.at(node->strval)(cells,node->children[0]->rangeval);
				}
			} catch(out_of_range){ //on the functionmap.at
				//should not happen, again because of parser checks
				return string("?function?");
			}
		case AN_OPERATOR:{
			Either<double,string> arg[2]={
				evaluateSubtree(node->children[0],cells),
				evaluateSubtree(node->children[1],cells)
			};
			double argd[2];
			const char *startp;
			char *endp;
			for(int i=0;i<2;i++){
				if(arg[i].isLeft())argd[i]=arg[i].fromLeft();
				else {
					const string &argstr=arg[i].fromRight();
					startp=argstr.data();
					argd[i]=strtod(startp,&endp);
					if(endp-startp!=(ptrdiff_t)argstr.size())argd[i]=nan("");
				}
			}
			if(node->strval=="+")return argd[0]+argd[1];
			else if(node->strval=="-")return argd[0]-argd[1];
			else if(node->strval=="*")return argd[0]*argd[1];
			else if(node->strval=="/")return argd[0]/argd[1];
			else if(node->strval=="%")return modulo(argd[0],argd[1]);
			else if(node->strval=="^")return pow(argd[0],argd[1]);
			else return string("?operator?"); //again, should not happen etc.
		}
		default:
			return string("?nodetype?"); //shouldn't even be think of it
	}
}

string Formula::evaluate(const CellArray &cells) const {
	Either<double,string> res=evaluateSubtree(root,cells);
	if(res.isLeft()){
		stringstream ss;
		ss<<res.fromLeft();
		return ss.str();
	}
	return res.fromRight();
}