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

#include "materials.h"

#define MATERIAL_PROPERTIES_COUNT 256

// These correspond to the CONTENT_* constants
MaterialProperties g_material_properties[MATERIAL_PROPERTIES_COUNT];

bool g_material_properties_initialized = false;

void setStoneLikeDiggingProperties(u8 material, float toughness)
{
	g_material_properties[material].setDiggingProperties("",
			DiggingProperties(true, 15.0*toughness, 0));
	
	g_material_properties[material].setDiggingProperties("WPick",
			DiggingProperties(true, 1.3*toughness, 65535./30.*toughness));
	g_material_properties[material].setDiggingProperties("STPick",
			DiggingProperties(true, 0.75*toughness, 65535./100.*toughness));
	g_material_properties[material].setDiggingProperties("SteelPick",
			DiggingProperties(true, 0.50*toughness, 65535./333.*toughness));

	/*g_material_properties[material].setDiggingProperties("MesePick",
			DiggingProperties(true, 0.0*toughness, 65535./20.*toughness));*/
}

void setDirtLikeDiggingProperties(u8 material, float toughness)
{
	g_material_properties[material].setDiggingProperties("",
			DiggingProperties(true, 0.75*toughness, 0));
	
	g_material_properties[material].setDiggingProperties("WShovel",
			DiggingProperties(true, 0.4*toughness, 65535./50.*toughness));
	g_material_properties[material].setDiggingProperties("STShovel",
			DiggingProperties(true, 0.2*toughness, 65535./150.*toughness));
	g_material_properties[material].setDiggingProperties("SteelShovel",
			DiggingProperties(true, 0.15*toughness, 65535./400.*toughness));
}

void setWoodLikeDiggingProperties(u8 material, float toughness)
{
	g_material_properties[material].setDiggingProperties("",
			DiggingProperties(true, 3.0*toughness, 0));
	
	g_material_properties[material].setDiggingProperties("WAxe",
			DiggingProperties(true, 1.5*toughness, 65535./30.*toughness));
	g_material_properties[material].setDiggingProperties("STAxe",
			DiggingProperties(true, 0.75*toughness, 65535./100.*toughness));
	g_material_properties[material].setDiggingProperties("SteelAxe",
			DiggingProperties(true, 0.5*toughness, 65535./333.*toughness));
}

void initializeMaterialProperties()
{
	/*
		Now, the g_material_properties array is already initialized
		by the constructors to such that no digging is possible.

		Add some digging properties to them.
	*/

	setStoneLikeDiggingProperties(CONTENT_STONE, 1.0);
	setStoneLikeDiggingProperties(CONTENT_SANDSTONE, 1.0);
	setStoneLikeDiggingProperties(CONTENT_BRICK, 3.0);
	setStoneLikeDiggingProperties(CONTENT_MESE, 0.5);
	setStoneLikeDiggingProperties(CONTENT_COALSTONE, 1.5);
	setStoneLikeDiggingProperties(CONTENT_FURNACE, 3.0);
	setStoneLikeDiggingProperties(CONTENT_COBBLE, 1.0);
	setStoneLikeDiggingProperties(CONTENT_STEEL, 5.0);

	setDirtLikeDiggingProperties(CONTENT_MUD, 1.0);
	setDirtLikeDiggingProperties(CONTENT_GRASS, 1.0);
	setDirtLikeDiggingProperties(CONTENT_GRASS_FOOTSTEPS, 1.0);
	setDirtLikeDiggingProperties(CONTENT_SAND, 1.0);
	setDirtLikeDiggingProperties(CONTENT_CLAY, 1.0);
	
	setWoodLikeDiggingProperties(CONTENT_TREE, 1.0);
	setWoodLikeDiggingProperties(CONTENT_LEAVES, 0.15);
	setWoodLikeDiggingProperties(CONTENT_CACTUS, 0.75);
	setWoodLikeDiggingProperties(CONTENT_PAPYRUS, 0.25);
	setWoodLikeDiggingProperties(CONTENT_GLASS, 0.15);
	setWoodLikeDiggingProperties(CONTENT_FENCE, 0.75);
	setDirtLikeDiggingProperties(CONTENT_RAIL, 0.75);
	setWoodLikeDiggingProperties(CONTENT_WOOD, 0.75);
	setWoodLikeDiggingProperties(CONTENT_BOOKSHELF, 0.75);
	setWoodLikeDiggingProperties(CONTENT_CHEST, 1.0);
	setWoodLikeDiggingProperties(CONTENT_WORKBENCH, 1.0);

	g_material_properties[CONTENT_SIGN_WALL].setDiggingProperties("",
			DiggingProperties(true, 0.5, 0));
	g_material_properties[CONTENT_TORCH].setDiggingProperties("",
			DiggingProperties(true, 0.0, 0));
	
	/*
		Add MesePick to everything
	*/
	for(u16 i=0; i<MATERIAL_PROPERTIES_COUNT; i++)
	{
		g_material_properties[i].setDiggingProperties("MesePick",
				DiggingProperties(true, 0.0, 65535./1337));
	}

	g_material_properties_initialized = true;
}

MaterialProperties * getMaterialProperties(u8 material)
{
	assert(g_material_properties_initialized);
	return &g_material_properties[material];
}

DiggingProperties getDiggingProperties(u8 material, const std::string &tool)
{
	MaterialProperties *mprop = getMaterialProperties(material);
	if(mprop == NULL)
		// Not diggable
		return DiggingProperties();
	
	return mprop->getDiggingProperties(tool);
}

