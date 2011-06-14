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

#include "nodemetadata.h"
#include "utility.h"
#include "mapnode.h"
#include "exceptions.h"
#include "inventory.h"
#include <sstream>
#include "content_mapnode.h"

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
	NodeMetadataList
*/

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
	// Update at a fixed frequency
	const float interval = 0; // Can be instant
	m_step_accumulator += dtime;
	if (m_step_accumulator < interval)
		return false;
	m_step_accumulator -= interval;
	dtime = interval;

	InventoryList* clist = m_inventory->getList("workbench_craft");
	assert(clist);
	InventoryList* rlist = m_inventory->getList("workbench_craftresult");
	assert(rlist);

	if (rlist->getUsedSlots() != 0)
		m_crafted = false;

	if (m_crafted) {
		// We have crafted something so we clear our clist
		clist->clearItems();
		m_crafted = false;
		return false;
	}

	if (clist && rlist) {
		InventoryItem *items[WORKBENCH_SIZE];
		for (u16 i = 0; i < WORKBENCH_SIZE; i++) {
			items[i] = clist->getItem(i);
		}

		bool found = false;

		// Wood
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TREE);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				rlist->addItem(new MaterialItem(CONTENT_WOOD, 4));
				found = true;
			}
		}

		// Stick
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				rlist->addItem(new CraftItem("Stick", 4));
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
				rlist->addItem(new MaterialItem(CONTENT_FENCE, 2));
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
				//rlist->addItem(new MapBlockObjectItem("Sign"));
				rlist->addItem(new MaterialItem(CONTENT_SIGN_WALL, 1));
				found = true;
			}
		}

		// Torch
		if (!found) {
			ItemSpec specs[WORKBENCH_SIZE];
			specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			if (checkItemCombination(items, specs, WORKBENCH_SIZE)) {
				rlist->addItem(new MaterialItem(CONTENT_TORCH, 4));
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
				rlist->addItem(new ToolItem("WPick", 0));
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
				rlist->addItem(new ToolItem("STPick", 0));
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
				rlist->addItem(new ToolItem("SteelPick", 0));
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
				rlist->addItem(new ToolItem("MesePick", 0));
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
				rlist->addItem(new ToolItem("WShovel", 0));
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
				rlist->addItem(new ToolItem("STShovel", 0));
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
				rlist->addItem(new ToolItem("SteelShovel", 0));
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
				rlist->addItem(new ToolItem("WAxe", 0));
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
				rlist->addItem(new ToolItem("STAxe", 0));
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
				rlist->addItem(new ToolItem("SteelAxe", 0));
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
				rlist->addItem(new ToolItem("WSword", 0));
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
				rlist->addItem(new ToolItem("STSword", 0));
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
				rlist->addItem(new ToolItem("SteelSword", 0));
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
				rlist->addItem(new MaterialItem(CONTENT_CHEST, 1));
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
				rlist->addItem(new MaterialItem(CONTENT_RAIL, 15));
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
				rlist->addItem(new MaterialItem(CONTENT_WORKBENCH, 1));
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
				rlist->addItem(new MaterialItem(CONTENT_FURNACE, 1));
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
				rlist->addItem(new MaterialItem(CONTENT_STEEL, 1));
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
				rlist->addItem(new MaterialItem(CONTENT_SANDSTONE, 1));
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
				rlist->addItem(new MaterialItem(CONTENT_CLAY, 1));
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
				rlist->addItem(new MaterialItem(CONTENT_BRICK, 1));
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
				rlist->addItem(new CraftItem("paper", 1));
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
				rlist->addItem(new CraftItem("book", 1));
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
				rlist->addItem(new MaterialItem(CONTENT_BOOKSHELF, 1));
				found = true;
			}
		}

		if (found) {
			// If we found something we can clear the list
			m_crafted = true;
		}
		else {
			// Don't allow obtaining the craft result if the crafting pattern has changed
			rlist->clearItems();
		}
	}
	return true;
}
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
	}
	return something_changed;
}
