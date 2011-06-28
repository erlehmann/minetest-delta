/*
Minetest-c55
Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "content_nodemeta.h"
#include "inventory.h"
#include "content_mapnode.h"

/*
	SignNodeMetadata
*/

// Prototype
SignNodeMetadata proto_SignNodeMetadata("");

SignNodeMetadata::SignNodeMetadata(std::string text):
	m_text(text)
{
	NodeMetadata::registerType(typeId(), create);
}
u16 SignNodeMetadata::typeId() const
{
	return CONTENT_SIGN_WALL;
}
NodeMetadata* SignNodeMetadata::create(std::istream &is)
{
	std::string text = deSerializeString(is);
	return new SignNodeMetadata(text);
}
NodeMetadata* SignNodeMetadata::clone()
{
	return new SignNodeMetadata(m_text);
}
void SignNodeMetadata::serializeBody(std::ostream &os)
{
	os<<serializeString(m_text);
}
std::string SignNodeMetadata::infoText()
{
	return std::string("\"")+m_text+"\"";
}

/*
	ChestNodeMetadata
*/

// Prototype
ChestNodeMetadata proto_ChestNodeMetadata;

ChestNodeMetadata::ChestNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);
	
	m_inventory = new Inventory();
	m_inventory->addList("0", 8*4);
}
ChestNodeMetadata::~ChestNodeMetadata()
{
	delete m_inventory;
}
u16 ChestNodeMetadata::typeId() const
{
	return CONTENT_CHEST;
}
NodeMetadata* ChestNodeMetadata::create(std::istream &is)
{
	ChestNodeMetadata *d = new ChestNodeMetadata();
	d->m_inventory->deSerialize(is);
	return d;
}
NodeMetadata* ChestNodeMetadata::clone()
{
	ChestNodeMetadata *d = new ChestNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void ChestNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
}
std::string ChestNodeMetadata::infoText()
{
	return "Chest";
}
bool ChestNodeMetadata::nodeRemovalDisabled()
{
	/*
		Disable removal if chest contains something
	*/
	InventoryList *list = m_inventory->getList("0");
	if(list == NULL)
		return false;
	if(list->getUsedSlots() == 0)
		return false;
	return true;
}
std::string ChestNodeMetadata::getInventoryDrawSpecString()
{
	return
		"invsize[8,9;]"
		"list[current_name;0;0,0;8,4;]"
		"list[current_player;main;0,5;8,4;]";
}

/*
 WorkbenchNodeMetadata
 */

// Prototype
WorkbenchNodeMetadata proto_WorkbenchNodeMetadata;

