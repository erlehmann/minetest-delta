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

/*
=============================== NOTES ==============================
NOTE: Things starting with TODO are sometimes only suggestions.

NOTE: iostream.imbue(std::locale("C")) is very slow
NOTE: Global locale is now set at initialization

NOTE: If VBO (EHM_STATIC) is used, remember to explicitly free the
      hardware buffer (it is not freed automatically)

Old, wild and random suggestions that probably won't be done:
-------------------------------------------------------------

SUGG: If player is on ground, mainly fetch ground-level blocks

SUGG: Expose Connection's seqnums and ACKs to server and client.
      - This enables saving many packets and making a faster connection
	  - This also enables server to check if client has received the
	    most recent block sent, for example.
SUGG: Add a sane bandwidth throttling system to Connection

SUGG: More fine-grained control of client's dumping of blocks from
      memory
	  - ...What does this mean in the first place?

SUGG: A map editing mode (similar to dedicated server mode)

SUGG: Transfer more blocks in a single packet
SUGG: A blockdata combiner class, to which blocks are added and at
      destruction it sends all the stuff in as few packets as possible.
SUGG: Make a PACKET_COMBINED which contains many subpackets. Utilize
      it by sending more stuff in a single packet.
	  - Add a packet queue to RemoteClient, from which packets will be
	    combined with object data packets
		- This is not exactly trivial: the object data packets are
		  sometimes very big by themselves
	  - This might not give much network performance gain though.

SUGG: Precalculate lighting translation table at runtime (at startup)
      - This is not doable because it is currently hand-made and not
	    based on some mathematical function.
		- Note: This has been changing lately

SUGG: A version number to blocks, which increments when the block is
      modified (node add/remove, water update, lighting update)
	  - This can then be used to make sure the most recent version of
	    a block has been sent to client, for example

SUGG: Make the amount of blocks sending to client and the total
	  amount of blocks dynamically limited. Transferring blocks is the
	  main network eater of this system, so it is the one that has
	  to be throttled so that RTTs stay low.

SUGG: Meshes of blocks could be split into 6 meshes facing into
      different directions and then only those drawn that need to be

SUGG: Calculate lighting per vertex to get a lighting effect like in
      bartwe's game

SUGG: Background music based on cellular automata?
      http://www.earslap.com/projectslab/otomata

SUGG: Simple light color information to air

SUGG: Server-side objects could be moved based on nodes to enable very
      lightweight operation and simple AI
	- Not practical; client would still need to show smooth movement.

SUGG: Make a system for pregenerating quick information for mapblocks, so
	  that the client can show them as cubes before they are actually sent
	  or even generated.

Gaming ideas:
-------------

- Aim for something like controlling a single dwarf in Dwarf Fortress
- The player could go faster by a crafting a boat, or riding an animal
- Random NPC traders. what else?

Game content:
-------------

- When furnace is destroyed, move items to player's inventory
- Add lots of stuff
- Glass blocks
- Growing grass, decaying leaves
	- This can be done in the active blocks I guess.
	- Lots of stuff can be done in the active blocks.
	- Uh, is there an active block list somewhere? I think not. Add it.
- Breaking weak structures
	- This can probably be accomplished in the same way as grass
- Player health points
	- When player dies, throw items on map (needs better item-on-map
	  implementation)
- Cobble to get mossy if near water
- More slots in furnace source list, so that multiple ingredients
  are possible.
- Keys to chests?

- The Treasure Guard; a big monster with a hammer
	- The hammer does great damage, shakes the ground and removes a block
	- You can drop on top of it, and have some time to attack there
	  before he shakes you off

- Maybe the difficulty could come from monsters getting tougher in
  far-away places, and the player starting to need something from
  there when time goes by.
  - The player would have some of that stuff at the beginning, and
    would need new supplies of it when it runs out

- A bomb
- A spread-items-on-map routine for the bomb, and for dying players

- Fighting:
  - Proper sword swing simulation
  - Player should get damage from colliding to a wall at high speed

Documentation:
--------------

Build system / running:
-----------------------

Networking and serialization:
-----------------------------

SUGG: Fix address to be ipv6 compatible

User Interface:
---------------

Graphics:
---------

SUGG: Combine MapBlock's face caches to so big pieces that VBO
      can be used
      - That is >500 vertices
	  - This is not easy; all the MapBlocks close to the player would
	    still need to be drawn separately and combining the blocks
		would have to happen in a background thread

SUGG: Make fetching sector's blocks more efficient when rendering
      sectors that have very large amounts of blocks (on client)
	  - Is this necessary at all?

TODO: Flowing water animation

SUGG: Draw cubes in inventory directly with 3D drawing commands, so that
      animating them is easier.

SUGG: Option for enabling proper alpha channel for textures
TODO: A setting for enabling bilinear filtering for textures

TODO: Better control of draw_control.wanted_max_blocks

TODO: Block mesh generator to tile properly on smooth lighting

Configuration:
--------------

Client:
-------

TODO: Untie client network operations from framerate
      - Needs some input queues or something
	  - This won't give much performance boost because calculating block
	    meshes takes so long

SUGG: Make morning and evening transition more smooth and maybe shorter

TODO: Don't update all meshes always on single node changes, but
      check which ones should be updated
	  - implement Map::updateNodeMeshes() and the usage of it
	  - It will give almost always a 4x boost in mesh update performance.

- A weapon engine

- Tool/weapon visualization

FIXME: When disconnected to the menu, memory is not freed properly

Server:
-------

SUGG: Make an option to the server to disable building and digging near
      the starting position

FIXME: Server sometimes goes into some infinite PeerNotFoundException loop

* Fix the problem with the server constantly saving one or a few
  blocks? List the first saved block, maybe it explains.
  - It is probably caused by oscillating water
* Make a small history check to transformLiquids to detect and log
  continuous oscillations, in such detail that they can be fixed.

FIXME: The new optimized map sending doesn't sometimes send enough blocks
       from big caves and such

* Take player's walking direction into account in GetNextBlocks

Environment:
------------

TODO: A list of "active blocks" in which stuff happens.
	+ Add a never-resetted game timer to the server
	+ Add a timestamp value to blocks
	+ The simple rule: All blocks near some player are "active"
	- Do stuff in real time in active blocks
		+ Handle objects
		TODO: Make proper hooks in here
		- Grow grass, delete leaves without a tree
		- Spawn some mobs based on some rules
		- Transform cobble to mossy cobble near water
		- Run a custom script
		- ...And all kinds of other dynamic stuff
	+ Keep track of when a block becomes active and becomes inactive
	+ When a block goes inactive:
		+ Store objects statically to block
		+ Store timer value as the timestamp
	+ When a block goes active:
		+ Create active objects out of static objects
		TODO: Make proper hooks in here
		- Simulate the results of what would have happened if it would have
		  been active for all the time
		  	- Grow a lot of grass and so on
	+ Initially it is fine to send information about every active object
	  to every player. Eventually it should be modified to only send info
	  about the nearest ones.
	  	+ This was left to be done by the old system and it sends only the
		  nearest ones.

Objects:
--------

TODO: Get rid of MapBlockObjects and use only ActiveObjects
	- Skipping the MapBlockObject data is nasty - there is no "total
	  length" stored; have to make a SkipMBOs function which contains
	  enough of the current code to skip them properly.

SUGG: MovingObject::move and Player::move are basically the same.
      combine them.
	- NOTE: Player::move is more up-to-date.
	- NOTE: There is a simple move implementation now in collision.{h,cpp}
	- NOTE: MovingObject will be deleted (MapBlockObject)

TODO: Add a long step function to objects that is called with the time
      difference when block activates

Map:
----

TODO: Mineral and ground material properties
      - This way mineral ground toughness can be calculated with just
	    some formula, as well as tool strengths

TODO: Flowing water to actually contain flow direction information
      - There is a space for this - it just has to be implemented.

SUGG: Erosion simulation at map generation time
	- Simulate water flows, which would carve out dirt fast and
	  then turn stone into gravel and sand and relocate it.
	- How about relocating minerals, too? Coal and gold in
	  downstream sand and gravel would be kind of cool
	  - This would need a better way of handling minerals, mainly
		to have mineral content as a separate field. the first
		parameter field is free for this.
	- Simulate rock falling from cliffs when water has removed
	  enough solid rock from the bottom

SUGG: Try out the notch way of generating maps, that is, make bunches
      of low-res 3d noise and interpolate linearly.

Mapgen v2:
* Possibly add some kind of erosion and other stuff
* Better water generation (spread it to underwater caverns but don't
  fill dungeons that don't touch big water masses)
* When generating a chunk and the neighboring chunk doesn't have mud
  and stuff yet and the ground is fairly flat, the mud will flow to
  the other chunk making nasty straight walls when the other chunk
  is generated. Fix it. Maybe just a special case if the ground is
  flat?

Misc. stuff:
------------
* Move digging property stuff from material.{h,cpp} to mapnode.cpp
  - ...Or maybe move content_features to material.{h,cpp}?

Making it more portable:
------------------------
 
Stuff to do before release:
---------------------------

Fixes to the current release:
-----------------------------
- Fix client password crash
- Remember to release the fixes (some are already done)

Stuff to do after release:
---------------------------
- Make sure server handles removing grass when a block is placed (etc)
    - The client should not do it by itself
- Block cube placement around player's head
- Protocol version field
- Consider getting some textures from cisoun's texture pack
	- Ask from Cisoun
- Make sure the fence implementation and data format is good
	- Think about using same bits for material for fences and doors, for
	example
- Finish the ActiveBlockModifier stuff and use it for something
- Move mineral to param2, increment map serialization version, add conversion

======================================================================

*/

