// Tom Smeding (s1685694), Tim Brouwer (s1663615), Ruben Turkenburg (s1659685)

#include <ncurses.h>
#include "controller.h"

SheetController::SheetController(string filename) : sheet(20, 20), view(sheet), fname(filename) {
	if (!filename.empty()) {
		sheet.loadFromDisk(fname);
	}
	view.redraw();
}

void SheetController::save() {
	bool success;
	if (!fname.empty()) {
		Maybe<string> mfname = view.askStringOfUser("Filename to save to:", fname);
		if (mfname.isNothing()) {
			return;
		}
		fname = mfname.fromJust();
		success = sheet.saveToDisk(fname);
	} else {
		Maybe<string> mfname = view.askStringOfUser("Filename to save to:", "");
		if (mfname.isNothing()) {
			return;
		}
		fname = mfname.fromJust();
		success = sheet.saveToDisk(fname);
	}
	if (!success) {
		view.displayStatusString("Save was unsuccessful. Please try again.");
	} else {
		view.displayStatusString("Saved successfully!");
	}
}

void SheetController::runloop() {
	while(true) {
		switch(view.getChar()) {
			case 'l': {
				bool success;
				if (!fname.empty()) {
					Maybe<string> mfname = view.askStringOfUser("Filename to load from:", fname);
					if (mfname.isNothing()) {
						break;
					}
					fname = mfname.fromJust();
					success = sheet.loadFromDisk(fname);
				} else {
					Maybe<string> mfname = view.askStringOfUser("Filename to load from:", "");
					if (mfname.isNothing()) {
						break;
					}
					fname = mfname.fromJust();
					success = sheet.loadFromDisk(fname);
				}
				if (!success) {
					view.displayStatusString("Load was unsuccessful. Please try again.");
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
				Maybe<string> mresponse = view.askStringOfUser("Would you like to save? (y/n)", "y");
				if (mresponse.isNothing()) {
					break;
				}
				if (mresponse.fromJust() == "y") {
					save();
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
