/*
 *Minetest-delta
 *Copyright (C) 2011 Free Software Foundation, Inc. <http://fsf.org/>
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

#include "nodemetadata.h"
#include "utility.h"
#include "mapnode.h"
#include "exceptions.h"
#include "inventory.h"
#include <sstream>

/*
	NodeMetadata
*/

core::map<u16, NodeMetadata::Factory> NodeMetadata::m_types;

NodeMetadata::NodeMetadata()
{
}

NodeMetadata::~NodeMetadata()
{
}

NodeMetadata* NodeMetadata::deSerialize(std::istream &is)
{
	// Read id
	u8 buf[2];
	is.read((char*)buf, 2);
	s16 id = readS16(buf);

	// Read data
	std::string data = deSerializeString(is);

	// Find factory function
	core::map<u16, Factory>::Node *n;
	n = m_types.find(id);
	if(n == NULL)
	{
		// If factory is not found, just return.
		dstream<<"WARNING: NodeMetadata: No factory for typeId="
		       <<id<<std::endl;
		return NULL;
	}

	// Try to load the metadata. If it fails, just return.
	try
	{
		std::istringstream iss(data, std::ios_base::binary);

		Factory f = n->getValue();
		NodeMetadata *meta = (*f)(iss);
		return meta;
	}
	catch(SerializationError &e)
	{
		dstream<<"WARNING: NodeMetadata: ignoring SerializationError"<<std::endl;
		return NULL;
	}
}

void NodeMetadata::serialize(std::ostream &os)
{
	u8 buf[2];
	writeU16(buf, typeId());
	os.write((char*)buf, 2);

	std::ostringstream oss(std::ios_base::binary);
	serializeBody(oss);
	os<<serializeString(oss.str());
}

void NodeMetadata::registerType(u16 id, Factory f)
{
	core::map<u16, Factory>::Node *n;
	n = m_types.find(id);
	if(n)
		return;
	m_types.insert(id, f);
}

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

/*
 WorkbenchNodeMetadata
 */

// Prototype
WorkbenchNodeMetadata proto_WorkbenchNodeMetadata;

