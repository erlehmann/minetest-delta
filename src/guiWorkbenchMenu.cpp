/*
 *Minetest-delta
 *Copyright (C) 2011 Free Software Foundation, Inc. <http://fsf.org/>
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

#include "guiWorkbenchMenu.h"
#include "client.h"

GUIWorkbenchMenu::GUIWorkbenchMenu(gui::IGUIEnvironment* env,
		gui::IGUIElement* parent, s32 id, IMenuManager *menumgr, v3s16 nodepos,
		Client *client) :
	GUIInventoryMenu(env, parent, id, menumgr, v2s16(8, 9),
			client->getInventoryContext(), client), m_nodepos(nodepos),
			m_client(client) {

	std::string workbench_inv_id;
	workbench_inv_id += "nodemeta:";
	workbench_inv_id += itos(nodepos.X);
	workbench_inv_id += ",";
	workbench_inv_id += itos(nodepos.Y);
	workbench_inv_id += ",";
	workbench_inv_id += itos(nodepos.Z);

	core::array<GUIInventoryMenu::DrawSpec> draw_spec;

	draw_spec.push_back(GUIInventoryMenu::DrawSpec("list", workbench_inv_id,
			"workbench_craft", v2s32(3, 0), v2s32(3, 3)));
	draw_spec.push_back(GUIInventoryMenu::DrawSpec("list", workbench_inv_id,
			"workbench_craftresult", v2s32(7, 1), v2s32(1, 1)));
	draw_spec.push_back(GUIInventoryMenu::DrawSpec("list", "current_player",
			"main", v2s32(0, 3), v2s32(8, 4)));

	setDrawSpec(draw_spec);

	Inventory *inv_to = m_invmgr->getInventory(m_c, workbench_inv_id);
	InventoryList *list_to = inv_to->getList("workbench_craftresult");
	list_to->setReadOnly(true);
}