#ifdef NDEBUG
	#ifdef _WIN32
		#pragma message ("Disabling unit tests")
	#else
		#warning "Disabling unit tests"
	#endif
	// Disable unit tests
	#define ENABLE_TESTS 0
#else
	// Enable unit tests
	#define ENABLE_TESTS 1
#endif

#ifdef _MSC_VER
	#pragma comment(lib, "Irrlicht.lib")
	//#pragma comment(lib, "jthread.lib")
	#pragma comment(lib, "zlibwapi.lib")
	#pragma comment(lib, "Shell32.lib")
	// This would get rid of the console window
	//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#include <iostream>
#include <fstream>
//#include <jmutexautolock.h>
#include <locale.h>
#include "main.h"
#include "common_irrlicht.h"
#include "debug.h"
//#include "map.h"
//#include "player.h"
#include "test.h"
#include "server.h"
//#include "client.h"
#include "constants.h"
#include "porting.h"
#include "gettime.h"
#include "guiMessageMenu.h"
#include "filesys.h"
#include "config.h"
#include "guiMainMenu.h"
#include "mineral.h"
//#include "noise.h"
//#include "tile.h"
#include "materials.h"
#include "game.h"
#include "keycode.h"

// This makes textures
ITextureSource *g_texturesource = NULL;