WorkbenchNodeMetadata::WorkbenchNodeMetadata()
{
	NodeMetadata::registerType(typeId(), create);

	m_inventory = new Inventory();
	m_inventory->addList("workbench_craft", 3 * 3);
	m_inventory->addList("workbench_craftresult", 1);
	m_crafted = false;
}
WorkbenchNodeMetadata::~WorkbenchNodeMetadata()
{
	delete m_inventory;
}
u16 WorkbenchNodeMetadata::typeId() const
{
	return CONTENT_WORKBENCH;
}
NodeMetadata* WorkbenchNodeMetadata::create(std::istream &is)
{
	WorkbenchNodeMetadata *d = new WorkbenchNodeMetadata();
	d->m_inventory->deSerialize(is);
	d->m_inventory->getList("workbench_craftresult")->setReadOnly(true); // Set craft result as read only
	return d;
}
NodeMetadata* WorkbenchNodeMetadata::clone()
{
	WorkbenchNodeMetadata *d = new WorkbenchNodeMetadata();
	*d->m_inventory = *m_inventory;
	return d;
}
void WorkbenchNodeMetadata::serializeBody(std::ostream &os)
{
	m_inventory->serialize(os);
}
std::string WorkbenchNodeMetadata::infoText()
{
	return "Workbench";
}
bool WorkbenchNodeMetadata::nodeRemovalDisabled()
{
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

bool WorkbenchNodeMetadata::step(float dtime)
{
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

	if (m_crafted)
	{
		// We have crafted something, so we decrement the clist
		clist->decrementMaterials(1);
		m_crafted = false;
		return false;
	}

	if (clist && rlist)
	{
		InventoryItem *items[WORKBENCH_SIZE];
		for (u16 i = 0; i < WORKBENCH_SIZE; i++)
		{
			items[i] = clist->getItem(i);
		}

		bool found = false;

		InventoryItem *item = NULL;

		// Wood
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TREE);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_WOOD, 4);
				found = true;
			}
		}

		// Stick
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new CraftItem("Stick", 4);
				found = true;
			}
		}

		// Fence
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_FENCE, 2);
				found = true;
			}
		}

		// Sign
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				//item = new MapBlockObjectItem("Sign"));
				item = new MaterialItem(CONTENT_SIGN_WALL, 1);
				found = true;
			}
		}

		// Torch
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_TORCH, 4);
				found = true;
			}
		}

		// Wooden pick
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("WPick", 0);
				found = true;
			}
		}

		// Stone pick
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("STPick", 0);
				found = true;
			}
		}

		// Steel pick
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("SteelPick", 0);
				found = true;
			}
		}

		// Mese pick
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("MesePick", 0);
				found = true;
			}
		}

		// Wooden shovel
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("WShovel", 0);
				found = true;
			}
		}

		// Stone shovel
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("STShovel", 0);
				found = true;
			}
		}

		// Steel shovel
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("SteelShovel", 0);
				found = true;
			}
		}

		// Wooden axe
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("WAxe", 0);
				found = true;
			}
		}

		// Stone axe
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("STAxe", 0);
				found = true;
			}
		}

		// Steel axe
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("SteelAxe", 0);
				found = true;
			}
		}

		// Wooden sword
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("WSword", 0);
				found = true;
			}
		}

		// Stone sword
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("STSword", 0);
				found = true;
			}
		}

		// Steel sword
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new ToolItem("SteelSword", 0);
				found = true;
			}
		}

		// Chest
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_CHEST, 1);
				found = true;
			}
		}
		// Rail
		if (!found)
		{
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
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_RAIL, 15);
				found = true;
			}
		}

		// Workbench
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_WORKBENCH, 1);
				found = true;
			}
		}

		// Furnace
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_FURNACE, 1);
				found = true;
			}
		}

		// Steel block
		if (!found)
		{
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
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_STEEL, 1);
				found = true;
			}
		}

		// Sandstone
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_SANDSTONE, 1);
				found = true;
			}
		}

		// Clay
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			specs[4] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_CLAY, 1);
				found = true;
			}
		}

		// Brick
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_CRAFT, "clay_brick");
			specs[4] = ItemSpec(ITEM_CRAFT, "clay_brick");
			specs[6] = ItemSpec(ITEM_CRAFT, "clay_brick");
			specs[7] = ItemSpec(ITEM_CRAFT, "clay_brick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_BRICK, 1);
				found = true;
			}
		}

		// Paper
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new CraftItem("paper", 1);
				found = true;
			}
		}

		// Book
		if (!found)
		{
			ItemSpec specs[WORKBENCH_SIZE];
			specs[1] = ItemSpec(ITEM_CRAFT, "paper");
			specs[4] = ItemSpec(ITEM_CRAFT, "paper");
			specs[7] = ItemSpec(ITEM_CRAFT, "paper");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new CraftItem("book", 1);
				found = true;
			}
		}

		// Book shelf
		if (!found)
		{
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
			if (checkItemCombination(items, specs, WORKBENCH_SIZE))
			{
				item = new MaterialItem(CONTENT_BOOKSHELF, 1);
				found = true;
			}
		}

		// If the craft result doesn't match the current crafting pattern, clear it
		if(item != rlist->getItem(0))
			rlist->clearItems();

		if (found)
		{
			m_crafted = true;

			// If the craft result has not been added yet, do so
			if (rlist->getUsedSlots() == 0)
				rlist->addItem(item);
		}
	}
	return true;
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

	d->m_inventory->getList("dst")->setReadOnly(true); // Set result as read only

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
	// Update at a fixed frequency
	const float interval = 0.5;
	m_step_accumulator += dtime;
	if(m_step_accumulator < interval)
		return false;
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
		return true;
	}

	if(src_item == NULL || m_src_totaltime < 0.001)
	{
		return false;
	}

	bool changed = false;

	//dstream<<"Furnace is out of fuel"<<std::endl;

	InventoryList *fuel_list = m_inventory->getList("fuel");
	assert(fuel_list);
	InventoryItem *fuel_item = fuel_list->getItem(0);

	if(ItemSpec(ITEM_MATERIAL, CONTENT_TREE).checkItem(fuel_item))
	{
		m_fuel_totaltime = 10;
		m_fuel_time = 0;
		fuel_list->decrementMaterials(1);
		changed = true;
	}
	else if(ItemSpec(ITEM_MATERIAL, CONTENT_WOOD).checkItem(fuel_item))
	{
		m_fuel_totaltime = 5;
		m_fuel_time = 0;
		fuel_list->decrementMaterials(1);
		changed = true;
	}
	else if(ItemSpec(ITEM_CRAFT, "lump_of_coal").checkItem(fuel_item))
	{
		m_fuel_totaltime = 10;
		m_fuel_time = 0;
		fuel_list->decrementMaterials(1);
		changed = true;
	}
	else
	{
		//dstream<<"No fuel found"<<std::endl;
	}

	return changed;
}

