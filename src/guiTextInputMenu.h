/*
Minetest-c55
Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef GUITEXTINPUTMENU_HEADER
#define GUITEXTINPUTMENU_HEADER

#include "common_irrlicht.h"
#include "modalMenu.h"
#include "utility.h"
#include <string>

struct TextDest
{
	virtual void gotText(std::wstring text) = 0;
};

class GUITextInputMenu : public GUIModalMenu
{
public:
	GUITextInputMenu(gui::IGUIEnvironment* env,
	                 gui::IGUIElement* parent, s32 id,
	                 IMenuManager *menumgr,
	                 TextDest *dest,
	                 std::wstring initial_text);
	~GUITextInputMenu();

	void removeChildren();
	/*
		Remove and re-add (or reposition) stuff
	*/
	void regenerateGui(v2u32 screensize);

	void drawMenu();

	void acceptInput();

	bool OnEvent(const SEvent& event);

private:
	TextDest *m_dest;
	std::wstring m_initial_text;
};

#endif

