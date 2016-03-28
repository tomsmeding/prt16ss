#include "spreadsheet.h"
#include "cells.h"
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


Spreadsheet sheet;


unordered_map<string,pair<string,function<void(vector<string>)>>> commands={
	{"help",{"show this help",[](vector<string>) -> void {
		for(const auto &p : commands){
			cout<<p.first<<": "<<p.second.first<<endl;
		}
	}}},
	{"exit",{"exit",[](vector<string>) -> void {
		exit(0);
	}}},
	{"disp",{"[addr] get display string of cell",[](vector<string> cmd) -> void {
		if(cmd.size()!=2){
			cout<<"Pass addr repr ('A1') as argument"<<endl;
			return;
		}
		Maybe<CellAddress> addr=CellAddress::fromRepresentation(cmd[1]);
		if(addr.isNothing()){
			cout<<"Invalid cell address"<<endl;
			return;
		}
		// cout<<"addr: row="<<addr.fromJust().row<<" column="<<addr.fromJust().column<<endl;
		Maybe<string> ret=sheet.getCellDisplayString(addr.fromJust());
		if(ret.isNothing()){
			cout<<"Out of bounds address"<<endl;
			return;
		}
		cout<<'"'<<ret.fromJust()<<'"'<<endl;
	}}},
	{"ensure",{"[width] [height] ensure sheet size",[](vector<string> cmd) -> void {
		if(cmd.size()!=3){
			cout<<"Pass width and height"<<endl;
			return;
		}
		sheet.ensureSheetSize(stoi(cmd[1]),stoi(cmd[2]));
	}}},
	{"change",{"[addr] [value]? change value at addr",[](vector<string> cmd) -> void {
		if(cmd.size()!=3&&cmd.size()!=2){
			cout<<"Pass addr repr and value (empty value also valid)"<<endl;
			return;
		}
		Maybe<CellAddress> addr=CellAddress::fromRepresentation(cmd[1]);
		if(addr.isNothing()){
			cout<<"Invalid cell address"<<endl;
			return;
		}
		Maybe<set<CellAddress>> changed=sheet.changeCellValue(addr.fromJust(),cmd.size()==3?cmd[2]:"");
		if(changed.isNothing()){
			cout<<"Out of bounds address"<<endl;
			return;
		}
		cout<<"Changed: ";
		for(const CellAddress &addr : changed.fromJust()){
			cout<<addr.toRepresentation()<<' ';
		}
		cout<<endl;
	}}},
	{"ca",{"[addr] / [row] [column]",[](vector<string> cmd) -> void {
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
		it->second.second(split(line));
	}
}