/*
	NodeMetadatalist
*/

void NodeMetadataList::serialize(std::ostream &os)
{
	u8 buf[6];

	u16 version = 1;
	writeU16(buf, version);
	os.write((char*)buf, 2);

	u16 count = m_data.size();
	writeU16(buf, count);
	os.write((char*)buf, 2);

	for(core::map<v3s16, NodeMetadata*>::Iterator
	        i = m_data.getIterator();
	        i.atEnd()==false; i++)
	{
		v3s16 p = i.getNode()->getKey();
		NodeMetadata *data = i.getNode()->getValue();

		u16 p16 = p.Z*MAP_BLOCKSIZE*MAP_BLOCKSIZE + p.Y*MAP_BLOCKSIZE + p.X;
		writeU16(buf, p16);
		os.write((char*)buf, 2);

		data->serialize(os);
	}

}
void NodeMetadataList::deSerialize(std::istream &is)
{
	m_data.clear();

	u8 buf[6];

	is.read((char*)buf, 2);
	u16 version = readU16(buf);

	if(version > 1)
	{
		dstream<<__FUNCTION_NAME<<": version "<<version<<" not supported"
		       <<std::endl;
		throw SerializationError("NodeMetadataList::deSerialize");
	}

	is.read((char*)buf, 2);
	u16 count = readU16(buf);

	for(u16 i=0; i<count; i++)
	{
		is.read((char*)buf, 2);
		u16 p16 = readU16(buf);

		v3s16 p(0,0,0);
		p.Z += p16 / MAP_BLOCKSIZE / MAP_BLOCKSIZE;
		p16 -= p.Z * MAP_BLOCKSIZE * MAP_BLOCKSIZE;
		p.Y += p16 / MAP_BLOCKSIZE;
		p16 -= p.Y * MAP_BLOCKSIZE;
		p.X += p16;

		NodeMetadata *data = NodeMetadata::deSerialize(is);

		if(data == NULL)
			continue;

		if(m_data.find(p))
		{
			dstream<<"WARNING: NodeMetadataList::deSerialize(): "
			       <<"already set data at position"
			       <<"("<<p.X<<","<<p.Y<<","<<p.Z<<"): Ignoring."
			       <<std::endl;
			delete data;
			continue;
		}

		m_data.insert(p, data);
	}
}

NodeMetadataList::~NodeMetadataList()
{
	for(core::map<v3s16, NodeMetadata*>::Iterator
	        i = m_data.getIterator();
	        i.atEnd()==false; i++)
	{
		delete i.getNode()->getValue();
	}
}

NodeMetadata* NodeMetadataList::get(v3s16 p)
{
	core::map<v3s16, NodeMetadata*>::Node *n;
	n = m_data.find(p);
	if(n == NULL)
		return NULL;
	return n->getValue();
}

void NodeMetadataList::remove(v3s16 p)
{
	NodeMetadata *olddata = get(p);
	if(olddata)
	{
		delete olddata;
		m_data.remove(p);
	}
}

void NodeMetadataList::set(v3s16 p, NodeMetadata *d)
{
	remove(p);
	m_data.insert(p, d);
}

bool NodeMetadataList::step(float dtime)
{
	bool something_changed = false;
	for(core::map<v3s16, NodeMetadata*>::Iterator
	        i = m_data.getIterator();
	        i.atEnd()==false; i++)
	{
		v3s16 p = i.getNode()->getKey();
		NodeMetadata *meta = i.getNode()->getValue();
		bool changed = meta->step(dtime);
		if(changed)
			something_changed = true;
		/*if(res.inventory_changed)
		{
			std::string inv_id;
			inv_id += "nodemeta:";
			inv_id += itos(p.X);
			inv_id += ",";
			inv_id += itos(p.Y);
			inv_id += ",";
			inv_id += itos(p.Z);
			InventoryContext c;
			c.current_player = NULL;
			inv_mgr->inventoryModified(&c, inv_id);
		}*/
	}
	return something_changed;
}

