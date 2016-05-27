#include <unordered_map>
#include <functional>
#include <ncurses.h>
#include "controller.h"

using namespace std;

const unordered_map<string,function<SheetController::CommandRet(SheetController&)>> SheetController::commands={
	{"quit",[](SheetController &self){
		if (self.sheet.isClobbered()) {
			Maybe<string> mresponse =
				self.view.askStringOfUser("Would you like to save? (Y/n)", "", true, true);
			if (mresponse.isNothing()) {
				self.view.displayStatusString("Cancelled");
				return CR_CANCELLED;
			}
			const string &response = mresponse.fromJust();
			char res = response.size() == 0 ? 'y' : response[0];
			if (res == 'Y' || res == 'y') {
				CommandRet ret = commands.at("save")(self);
				if (ret != CR_OK) return ret;
			} else if (res != 'N' && res != 'n') {
				self.view.displayStatusString("Cancelled");
				return CR_CANCELLED;
			}
		}
		return CR_QUIT;
	}},
	{"q",[](SheetController &self){return commands.at("quit")(self);}},

	{"save",[](SheetController &self){
		bool success;
		Maybe<string> mfname = self.view.askStringOfUser("File to save to:", self.fname);
		if (mfname.isNothing()) {
			return CR_CANCELLED;
		}
		self.fname = mfname.fromJust();
		success = self.sheet.saveToDisk(self.fname);

		if (!success) {
			self.view.displayStatusString("Error while saving!");
			return CR_FAIL;
		} else {
			self.view.displayStatusString("Saved.");
			return CR_OK;
		}
	}},
	{"s",[](SheetController &self){return commands.at("save")(self);}},
	{"write",[](SheetController &self){return commands.at("save")(self);}},
	{"w",[](SheetController &self){return commands.at("save")(self);}},

	{"load",[](SheetController &self){
		if (self.sheet.isClobbered()) {
			Maybe<string> mresponse = self.view.askStringOfUser("Save unsaved changes before loading? (Y/n) ", "", true, true);
			if (mresponse.isNothing() || mresponse.fromJust().size() == 0) {
				return CR_CANCELLED;
			}
			const char res = mresponse.fromJust()[0];
			if (res != 'n' && res != 'N') {
				if (commands.at("save")(self) == CR_CANCELLED) {
					return CR_CANCELLED;
				}
			}
		}
		bool success;
		Maybe<string> mfname = self.view.askStringOfUser("File to load from:", self.fname);
		if (mfname.isNothing()) {
			return CR_CANCELLED;
		}
		self.fname = mfname.fromJust();
		success = self.sheet.loadFromDisk(self.fname);

		if (!success) {
			self.view.displayStatusString("Error while loading!");
			return CR_FAIL;
		}
		self.view.redraw();
		return CR_OK;
	}},
	{"l",[](SheetController &self){return commands.at("load")(self);}},
	{"edit",[](SheetController &self){return commands.at("load")(self);}},
	{"e",[](SheetController &self){return commands.at("load")(self);}},
};

SheetController::SheetController(string filename) : sheet(20, 20), view(sheet), fname(filename) {
	if (!filename.empty()) {
		sheet.loadFromDisk(fname);
	}
	view.redraw();
}

void SheetController::runloop() {
	while(true) {
		const int keychar = view.getChar();
		switch(keychar) {
			case ':': {
				Maybe<string> mcommand = view.askStringOfUser(":", "", false);
				if (mcommand.isNothing()) break;
				auto it = commands.find(mcommand.fromJust());
				if (it == commands.end()) {
					view.displayStatusString("Command '" + mcommand.fromJust() + "' not found!");
					break;
				}
				if (it->second(*this) == CR_QUIT) {
					return;
				}
				break;
			}

			case KEY_UP: {
				CellAddress addr = view.getCursorPosition();
				if (addr.row == 0) break;
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
				if (addr.column == 0) break;
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

			default: {
				if (keychar != '\n' && (keychar < 32 || keychar > 126)) {
					beep();
					break;
				}
				CellAddress curpos = view.getCursorPosition();
				Maybe<string> currval = sheet.getCellEditString(curpos);
				if (currval.isNothing()) break;
				string currvalstring = currval.fromJust();
				if (keychar != '\n') {
					currvalstring = string(1, keychar);
				}
				Maybe<string> editval = view.getStringWithEditWindowOverCell(curpos, currvalstring);
				if (editval.isNothing()) break;
				string editvalstring = editval.fromJust();
				set<CellAddress> changed = sheet.changeCellValue(curpos, editvalstring).fromJust();
				for (CellAddress cell : changed) {
					view.redrawCell(cell);
				}

				curpos.row++;
				view.setCursorPosition(curpos);
				break;
			}
		}
	}
}