/*
	Settings.
	These are loaded from the config file.
*/

Settings g_settings;
// This is located in defaultsettings.cpp
extern void set_default_settings();

// Global profiler
Profiler g_profiler;

/*
	Random stuff
*/

/*
	GUI Stuff
*/

gui::IGUIEnvironment* guienv = NULL;
gui::IGUIStaticText *guiroot = NULL;

MainMenuManager g_menumgr;

bool noMenuActive()
{
	return (g_menumgr.menuCount() == 0);
}

// Passed to menus to allow disconnecting and exiting

MainGameCallback *g_gamecallback = NULL;

/*
	Debug streams
*/

// Connection
std::ostream *dout_con_ptr = &dummyout;
std::ostream *derr_con_ptr = &dstream_no_stderr;
//std::ostream *dout_con_ptr = &dstream_no_stderr;
//std::ostream *derr_con_ptr = &dstream_no_stderr;
//std::ostream *dout_con_ptr = &dstream;
//std::ostream *derr_con_ptr = &dstream;

// Server
std::ostream *dout_server_ptr = &dstream;
std::ostream *derr_server_ptr = &dstream;

// Client
std::ostream *dout_client_ptr = &dstream;
std::ostream *derr_client_ptr = &dstream;

/*
	gettime.h implementation
*/

// A small helper class
class TimeGetter
{
public:
	virtual u32 getTime() = 0;
};

// A precise irrlicht one
class IrrlichtTimeGetter: public TimeGetter
{
public:
	IrrlichtTimeGetter(IrrlichtDevice *device):
		m_device(device)
	{}
	u32 getTime()
	{
		if(m_device == NULL)
			return 0;
		return m_device->getTimer()->getRealTime();
	}
private:
	IrrlichtDevice *m_device;
};
// Not so precise one which works without irrlicht
class SimpleTimeGetter: public TimeGetter
{
public:
	u32 getTime()
	{
		return porting::getTimeMs();
	}
};

// A pointer to a global instance of the time getter
// TODO: why?
TimeGetter *g_timegetter = NULL;

u32 getTimeMs()
{
	if(g_timegetter == NULL)
		return 0;
	return g_timegetter->getTime();
}

/*
	Event handler for Irrlicht

	NOTE: Everything possible should be moved out from here,
	      probably to InputHandler and the_game
*/

class MyEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		/*
			React to nothing here if a menu is active
		*/
		if(noMenuActive() == false)
		{
			return false;
		}

		// Remember whether each key is down or up
		if(event.EventType == irr::EET_KEY_INPUT_EVENT)
		{
			keyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

			if(event.KeyInput.PressedDown)
				keyWasDown[event.KeyInput.Key] = true;
		}

		if(event.EventType == irr::EET_MOUSE_INPUT_EVENT)
		{
			if(noMenuActive() == false)
			{
				left_active = false;
				middle_active = false;
				right_active = false;
			}
			else
			{
				//dstream<<"MyEventReceiver: mouse input"<<std::endl;
				left_active = event.MouseInput.isLeftPressed();
				middle_active = event.MouseInput.isMiddlePressed();
				right_active = event.MouseInput.isRightPressed();

				if(event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
				{
					leftclicked = true;
				}
				if(event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
				{
					rightclicked = true;
				}
				if(event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
				{
					leftreleased = true;
				}
				if(event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
				{
					rightreleased = true;
				}
				if(event.MouseInput.Event == EMIE_MOUSE_WHEEL)
				{
					mouse_wheel += event.MouseInput.Wheel;
				}
			}
		}

		return false;
	}

	bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return keyIsDown[keyCode];
	}
	
	// Checks whether a key was down and resets the state
	bool WasKeyDown(EKEY_CODE keyCode)
	{
		bool b = keyWasDown[keyCode];
		keyWasDown[keyCode] = false;
		return b;
	}

	s32 getMouseWheel()
	{
		s32 a = mouse_wheel;
		mouse_wheel = 0;
		return a;
	}

	void clearInput()
	{
		for(u32 i=0; i<KEY_KEY_CODES_COUNT; i++)
		{
			keyIsDown[i] = false;
			keyWasDown[i] = false;
		}
		
		leftclicked = false;
		rightclicked = false;
		leftreleased = false;
		rightreleased = false;

		left_active = false;
		middle_active = false;
		right_active = false;

		mouse_wheel = 0;
	}

	MyEventReceiver()
	{
		clearInput();
	}

	bool leftclicked;
	bool rightclicked;
	bool leftreleased;
	bool rightreleased;

	bool left_active;
	bool middle_active;
	bool right_active;

	s32 mouse_wheel;

private:
	IrrlichtDevice *m_device;
	
	// The current state of keys
	bool keyIsDown[KEY_KEY_CODES_COUNT];
	// Whether a key has been pressed or not
	bool keyWasDown[KEY_KEY_CODES_COUNT];
};

/*
	Separated input handler
*/

class RealInputHandler : public InputHandler
{
public:
	RealInputHandler(IrrlichtDevice *device, MyEventReceiver *receiver):
		m_device(device),
		m_receiver(receiver)
	{
	}
	virtual bool isKeyDown(EKEY_CODE keyCode)
	{
		return m_receiver->IsKeyDown(keyCode);
	}
	virtual bool wasKeyDown(EKEY_CODE keyCode)
	{
		return m_receiver->WasKeyDown(keyCode);
	}
	virtual v2s32 getMousePos()
	{
		return m_device->getCursorControl()->getPosition();
	}
	virtual void setMousePos(s32 x, s32 y)
	{
		m_device->getCursorControl()->setPosition(x, y);
	}

	virtual bool getLeftState()
	{
		return m_receiver->left_active;
	}
	virtual bool getRightState()
	{
		return m_receiver->right_active;
	}
	
	virtual bool getLeftClicked()
	{
		return m_receiver->leftclicked;
	}
	virtual bool getRightClicked()
	{
		return m_receiver->rightclicked;
	}
	virtual void resetLeftClicked()
	{
		m_receiver->leftclicked = false;
	}
	virtual void resetRightClicked()
	{
		m_receiver->rightclicked = false;
	}

	virtual bool getLeftReleased()
	{
		return m_receiver->leftreleased;
	}
	virtual bool getRightReleased()
	{
		return m_receiver->rightreleased;
	}
	virtual void resetLeftReleased()
	{
		m_receiver->leftreleased = false;
	}
	virtual void resetRightReleased()
	{
		m_receiver->rightreleased = false;
	}

	virtual s32 getMouseWheel()
	{
		return m_receiver->getMouseWheel();
	}

	void clear()
	{
		m_receiver->clearInput();
	}
private:
	IrrlichtDevice *m_device;
	MyEventReceiver *m_receiver;
};

class RandomInputHandler : public InputHandler
{
public:
	RandomInputHandler()
	{
		leftdown = false;
		rightdown = false;
		leftclicked = false;
		rightclicked = false;
		leftreleased = false;
		rightreleased = false;
		for(u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
			keydown[i] = false;
	}
	virtual bool isKeyDown(EKEY_CODE keyCode)
	{
		return keydown[keyCode];
	}
	virtual bool wasKeyDown(EKEY_CODE keyCode)
	{
		return false;
	}
	virtual v2s32 getMousePos()
	{
		return mousepos;
	}
	virtual void setMousePos(s32 x, s32 y)
	{
		mousepos = v2s32(x,y);
	}

	virtual bool getLeftState()
	{
		return leftdown;
	}
	virtual bool getRightState()
	{
		return rightdown;
	}

	virtual bool getLeftClicked()
	{
		return leftclicked;
	}
	virtual bool getRightClicked()
	{
		return rightclicked;
	}
	virtual void resetLeftClicked()
	{
		leftclicked = false;
	}
	virtual void resetRightClicked()
	{
		rightclicked = false;
	}

	virtual bool getLeftReleased()
	{
		return leftreleased;
	}
	virtual bool getRightReleased()
	{
		return rightreleased;
	}
	virtual void resetLeftReleased()
	{
		leftreleased = false;
	}
	virtual void resetRightReleased()
	{
		rightreleased = false;
	}

	virtual s32 getMouseWheel()
	{
		return 0;
	}

	virtual void step(float dtime)
	{
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown[getKeySetting("keymap_jump")] =
						!keydown[getKeySetting("keymap_jump")];
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown[getKeySetting("keymap_special1")] =
						!keydown[getKeySetting("keymap_special1")];
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown[getKeySetting("keymap_forward")] =
						!keydown[getKeySetting("keymap_forward")];
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 40);
				keydown[getKeySetting("keymap_left")] =
						!keydown[getKeySetting("keymap_left")];
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 20);
				mousespeed = v2s32(Rand(-20,20), Rand(-15,20));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 30);
				leftdown = !leftdown;
				if(leftdown)
					leftclicked = true;
				if(!leftdown)
					leftreleased = true;
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if(counter1 < 0.0)
			{
				counter1 = 0.1*Rand(1, 15);
				rightdown = !rightdown;
				if(rightdown)
					rightclicked = true;
				if(!rightdown)
					rightreleased = true;
			}
		}
		mousepos += mousespeed;
	}

	s32 Rand(s32 min, s32 max)
	{
		return (myrand()%(max-min+1))+min;
	}
