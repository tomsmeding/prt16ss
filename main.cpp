#include <ncurses.h>
#include <iostream>
#include <unistd.h>
#include "view.h"

using namespace std;

int main(){
	Spreadsheet sheet(9,23);
	SheetView view(sheet);
	CellAddress cursor(0,0);
	string lastsavefname;
	while(true){
		int c=view.getChar();
		switch(c){
			case 'q':
				return 0;
			case '\n':{
				Maybe<string> mv=view.getStringWithEditWindowOverCell(cursor,sheet.getCellEditString(cursor).fromJust());
				if(mv.isJust()){
					sheet.changeCellValue(cursor,mv.fromJust());
					view.redraw();
					/*for(CellAddress addr : sheet.changeCellValue(cursor,mv.fromJust()).fromJust()){
						view.redrawCell(addr);
					}*/
				}
				break;
			}
			case 's':{
				Maybe<string> mfname=view.askStringOfUser("Save to file name?",lastsavefname);
				if(mfname.isNothing()){
					view.displayStatusString("Not saved.");
					break;
				}
				lastsavefname=mfname.fromJust();
				if(sheet.saveToDisk(lastsavefname)){
					view.displayStatusString("Saved.");
				} else {
					view.displayStatusString("Error while saving!");
				}
				break;
			}
			case 'l':{
				Maybe<string> mfname=view.askStringOfUser("Load from file name?",lastsavefname);
				if(mfname.isNothing()){
					view.displayStatusString("Not loaded.");
					break;
				}
				lastsavefname=mfname.fromJust();
				if(sheet.loadFromDisk(lastsavefname)){
					view.displayStatusString("Loaded.");
				} else {
					view.displayStatusString("Error while loading!");
				}
				view.redraw();
				break;
			}
			case KEY_LEFT:
				if(cursor.column>0){
					cursor.column--;
					view.setCursorPosition(cursor);
				}
				break;
			case KEY_RIGHT:
				cursor.column++;
				view.setCursorPosition(cursor);
				break;
			case KEY_UP:
				if(cursor.row>0){
					cursor.row--;
					view.setCursorPosition(cursor);
				}
				break;
			case KEY_DOWN:
				cursor.row++;
				view.setCursorPosition(cursor);
				break;
			default:
				cout<<'\007'<<flush;
				break;
		}
	}
	return 0;
}
