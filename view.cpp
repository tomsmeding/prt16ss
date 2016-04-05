#include <cstdlib>
#include "view.h"
#include "util.h"

SheetView::SheetView(Spreadsheet &sheet)
		:sheet(sheet){
	initscr();
	start_color();
	noecho();
	win=newwin(LINES,COLS,0,0);
	keypad(win, TRUE);
	redraw();
}

SheetView::~SheetView(){
	delwin(win);
	endwin();
}

void SheetView::redraw(){
	int i,j;
	attron(A_REVERSE);
	for(i=0;i<COLS/8-1;i++){
		string label=centreString(columnLabel(i+scroll.column),8);
		mvwaddstr(win,0,columnToX(i),label.data());
	}
	for(i=0;i<LINES-1;i++){
		string label=centreString(to_string(i+scroll.row),8);
		mvwaddstr(win,rowToY(i),0,label.data());
	}
	attron(A_NORMAL);
	for(i=0;i<COLS/8-1;i++){
		for(j=0;j<LINES-1;j++){
			redrawCell(CellAddress(j+scroll.row,i+scroll.column),false);
		}
	}
	wrefresh(win);
	wmove(win,rowToY(cursor.row),columnToX(cursor.column));
}

void SheetView::redrawCell(CellAddress addr){
	redrawCell(addr,true);
}

void SheetView::redrawCell(CellAddress addr,bool dorefresh){
	if(addr.row<scroll.row||addr.column<scroll.column||
	   addr.row>=scroll.row+LINES-1||addr.column>=scroll.column+COLS/8-1){
		return;
	}
	sheet.ensureSheetSize(addr.column+1,addr.row+1);
	int leftx;
	string value;
	if(addr==cursor){
		value=sheet.getCellEditString(addr).fromJust();
		attron(A_REVERSE);
		leftx=min(columnToX(addr.column),COLS-1-(int)value.size());
	} else {
		value=sheet.getCellDisplayString(addr).fromJust();
		if(value.size()>8)value.erase(value.begin()+8,value.end());
		leftx=columnToX(addr.column);
	}
	string blankstr(max(8,(int)value.size()),' ');
	mvwaddstr(win,rowToY(addr.row),leftx,blankstr.data());
	mvwaddstr(win,rowToY(addr.row),leftx,value.data());
	if(addr==cursor){
		attroff(A_REVERSE);
		cursordispval=value;
	}
	if(dorefresh){
		wrefresh(win);
		wmove(win,rowToY(cursor.row),leftx);
	}
}

void SheetView::setCursorPosition(CellAddress addr){
	CellAddress oldcursor=cursor;
	cursor=addr;
	redrawCell(oldcursor,false);
	redrawCell(cursor,true);
}

Maybe<string> SheetView::getStringWithEditWindowOverCell(CellAddress loc,string defval){
	const int cellx=columnToX(loc.column),celly=rowToY(loc.row);
	//const int popupx=max(0,cellx-)
}

int SheetView::getChar(){
	return wgetch(win);
}

int SheetView::rowToY(int row) const {
	return row-scroll.row+1;
}

int SheetView::columnToX(int column) const {
	return 8+8*(column-scroll.column);
}