private:
	bool keydown[KEY_KEY_CODES_COUNT];
	v2s32 mousepos;
	v2s32 mousespeed;
	bool leftdown;
	bool rightdown;
	bool leftclicked;
	bool rightclicked;
	bool leftreleased;
	bool rightreleased;
};

// These are defined global so that they're not optimized too much.
// Can't change them to volatile.
s16 temp16;
f32 tempf;
v3f tempv3f1;
v3f tempv3f2;
std::string tempstring;
std::string tempstring2;

void SpeedTests()
{
	{
		dstream<<"The following test should take around 20ms."<<std::endl;
		TimeTaker timer("Testing std::string speed");
		const u32 jj = 10000;
		for(u32 j=0; j<jj; j++)
		{
			tempstring = "";
			tempstring2 = "";
			const u32 ii = 10;
			for(u32 i=0; i<ii; i++){
				tempstring2 += "asd";
			}
			for(u32 i=0; i<ii+1; i++){
				tempstring += "asd";
				if(tempstring == tempstring2)
					break;
			}
		}
	}
	
	dstream<<"All of the following tests should take around 100ms each."
			<<std::endl;

	{
		TimeTaker timer("Testing floating-point conversion speed");
		tempf = 0.001;
		for(u32 i=0; i<4000000; i++){
			temp16 += tempf;
			tempf += 0.001;
		}
	}
	
	{
		TimeTaker timer("Testing floating-point vector speed");

		tempv3f1 = v3f(1,2,3);
		tempv3f2 = v3f(4,5,6);
		for(u32 i=0; i<10000000; i++){
			tempf += tempv3f1.dotProduct(tempv3f2);
			tempv3f2 += v3f(7,8,9);
		}
	}

	{
		TimeTaker timer("Testing core::map speed");
		
		core::map<v2s16, f32> map1;
		tempf = -324;
		const s16 ii=300;
		for(s16 y=0; y<ii; y++){
			for(s16 x=0; x<ii; x++){
				map1.insert(v2s16(x,y), tempf);
				tempf += 1;
			}
		}
		for(s16 y=ii-1; y>=0; y--){
			for(s16 x=0; x<ii; x++){
				tempf = map1[v2s16(x,y)];
			}
		}
	}

	{
		dstream<<"Around 5000/ms should do well here."<<std::endl;
		TimeTaker timer("Testing mutex speed");
		
		JMutex m;
		m.Init();
		u32 n = 0;
		u32 i = 0;
		do{
			n += 10000;
			for(; i<n; i++){
				m.Lock();
				m.Unlock();
			}
		}
		// Do at least 10ms
		while(timer.getTime() < 10);

		u32 dtime = timer.stop();
		u32 per_ms = n / dtime;
		std::cout<<"Done. "<<dtime<<"ms, "
				<<per_ms<<"/ms"<<std::endl;
	}
}

void drawMenuBackground(video::IVideoDriver* driver)
{
	core::dimension2d<u32> screensize = driver->getScreenSize();
		
	video::ITexture *bgtexture =
			driver->getTexture(getTexturePath("mud.png").c_str());
	if(bgtexture)
	{
		s32 texturesize = 128;
		s32 tiled_y = screensize.Height / texturesize + 1;
		s32 tiled_x = screensize.Width / texturesize + 1;
		
		for(s32 y=0; y<tiled_y; y++)
		for(s32 x=0; x<tiled_x; x++)
		{
			core::rect<s32> rect(0,0,texturesize,texturesize);
			rect += v2s32(x*texturesize, y*texturesize);
			driver->draw2DImage(bgtexture, rect,
				core::rect<s32>(core::position2d<s32>(0,0),
				core::dimension2di(bgtexture->getSize())),
				NULL, NULL, true);
		}
	}
	
	video::ITexture *logotexture =
			driver->getTexture(getTexturePath("menulogo.png").c_str());
	if(logotexture)
	{
		v2s32 logosize(logotexture->getOriginalSize().Width,
				logotexture->getOriginalSize().Height);
		logosize *= 4;

		video::SColor bgcolor(255,50,50,50);
		core::rect<s32> bgrect(0, screensize.Height-logosize.Y-20,
				screensize.Width, screensize.Height);
		driver->draw2DRectangle(bgcolor, bgrect, NULL);

		core::rect<s32> rect(0,0,logosize.X,logosize.Y);
		rect += v2s32(screensize.Width/2,screensize.Height-10-logosize.Y);
		rect -= v2s32(logosize.X/2, 0);
		driver->draw2DImage(logotexture, rect,
			core::rect<s32>(core::position2d<s32>(0,0),
			core::dimension2di(logotexture->getSize())),
			NULL, NULL, true);
	}
}