WorkbenchNodeMetadata::WorkbenchNodeMetadata() {
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("workbench_craft", 3 * 3);
	m_inventory->addList("workbench_craftresult", 1);
	m_crafted = false;
}
WorkbenchNodeMetadata::~WorkbenchNodeMetadata() {
	delete m_inventory;
}
u16 WorkbenchNodeMetadata::typeId() const {
	return CONTENT_WORKBENCH;
}
NodeMetadata* WorkbenchNodeMetadata::create(std::istream &is) {
	WorkbenchNodeMetadata *d = new WorkbenchNodeMetadata();
	d->m_inventory->deSerialize(is);
	d->m_inventory->getList("workbench_craftresult")->setReadOnly(true); // Set craft result as read only
	return d;
}
NodeMetadata* WorkbenchNodeMetadata::clone() {
	WorkbenchNodeMetadata *d = new WorkbenchNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void WorkbenchNodeMetadata::serializeBody(std::ostream &os) {
	m_inventory->serialize(os);
}
std::string WorkbenchNodeMetadata::infoText() {
	return "Workbench";
}
bool WorkbenchNodeMetadata::nodeRemovalDisabled() {
	/*
	 Disable removal if workbench contains something
	 */
	InventoryList *list = m_inventory->getList("workbench_craft");
	if (list == NULL)
		return false;
	if (list->getUsedSlots() == 0)
		return false;
	return true;
}

bool WorkbenchNodeMetadata::step(float dtime) {
	// This can be instant
	/* Update at a fixed frequency
	const float interval = 0.5;
	m_step_accumulator += dtime;
	if (m_step_accumulator < interval)
		return false;
	m_step_accumulator -= interval;
	dtime = interval; */

	InventoryList* clist = m_inventory->getList("workbench_craft");
	assert(clist);
	InventoryList* rlist = m_inventory->getList("workbench_craftresult");
	assert(rlist);

	if (rlist->getUsedSlots() != 0)
		m_crafted = false;

	if (m_crafted) {
		// We have crafted something, so we decrement the clist
		clist->decrementMaterials(1);
		m_crafted = false;
		return false;
	}

	if (clist && rlist) {
		InventoryItem *items[WORKBENCH_SIZE];
		for (u16 i = 0; i < WORKBENCH_SIZE; i++) {
			items[i] = clist->getItem(i);
		}

		bool found = false;

		InventoryItem *item = NULL;
/*
		// Wood
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TREE);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_WOOD, 4);
				found = true;
			}
		}

		// Stick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new CraftItem("Stick", 4);
				found = true;
			}
		}

		// Fence
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_FENCE, 2);
				found = true;
			}
		}

		// Sign
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				//item = new MapBlockObjectItem("Sign"));
				item = new MaterialItem(CONTENT_SIGN_WALL, 1);
				found = true;
			}
		}

		// Torch
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_TORCH, 4);
				found = true;
			}
		}

		// Wooden pick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("WPick", 0);
				found = true;
			}
		}

		// Stone pick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("STPick", 0);
				found = true;
			}
		}

		// Steel pick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("SteelPick", 0);
				found = true;
			}
		}

		// Mese pick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("MesePick", 0);
				found = true;
			}
		}

		// Wooden shovel
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("WShovel", 0);
				found = true;
			}
		}

		// Stone shovel
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("STShovel", 0);
				found = true;
			}
		}

		// Steel shovel
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("SteelShovel", 0);
				found = true;
			}
		}

		// Wooden axe
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("WAxe", 0);
				found = true;
			}
		}

		// Stone axe
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("STAxe", 0);
				found = true;
			}
		}

		// Steel axe
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("SteelAxe", 0);
				found = true;
			}
		}

		// Wooden sword
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("WSword", 0);
				found = true;
			}
		}

		// Stone sword
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("STSword", 0);
				found = true;
			}
		}

		// Steel sword
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new ToolItem("SteelSword", 0);
				found = true;
			}
		}

		// Chest
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_CHEST, 1);
				found = true;
			}
		}
		// Rail
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[1] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_RAIL, 15);
				found = true;
			}
		}

		// Workbench
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_WORKBENCH, 1);
				found = true;
			}
		}

		// Furnace
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_FURNACE, 1);
				found = true;
			}
		}

		// Steel block
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[7] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_STEEL, 1);
				found = true;
			}
		}

		// Sandstone
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_SANDSTONE, 1);
				found = true;
			}
		}

		// Clay
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			specs[4] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_CLAY, 1);
				found = true;
			}
		}

		// Brick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_CRAFT, "clay_brick");
			specs[4] = ItemSpec(ITEM_CRAFT, "clay_brick");
			specs[6] = ItemSpec(ITEM_CRAFT, "clay_brick");
			specs[7] = ItemSpec(ITEM_CRAFT, "clay_brick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_BRICK, 1);
				found = true;
			}
		}

		// Paper
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new CraftItem("paper", 1);
				found = true;
			}
		}

		// Book
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_CRAFT, "paper");
			specs[4] = ItemSpec(ITEM_CRAFT, "paper");
			specs[7] = ItemSpec(ITEM_CRAFT, "paper");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new CraftItem("book", 1);
				found = true;
			}
		}

		// Book shelf
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_CRAFT, "book");
			specs[4] = ItemSpec(ITEM_CRAFT, "book");
			specs[5] = ItemSpec(ITEM_CRAFT, "book");
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				item = new MaterialItem(CONTENT_BOOKSHELF, 1);
				found = true;
			}
		}
*/
		// If the craft result doesn't match the current crafting pattern, clear it
		if(item != rlist->getItem(0))
			rlist->clearItems();

		if (found) {
			m_crafted = true;

			// If the craft result has not been added yet, do so
			if (rlist->getUsedSlots() == 0)
				rlist->addItem(item);
		}
	}
	return true;
}
std::string WorkbenchNodeMetadata::getInventoryDrawSpecString()
{
	return
		"invsize[8,8;]"
		"list[current_name;workbench_craft;2,0;3,3;]"
		"list[current_name;workbench_craftresult;7,1;1,1;]"
		"list[current_player;main;0,4;8,4;]";
}

/*
	FurnaceNodeMetadata
*/

// Prototype
FurnaceNodeMetadata proto_FurnaceNodeMetadata;

FurnaceNodeMetadata::FurnaceNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);
	
	m_inventory = new Inventory();
	m_inventory->addList("fuel", 1);
	m_inventory->addList("src", 1);
	m_inventory->addList("dst", 4);

	m_step_accumulator = 0;
	m_fuel_totaltime = 0;
	m_fuel_time = 0;
	m_src_totaltime = 0;
	m_src_time = 0;
}
FurnaceNodeMetadata::~FurnaceNodeMetadata()
{
	delete m_inventory;
}
u16 FurnaceNodeMetadata::typeId() const
{
	return CONTENT_FURNACE;
}
NodeMetadata* FurnaceNodeMetadata::clone()
{
	FurnaceNodeMetadata *d = new FurnaceNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
NodeMetadata* FurnaceNodeMetadata::create(std::istream &is)
{
	FurnaceNodeMetadata *d = new FurnaceNodeMetadata();

	d->m_inventory->deSerialize(is);

	int temp;
	is>>temp;
	d->m_fuel_totaltime = (float)temp/10;
	is>>temp;
	d->m_fuel_time = (float)temp/10;

	return d;
}
void FurnaceNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
	os<<itos(m_fuel_totaltime*10)<<" ";
	os<<itos(m_fuel_time*10)<<" ";
}
std::string FurnaceNodeMetadata::infoText()
{
	//return "Furnace";
	if(m_fuel_time >= m_fuel_totaltime)
	{
		InventoryList *src_list = m_inventory->getList("src");
		assert(src_list);
		InventoryItem *src_item = src_list->getItem(0);

		if(src_item)
			return "Furnace is out of fuel";
		else
			return "Furnace is inactive";
	}
	else
	{
		std::string s = "Furnace is active (";
		s += itos(m_fuel_time/m_fuel_totaltime*100);
		s += "%)";
		return s;
	}
}
void FurnaceNodeMetadata::inventoryModified()
{
	dstream<<"Furnace inventory modification callback"<<std::endl;
}
bool FurnaceNodeMetadata::step(float dtime)
{
	if(dtime > 60.0)
		dstream<<"Furnace stepping a long time ("<<dtime<<")"<<std::endl;
	// Update at a fixed frequency
	const float interval = 2.0;
	m_step_accumulator += dtime;
	bool changed = false;
	while(m_step_accumulator > interval)
	{
		m_step_accumulator -= interval;
		dtime = interval;

		//dstream<<"Furnace step dtime="<<dtime<<std::endl;
		
		InventoryList *dst_list = m_inventory->getList("dst");
		assert(dst_list);

		InventoryList *src_list = m_inventory->getList("src");
		assert(src_list);
		InventoryItem *src_item = src_list->getItem(0);
		
		// Start only if there are free slots in dst, so that it can
		// accomodate any result item
		if(dst_list->getFreeSlots() > 0 && src_item && src_item->isCookable())
		{
			m_src_totaltime = 3;
		}
		else
		{
			m_src_time = 0;
			m_src_totaltime = 0;
		}
		
		/*
			If fuel is burning, increment the burn counters.
			If item finishes cooking, move it to result.
		*/
		if(m_fuel_time < m_fuel_totaltime)
		{
			//dstream<<"Furnace is active"<<std::endl;
			m_fuel_time += dtime;
			m_src_time += dtime;
			if(m_src_time >= m_src_totaltime && m_src_totaltime > 0.001
					&& src_item)
			{
				InventoryItem *cookresult = src_item->createCookResult();
				dst_list->addItem(cookresult);
				src_list->decrementMaterials(1);
				m_src_time = 0;
				m_src_totaltime = 0;
			}
			changed = true;
			continue;
		}
		
		/*
			If there is no source item or source item is not cookable, stop loop.
		*/
		if(src_item == NULL || m_src_totaltime < 0.001)
		{
			m_step_accumulator = 0;
			break;
		}
		
		//dstream<<"Furnace is out of fuel"<<std::endl;

		InventoryList *fuel_list = m_inventory->getList("fuel");
		assert(fuel_list);
		InventoryItem *fuel_item = fuel_list->getItem(0);

		if(ItemSpec(ITEM_MATERIAL, CONTENT_TREE).checkItem(fuel_item))
		{
			m_fuel_totaltime = 30;
			m_fuel_time = 0;
			fuel_list->decrementMaterials(1);
			changed = true;
		}
		else if(ItemSpec(ITEM_MATERIAL, CONTENT_WOOD).checkItem(fuel_item))
		{
			m_fuel_totaltime = 30/4;
			m_fuel_time = 0;
			fuel_list->decrementMaterials(1);
			changed = true;
		}
		else if(ItemSpec(ITEM_CRAFT, "Stick").checkItem(fuel_item))
		{
			m_fuel_totaltime = 30/4/4;
			m_fuel_time = 0;
			fuel_list->decrementMaterials(1);
			changed = true;
		}
		else if(ItemSpec(ITEM_CRAFT, "lump_of_coal").checkItem(fuel_item))
		{
			m_fuel_totaltime = 40;
			m_fuel_time = 0;
			fuel_list->decrementMaterials(1);
			changed = true;
		}
		else
		{
			//dstream<<"No fuel found"<<std::endl;
			// No fuel, stop loop.
			m_step_accumulator = 0;
			break;
		}
	}
	return changed;
}
std::string FurnaceNodeMetadata::getInventoryDrawSpecString()
{
	return
		"invsize[8,9;]"
		"list[current_name;fuel;2,3;1,1;]"
		"list[current_name;src;2,1;1,1;]"
		"list[current_name;dst;5,1;2,2;]"
		"list[current_player;main;0,5;8,4;]";
}


