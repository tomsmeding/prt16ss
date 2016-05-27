#include <ncurses.h>
#include "controller.h"

SheetController::SheetController(string filename) : sheet(20, 20), view(sheet), fname(filename) {
	if (!filename.empty()) {
		sheet.loadFromDisk(fname);
	}
	view.redraw();
}

bool SheetController::save() {
	bool success;
	Maybe<string> mfname = view.askStringOfUser("File to save to:", "");
	if (mfname.isNothing()) {
		return false;
	}
	fname = mfname.fromJust();
	success = sheet.saveToDisk(fname);

	if (!success) {
		view.displayStatusString("Error while saving!");
	} else {
		view.displayStatusString("Saved.");
	}
	return success;
}

void SheetController::runloop() {
	while(true) {
		switch(view.getChar()) {
			case 'l': {
				bool success;
				Maybe<string> mfname = view.askStringOfUser("File to load from:", "");
				if (mfname.isNothing()) {
					break;
				}
				fname = mfname.fromJust();
				success = sheet.loadFromDisk(fname);

				if (!success) {
					view.displayStatusString("Error while loading!");
				}
				view.redraw();
				break;
			}
				
			case 's': {
				save();
				break;
			}
				
			case 'q': {
				//Ask the user to save before return? Default value still needs to be set
				Maybe<string> mresponse = view.askStringOfUser("Would you like to save? (Y/n)", "");
				if (mresponse.isNothing()) {
					view.displayStatusString("Cancelled");
					break;
				}
				const string &response = mresponse.fromJust();
				char res = response.size() == 0 ? 'y' : response[0];
				if (res == 'Y' || res == 'y') {
					if (!save()) break;
				} else if (res != 'N' && res != 'n') {
					view.displayStatusString("Cancelled");
					break;
				}
				return;
			}
				
			case KEY_UP: {
				CellAddress addr = view.getCursorPosition();
				if (addr.row == 0) {
					break;
				}
				addr.row -= 1;
				view.setCursorPosition(addr);
				break;
			}
			
			case KEY_DOWN: {
				CellAddress addr = view.getCursorPosition();
				addr.row += 1;
				view.setCursorPosition(addr);
				break;
			}
				
			case KEY_LEFT: {
				CellAddress addr = view.getCursorPosition();
				if (addr.column == 0) {
					break;
				}
				addr.column -= 1;
				view.setCursorPosition(addr);
				break;
			}
				
			case KEY_RIGHT: {
				CellAddress addr = view.getCursorPosition();
				addr.column += 1;
				view.setCursorPosition(addr);
				break;
			}
				
			case KEY_BACKSPACE: {
				set<CellAddress> changed = sheet.changeCellValue(view.getCursorPosition(), "").fromJust();
				for (CellAddress cell : changed) {
					view.redrawCell(cell);
				}
				break;
			}
			
			case '\n': {
				Maybe<string> currval = sheet.getCellEditString(view.getCursorPosition());
				if (currval.isNothing()) {
					break;
				}
				string currvalstring = currval.fromJust();
				Maybe<string> editval = view.getStringWithEditWindowOverCell(view.getCursorPosition(), currvalstring);
				if (editval.isNothing()) {
					break;
				}
				string editvalstring = editval.fromJust();
				set<CellAddress> changed = sheet.changeCellValue(view.getCursorPosition(), editvalstring).fromJust();
				for (CellAddress cell : changed) {
					view.redrawCell(cell);
				}
				break;
			}
				
			default: {
				break;
			}
		}
	}
}