int main(int argc, char *argv[])
{
	/*
		Parse command line
	*/
	
	// List all allowed options
	core::map<std::string, ValueSpec> allowed_options;
	allowed_options.insert("help", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("server", ValueSpec(VALUETYPE_FLAG,
			"Run server directly"));
	allowed_options.insert("config", ValueSpec(VALUETYPE_STRING,
			"Load configuration from specified file"));
	allowed_options.insert("port", ValueSpec(VALUETYPE_STRING));
	allowed_options.insert("address", ValueSpec(VALUETYPE_STRING));
	allowed_options.insert("random-input", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("disable-unittests", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("enable-unittests", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("map-dir", ValueSpec(VALUETYPE_STRING));
#ifdef _WIN32
	allowed_options.insert("dstream-on-stderr", ValueSpec(VALUETYPE_FLAG));
#endif
	allowed_options.insert("speedtests", ValueSpec(VALUETYPE_FLAG));

	Settings cmd_args;
	
	bool ret = cmd_args.parseCommandLine(argc, argv, allowed_options);

	if(ret == false || cmd_args.getFlag("help"))
	{
		dstream<<"Allowed options:"<<std::endl;
		for(core::map<std::string, ValueSpec>::Iterator
				i = allowed_options.getIterator();
				i.atEnd() == false; i++)
		{
			dstream<<"  --"<<i.getNode()->getKey();
			if(i.getNode()->getValue().type == VALUETYPE_FLAG)
			{
			}
			else
			{
				dstream<<" <value>";
			}
			dstream<<std::endl;

			if(i.getNode()->getValue().help != NULL)
			{
				dstream<<"      "<<i.getNode()->getValue().help
						<<std::endl;
			}
		}

		return cmd_args.getFlag("help") ? 0 : 1;
	}
	
	/*
		Low-level initialization
	*/

	bool disable_stderr = false;
#ifdef _WIN32
	if(cmd_args.getFlag("dstream-on-stderr") == false)
		disable_stderr = true;
#endif

	// Initialize debug streams
	debugstreams_init(disable_stderr, DEBUGFILE);
	// Initialize debug stacks
	debug_stacks_init();

	DSTACK(__FUNCTION_NAME);

	porting::signal_handler_init();
	bool &kill = *porting::signal_handler_killstatus();
	
	porting::initializePaths();
	// Create user data directory
	fs::CreateDir(porting::path_userdata);
	
	// C-style stuff initialization
	initializeMaterialProperties();

	// Debug handler
	BEGIN_DEBUG_EXCEPTION_HANDLER

	// Print startup message
	dstream<<DTIME<<"minetest-c55"
			" with SER_FMT_VER_HIGHEST="<<(int)SER_FMT_VER_HIGHEST
			<<", "<<BUILD_INFO
			<<std::endl;
	
	/*
		Basic initialization
	*/

	// Initialize default settings
	set_default_settings();
	
	// Set locale. This is for forcing '.' as the decimal point.
	std::locale::global(std::locale("C"));
	// This enables printing all characters in bitmap font
	setlocale(LC_CTYPE, "en_US");

	// Initialize sockets
	sockets_init();
	atexit(sockets_cleanup);
	
	/*
		Initialization
	*/

	/*
		Read config file
	*/
	
	// Path of configuration file in use
	std::string configpath = "";
	
	if(cmd_args.exists("config"))
	{
		bool r = g_settings.readConfigFile(cmd_args.get("config").c_str());
		if(r == false)
		{
			dstream<<"Could not read configuration from \""
					<<cmd_args.get("config")<<"\""<<std::endl;
			return 1;
		}
		configpath = cmd_args.get("config");
	}
	else
	{
		core::array<std::string> filenames;
		filenames.push_back(porting::path_userdata + "/minetest.conf");
#ifdef RUN_IN_PLACE
		filenames.push_back(porting::path_userdata + "/../minetest.conf");
#endif

		for(u32 i=0; i<filenames.size(); i++)
		{
			bool r = g_settings.readConfigFile(filenames[i].c_str());
			if(r)
			{
				configpath = filenames[i];
				break;
			}
		}
		
		// If no path found, use the first one (menu creates the file)
		if(configpath == "")
			configpath = filenames[0];
	}

	// Initialize random seed
	srand(time(0));
	mysrand(time(0));

	/*
		Pre-initialize some stuff with a dummy irrlicht wrapper.

		These are needed for unit tests at least.
	*/
	
	// Initial call with g_texturesource not set.
	init_mapnode();

	/*
		Run unit tests
	*/

	if((ENABLE_TESTS && cmd_args.getFlag("disable-unittests") == false)
			|| cmd_args.getFlag("enable-unittests") == true)
	{
		run_tests();
	}
	
	/*for(s16 y=-100; y<100; y++)
	for(s16 x=-100; x<100; x++)
	{
		std::cout<<noise2d_gradient((double)x/10,(double)y/10, 32415)<<std::endl;
	}
	return 0;*/
	
	/*
		Game parameters
	*/

	// Port
	u16 port = 30000;
	if(cmd_args.exists("port"))
		port = cmd_args.getU16("port");
	else if(g_settings.exists("port"))
		port = g_settings.getU16("port");
	if(port == 0)
		port = 30000;
	
	// Map directory
	std::string map_dir = porting::path_userdata+"/world";
	if(cmd_args.exists("map-dir"))
		map_dir = cmd_args.get("map-dir");
	else if(g_settings.exists("map-dir"))
		map_dir = g_settings.get("map-dir");
	
	// Run dedicated server if asked to
	if(cmd_args.getFlag("server"))
	{
		DSTACK("Dedicated server branch");

		// Create time getter
		g_timegetter = new SimpleTimeGetter();
		
		// Create server
		Server server(map_dir.c_str());
		server.start(port);
		
		// Run server
		dedicated_server_loop(server, kill);

		return 0;
	}


	/*
		More parameters
	*/
	
	// Address to connect to
	std::string address = "";
	
	if(cmd_args.exists("address"))
	{
		address = cmd_args.get("address");
	}
	else
	{
		address = g_settings.get("address");
	}
	
	std::string playername = g_settings.get("name");

	/*
		Device initialization
	*/

	// Resolution selection
	
	bool fullscreen = false;
	u16 screenW = g_settings.getU16("screenW");
	u16 screenH = g_settings.getU16("screenH");

	// Determine driver

	video::E_DRIVER_TYPE driverType;
	
	std::string driverstring = g_settings.get("video_driver");

	if(driverstring == "null")
		driverType = video::EDT_NULL;
	else if(driverstring == "software")
		driverType = video::EDT_SOFTWARE;
	else if(driverstring == "burningsvideo")
		driverType = video::EDT_BURNINGSVIDEO;
	else if(driverstring == "direct3d8")
		driverType = video::EDT_DIRECT3D8;
	else if(driverstring == "direct3d9")
		driverType = video::EDT_DIRECT3D9;
	else if(driverstring == "opengl")
		driverType = video::EDT_OPENGL;
	else
	{
		dstream<<"WARNING: Invalid video_driver specified; defaulting "
				"to opengl"<<std::endl;
		driverType = video::EDT_OPENGL;
	}

	/*
		Create device and exit if creation failed
	*/

	MyEventReceiver receiver;

	IrrlichtDevice *device;
	device = createDevice(driverType,
			core::dimension2d<u32>(screenW, screenH),
			16, fullscreen, false, false, &receiver);

	if (device == 0)
		return 1; // could not create selected driver.
	
	// Set device in game parameters
	device = device;
	
	// Create time getter
	g_timegetter = new IrrlichtTimeGetter(device);
	
	// Create game callback for menus
	g_gamecallback = new MainGameCallback(device);
	
	// Create texture source
	g_texturesource = new TextureSource(device);

	/*
		Speed tests (done after irrlicht is loaded to get timer)
	*/
	if(cmd_args.getFlag("speedtests"))
	{
		dstream<<"Running speed tests"<<std::endl;
		SpeedTests();
		return 0;
	}
	
	device->setResizable(true);

	bool random_input = g_settings.getBool("random_input")
			|| cmd_args.getFlag("random-input");
	InputHandler *input = NULL;
	if(random_input)
		input = new RandomInputHandler();
	else
		input = new RealInputHandler(device, &receiver);
	
	/*
		Continue initialization
	*/

	//video::IVideoDriver* driver = device->getVideoDriver();

	/*
		This changes the minimum allowed number of vertices in a VBO.
		Default is 500.
	*/
	//driver->setMinHardwareBufferVertexCount(50);

	scene::ISceneManager* smgr = device->getSceneManager();

	guienv = device->getGUIEnvironment();
	gui::IGUISkin* skin = guienv->getSkin();
	gui::IGUIFont* font = guienv->getFont(getTexturePath("fontlucida.png").c_str());
	if(font)
		skin->setFont(font);
	else
		dstream<<"WARNING: Font file was not found."
				" Using default font."<<std::endl;
	// If font was not found, this will get us one
	font = skin->getFont();
	assert(font);
	
	u32 text_height = font->getDimension(L"Hello, world!").Height;
	dstream<<"text_height="<<text_height<<std::endl;

	//skin->setColor(gui::EGDC_BUTTON_TEXT, video::SColor(255,0,0,0));
	skin->setColor(gui::EGDC_BUTTON_TEXT, video::SColor(255,255,255,255));
	//skin->setColor(gui::EGDC_3D_HIGH_LIGHT, video::SColor(0,0,0,0));
	//skin->setColor(gui::EGDC_3D_SHADOW, video::SColor(0,0,0,0));
	skin->setColor(gui::EGDC_3D_HIGH_LIGHT, video::SColor(255,0,0,0));
	skin->setColor(gui::EGDC_3D_SHADOW, video::SColor(255,0,0,0));
	
	/*
		Preload some textures and stuff
	*/

	init_content_inventory_texture_paths();
	init_mapnode(); // Second call with g_texturesource set
	init_mineral();

	/*
		GUI stuff
	*/

	/*
		If an error occurs, this is set to something and the
		menu-game loop is restarted. It is then displayed before
		the menu.
	*/
	std::wstring error_message = L"";

	// The password entered during the menu screen,
	std::string password;

	/*
		Menu-game loop
	*/
	while(device->run() && kill == false)
	{

		// This is used for catching disconnects
		try
		{

			/*
				Clear everything from the GUIEnvironment
			*/
			guienv->clear();
			
			/*
				We need some kind of a root node to be able to add
				custom gui elements directly on the screen.
				Otherwise they won't be automatically drawn.
			*/
			guiroot = guienv->addStaticText(L"",
					core::rect<s32>(0, 0, 10000, 10000));
			
			/*
				Out-of-game menu loop.

				Loop quits when menu returns proper parameters.
			*/
			while(kill == false)
			{
				// Cursor can be non-visible when coming from the game
				device->getCursorControl()->setVisible(true);
				// Some stuff are left to scene manager when coming from the game
				// (map at least?)
				smgr->clear();
				// Reset or hide the debug gui texts
				/*guitext->setText(L"Minetest-c55");
				guitext2->setVisible(false);
				guitext_info->setVisible(false);
				guitext_chat->setVisible(false);*/
				
				// Initialize menu data
				MainMenuData menudata;
				menudata.address = narrow_to_wide(address);
				menudata.name = narrow_to_wide(playername);
				menudata.port = narrow_to_wide(itos(port));
				menudata.fancy_trees = g_settings.getBool("new_style_leaves");
				menudata.smooth_lighting = g_settings.getBool("smooth_lighting");
				menudata.creative_mode = g_settings.getBool("creative_mode");
				menudata.enable_damage = g_settings.getBool("enable_damage");

				GUIMainMenu *menu =
						new GUIMainMenu(guienv, guiroot, -1, 
							&g_menumgr, &menudata, g_gamecallback);
				menu->allowFocusRemoval(true);

				if(error_message != L"")
				{
					dstream<<"WARNING: error_message = "
							<<wide_to_narrow(error_message)<<std::endl;

					GUIMessageMenu *menu2 =
							new GUIMessageMenu(guienv, guiroot, -1, 
								&g_menumgr, error_message.c_str());
					menu2->drop();
					error_message = L"";
				}

				video::IVideoDriver* driver = device->getVideoDriver();
				
				dstream<<"Created main menu"<<std::endl;

				while(device->run() && kill == false)
				{
					if(menu->getStatus() == true)
						break;

					//driver->beginScene(true, true, video::SColor(255,0,0,0));
					driver->beginScene(true, true, video::SColor(255,128,128,128));

					drawMenuBackground(driver);

					guienv->drawAll();
					
					driver->endScene();
					
					// On some computers framerate doesn't seem to be
					// automatically limited
					sleep_ms(25);
				}
				
				// Break out of menu-game loop to shut down cleanly
				if(device->run() == false || kill == true)
					break;
				
				dstream<<"Dropping main menu"<<std::endl;

				menu->drop();
				
				// Delete map if requested
				if(menudata.delete_map)
				{
					bool r = fs::RecursiveDeleteContent(map_dir);
					if(r == false)
						error_message = L"Delete failed";
					continue;
				}

				playername = wide_to_narrow(menudata.name);

				password = translatePassword(playername, menudata.password);

				address = wide_to_narrow(menudata.address);
				int newport = stoi(wide_to_narrow(menudata.port));
				if(newport != 0)
					port = newport;
				g_settings.set("new_style_leaves", itos(menudata.fancy_trees));
				g_settings.set("smooth_lighting", itos(menudata.smooth_lighting));
				g_settings.set("creative_mode", itos(menudata.creative_mode));
				g_settings.set("enable_damage", itos(menudata.enable_damage));
				
				/*// Check for valid parameters, restart menu if invalid.
				if(playername == "")
				{
					error_message = L"Name required.";
					continue;
				}
				// Check that name has only valid chars
				if(string_allowed(playername, PLAYERNAME_ALLOWED_CHARS)==false)
				{
					error_message = L"Characters allowed: "
							+narrow_to_wide(PLAYERNAME_ALLOWED_CHARS);
					continue;
				}*/

				// Save settings
				g_settings.set("name", playername);
				g_settings.set("address", address);
				g_settings.set("port", itos(port));
				// Update configuration file
				if(configpath != "")
					g_settings.updateConfigFile(configpath.c_str());
			
				// Continue to game
				break;
			}
			
			// Break out of menu-game loop to shut down cleanly
			if(device->run() == false)
				break;
			
			// Initialize mapnode again to enable changed graphics settings
			init_mapnode();

			/*
				Run game
			*/
			the_game(
				kill,
				random_input,
				input,
				device,
				font,
				map_dir,
				playername,
				password,
				address,
				port,
				error_message
			);

		} //try
		catch(con::PeerNotFoundException &e)
		{
			dstream<<DTIME<<"Connection error (timed out?)"<<std::endl;
			error_message = L"Connection error (timed out?)";
		}
		catch(SocketException &e)
		{
			dstream<<DTIME<<"Socket error (port already in use?)"<<std::endl;
			error_message = L"Socket error (port already in use?)";
		}
#ifdef NDEBUG
		catch(std::exception &e)
		{
			std::string narrow_message = "Some exception, what()=\"";
			narrow_message += e.what();
			narrow_message += "\"";
			dstream<<DTIME<<narrow_message<<std::endl;
			error_message = narrow_to_wide(narrow_message);
		}
#endif

	} // Menu-game loop
	
	delete input;

	/*
		In the end, delete the Irrlicht device.
	*/
	device->drop();
	
	END_DEBUG_EXCEPTION_HANDLER
	
	debugstreams_deinit();
	
	return 0;
}

//END
