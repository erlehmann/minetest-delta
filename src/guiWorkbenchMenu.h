/*
 *Minetest-delta
 *Copyright (C) 2011 Sebastian Rühl <https://launchpad.net/~sebastian-ruehl>
 *Copyright (C) 2011 MirceaKitsune <https://github.com/MirceaKitsune>
 *
 *This program is free software: you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation, either version 2 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GUIWORKBENCHMENU_H_
#define GUIWORKBENCHMENU_H_

#include "guiInventoryMenu.h"

class Client;

class GUIWorkbenchMenu: public GUIInventoryMenu {
public:
	GUIWorkbenchMenu(gui::IGUIEnvironment* env, gui::IGUIElement* parent,
			s32 id, IMenuManager *menumgr, v3s16 nodepos, Client *client);
private:

	v3s16 m_nodepos;
	Client *m_client;
};

#endif /* GUIWORKBENCHMENU_H_ */
