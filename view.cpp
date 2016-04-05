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
	for(i=0;i<LINES-2;i++){
		string label=centreString(to_string(i+1+scroll.row),8);
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
		if(value.size()>8){
			//displayStatusString(value); //inform of full display value
			value.erase(value.begin()+8,value.end());
		}
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
	if(oldcursor.column<COLS/8-1+scroll.column-1){
		redrawCell(CellAddress(oldcursor.row,oldcursor.column+1));
	}
	redrawCell(cursor,true);
}

Maybe<string> SheetView::getStringWithEditWindowOverCell(CellAddress loc,string buffer){
	const int cellx=columnToX(loc.column),celly=rowToY(loc.row);
	const int popupx=min(cellx,COLS-17);
	drawBoxAround(popupx,celly,16,1);
	if(buffer.size()>16)buffer.erase(buffer.begin()+16,buffer.end());
	mvwaddstr(win,celly,popupx,buffer.data());
	while(true){
		int c=wgetch(win);
		if(c==27){ //escape
			return Nothing(); //didn't edit anything
		} else if(c==KEY_BACKSPACE){
			if(buffer.size()>0){
				buffer.pop_back();
				mvwaddch(win,celly,popupx+buffer.size(),' ');
				wmove(win,celly,popupx+buffer.size());
			}
		} else if(c=='\n'){
			break; //accepted value
		} else if(c>=32&&c<127){
			if(buffer.size()<16){
				buffer+=(char)c;
				waddch(win,(char)c);
			}
		}
	}
	redraw();
	return buffer;
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

void SheetView::drawBoxAround(int x,int y,int w,int h){
	wmove(win,y-1,x-1);
	waddch(win,'+');
	string hor(w,'-');
	waddstr(win,hor.data());
	waddch(win,'+');
	for(int i=y;i<y+h;i++){
		mvwaddch(win,i,x-1,'|');
		mvwaddch(win,i,x+w,'|');
	}
	mvwaddch(win,y+h,x-1,'+');
	waddstr(win,hor.data());
	waddch(win,'+');
}
