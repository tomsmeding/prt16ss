#include <cstdlib>
#include "view.h"
#include "util.h"

SheetView::SheetView(Spreadsheet &sheet)
		:sheet(sheet){
	initscr();
	start_color();
	noecho();
	keypad(stdscr, TRUE);
	redraw();
}

SheetView::~SheetView(){
	endwin();
}

void SheetView::redraw(){
	int i,j;
	move(0,0);
	addstr("        ");
	attron(A_REVERSE);
	for(i=0;i<COLS/8-1;i++){
		string label=centreString(columnLabel(i+scroll.column),8);
		mvaddstr(0,columnToX(i),label.data());
	}
	for(i=0;i<LINES-2;i++){
		string label=centreString(to_string(i+1+scroll.row),8);
		mvaddstr(rowToY(i),0,label.data());
	}
	attroff(A_REVERSE);
	for(i=0;i<COLS/8-1;i++){
		for(j=0;j<LINES-1;j++){
			if(CellAddress(j+scroll.row,i+scroll.column)==cursor)continue;
			redrawCell(CellAddress(j+scroll.row,i+scroll.column),false);
		}
	}
	redrawCell(cursor,true);
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
	string dispvalue,value;
	dispvalue=sheet.getCellDisplayString(addr).fromJust();
	if(addr==cursor){
		value=sheet.getCellEditString(addr).fromJust();
		if(dispvalue.size()>8&&dispvalue!=value){
			displayStatusString(dispvalue); //inform of full display value
		}
		attron(A_REVERSE);
		leftx=min(columnToX(addr.column),COLS-1-(int)value.size());
	} else {
		value=sheet.getCellDisplayString(addr).fromJust();
		if(value.size()>8)value.erase(value.begin()+8,value.end());
		leftx=columnToX(addr.column);
	}
	string blankstr(max(8,(int)value.size()),' ');
	mvaddstr(rowToY(addr.row),leftx,blankstr.data());
	mvaddstr(rowToY(addr.row),leftx,value.data());
	if(addr==cursor){
		attroff(A_REVERSE);
		cursordispval=value;
	}
	if(dorefresh){
		refresh();
		move(rowToY(cursor.row),leftx);
	}
}

void SheetView::setCursorPosition(CellAddress addr){
	bool didscroll=false;
	if(addr.column>=scroll.column+COLS/8-1){
		scroll.column+=addr.column-scroll.column-(COLS/8-1);
		didscroll=true;
	}
	if(addr.row>=scroll.row+LINES-2){
		scroll.row+=addr.row-scroll.row-(LINES-2);
		didscroll=true;
	}
	if(didscroll){
		redraw();
	} else {
		CellAddress oldcursor=cursor;
		cursor=addr;
		redrawCell(oldcursor,false);
		if(oldcursor.column<COLS/8-1+scroll.column-1){
			redrawCell(CellAddress(oldcursor.row,oldcursor.column+1));
		}
		displayStatusString("");
		redrawCell(cursor,true);
	}
}

Maybe<string> SheetView::getTextBoxString(int wid,string buffer){
	int storey,storex;
	getyx(stdscr,storey,storex);
	for(int i=0;i<wid;i++)addch(' ');
	if((int)buffer.size()>wid)buffer.erase(buffer.begin()+wid,buffer.end());
	mvaddstr(storey,storex,buffer.data());
	while(true){
		int c=getch();
		if(c==27){ //escape
			return Nothing(); //didn't edit anything
		} else if(c==KEY_BACKSPACE||c==127){
			if(buffer.size()>0){
				buffer.pop_back();
				mvaddch(storey,storex+buffer.size(),' ');
				move(storey,storex+buffer.size());
			}
		} else if(c=='\n'){
			break; //accepted value
		} else if(c>=32&&c<127){
			if((int)buffer.size()<wid){
				buffer+=(char)c;
				addch((char)c);
			}
		}
	}
	return buffer;
}

Maybe<string> SheetView::getStringWithEditWindowOverCell(CellAddress loc,string defval){
	const int cellx=columnToX(loc.column),celly=rowToY(loc.row);
	const int popupx=min(cellx,COLS-17);
	drawBoxAround(popupx,celly,16,1);
	move(celly,popupx);
	Maybe<string> ret=getTextBoxString(16,defval);
	redraw();
	return ret;
}

Maybe<string> SheetView::askStringOfUser(string prompt,string prefilled){
	int storey,storex;
	getyx(stdscr,storey,storex);
	move(LINES-1,0);
	if((int)prompt.size()>=COLS-10)prompt.erase(prompt.begin()+COLS-10,prompt.end());
	addstr(prompt.data());
	addch(' ');
	Maybe<string> ret=getTextBoxString(COLS-prompt.size()-1,prefilled);
	move(storey,storex);
	return ret;
}

void SheetView::displayStatusString(string s){
	int storey,storex;
	getyx(stdscr,storey,storex);
	move(LINES-1,0);
	if((int)s.size()>=COLS)s.erase(s.begin()+COLS,s.end());
	s.reserve(COLS);
	for(int i=s.size();i<COLS;i++)s+=' ';
	addstr(s.data());
	move(storey,storex);
}

int SheetView::getChar(){
	return getch();
}

int SheetView::rowToY(int row) const {
	return row-scroll.row+1;
}

int SheetView::columnToX(int column) const {
	return 8+8*(column-scroll.column);
}

void SheetView::drawBoxAround(int x,int y,int w,int h){
	move(y-1,x-1);
	addch('+');
	string hor(w,'-');
	addstr(hor.data());
	addch('+');
	for(int i=y;i<y+h;i++){
		mvaddch(i,x-1,'|');
		mvaddch(i,x+w,'|');
	}
	mvaddch(y+h,x-1,'+');
	addstr(hor.data());
	addch('+');
}
