#include "cell.h"
#include "spreadsheet.h"
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdlib>

using namespace std;


vector<string> split(string s){
	vector<string> res;
	size_t cursor=0;
	while(true){
		size_t idx=s.find(' ',cursor);
		if(idx==string::npos){
			res.push_back(s.substr(cursor));
			return res;
		}
		res.push_back(s.substr(cursor,idx-cursor));
		idx++;
		while(idx<s.size()&&s[idx]==' ')idx++;
		if(idx>=s.size())return res;
		cursor=idx;
	}
}


Spreadsheet sheet(1,1);


unordered_map<string,pair<string,function<void(vector<string>,string)>>> commands={
	{"help",{"show this help",[](vector<string>,string) -> void {
		for(const auto &p : commands){
			cout<<p.first<<": "<<p.second.first<<endl;
		}
	}}},
	{"exit",{"exit",[](vector<string>,string) -> void {
		exit(0);
	}}},
	{"disp",{"[addr] get display string of cell",[](vector<string> cmd,string) -> void {
		if(cmd.size()!=2){cout<<"Pass addr repr ('A1') as argument"<<endl;return;}
		Maybe<CellAddress> addr=CellAddress::fromRepresentation(cmd[1]);
		if(addr.isNothing()){cout<<"Invalid cell address"<<endl;return;}

		Maybe<string> ret=sheet.getCellDisplayString(addr.fromJust());

		if(ret.isNothing()){cout<<"Out of bounds address"<<endl;return;}
		cout<<'"'<<ret.fromJust()<<'"'<<endl;
	}}},
	{"edit",{"[addr] get edit string of cell",[](vector<string> cmd,string) -> void {
		if(cmd.size()!=2){cout<<"Pass addr repr ('A1') as argument"<<endl;return;}
		Maybe<CellAddress> addr=CellAddress::fromRepresentation(cmd[1]);
		if(addr.isNothing()){cout<<"Invalid cell address"<<endl;return;}

		Maybe<string> ret=sheet.getCellEditString(addr.fromJust());

		if(ret.isNothing()){cout<<"Out of bounds address"<<endl;return;}
		cout<<'"'<<ret.fromJust()<<'"'<<endl;
	}}},
	{"ensure",{"[width] [height] ensure sheet size",[](vector<string> cmd,string) -> void {
		if(cmd.size()!=3){
			cout<<"Pass width and height"<<endl;
			return;
		}
		sheet.ensureSheetSize(stoi(cmd[1]),stoi(cmd[2]));
	}}},
	{"change",{"[addr] [value]? change value at addr",[](vector<string> cmd,string line) -> void {
		if(cmd.size()<2){
			cout<<"Pass addr repr and value (empty value also valid)"<<endl;
			return;
		}
		Maybe<CellAddress> addr=CellAddress::fromRepresentation(cmd[1]);
		if(addr.isNothing()){
			cout<<"Invalid cell address"<<endl;
			return;
		}
		string value=cmd.size()==2?"":line.substr(line.find(' ',line.find(' ')+1)+1);
		Maybe<set<CellAddress>> changed=sheet.changeCellValue(addr.fromJust(),value);
		if(changed.isNothing()){
			cout<<"Out of bounds address"<<endl;
			return;
		}
		cout<<"Changed:";
		for(const CellAddress &addr : changed.fromJust()){
			cout<<' '<<addr.toRepresentation();
		}
		cout<<endl;
	}}},
	{"ca",{"[addr] / [row] [column]",[](vector<string> cmd,string) -> void {
		if(cmd.size()!=2&&cmd.size()!=3){
			cout<<"Pass addr or row and column"<<endl;
			return;
		}
		if(cmd.size()==2){
			Maybe<CellAddress> addr=CellAddress::fromRepresentation(cmd[1]);
			if(addr.isNothing()){
				cout<<"Invalid cell address"<<endl;
				return;
			}
			cout<<"row="<<addr.fromJust().row<<" column="<<addr.fromJust().column<<endl;
		} else {
			cout<<CellAddress(stoul(cmd[1]),stoul(cmd[2])).toRepresentation()<<endl;
		}
	}}},
	{"print",{"[width] [height] print part of sheet",[](vector<string> cmd,string) -> void {
		if(cmd.size()!=3){
			cout<<"Pass width and height to print"<<endl;
			return;
		}
		int width,height;
		try {
			width=stoi(cmd[1]),height=stoi(cmd[2]);
		} catch(invalid_argument){
			cout<<"Please pass ints"<<endl;
			return;
		}
		int x,y;
		sheet.ensureSheetSize(width,height);
		for(y=0;y<height;y++){
			for(x=0;x<width;x++){
				if(x!=0)cout<<' ';
				string disp=sheet.getCellDisplayString(CellAddress(y,x)).fromJust();
				cout<<disp.substr(0,8);
				if(disp.size()<8)cout<<string(8-disp.size(),' ');
			}
			cout<<endl;
		}
	}}},
	{"save",{"[fname] save sheet to specified file",[](vector<string> cmd,string line) -> void {
		if(cmd.size()<2){
			cout<<"Pass file name"<<endl;
			return;
		}
		if(sheet.saveToDisk(line.substr(line.find(' ')+1))){
			cout<<"Success"<<endl;
		} else {
			cout<<"Failure"<<endl;
		}
	}}},
	{"load",{"[fname] load sheet from specified file",[](vector<string> cmd,string line) -> void {
		if(cmd.size()<2){
			cout<<"Pass file name"<<endl;
			return;
		}
		if(sheet.loadFromDisk(line.substr(line.find(' ')+1))){
			cout<<"Success"<<endl;
		} else {
			cout<<"Failure"<<endl;
		}
	}}}
};


int main(){
	string line;
	while(cin){
		cout<<"> ";
		getline(cin,line);
		if(line.size()==0)continue;
		string cmd=line.substr(0,line.find(' '));
		auto it=commands.find(cmd);
		if(it==commands.end()){
			cout<<"Command not found: '"<<cmd<<'\''<<endl;
			continue;
		}
		it->second.second(split(line),line);
	}
}
