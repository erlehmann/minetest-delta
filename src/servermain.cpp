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

/*
=============================== NOTES ==============================


*/

#ifndef SERVER
	#ifdef _WIN32
		#pragma error ("For a server build, SERVER must be defined globally")
	#else
		#error "For a server build, SERVER must be defined globally"
	#endif
#endif

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
#pragma comment(lib, "jthread.lib")
#pragma comment(lib, "zlibwapi.lib")
#endif

#include <iostream>
#include <fstream>
#include <time.h>
#include <jmutexautolock.h>
#include <locale.h>
#include "common_irrlicht.h"
#include "debug.h"
#include "map.h"
#include "player.h"
#include "main.h"
#include "test.h"
#include "environment.h"
#include "server.h"
#include "serialization.h"
#include "constants.h"
#include "strfnd.h"
#include "porting.h"
#include "materials.h"
#include "config.h"
#include "mineral.h"

/*
	Settings.
	These are loaded from the config file.
*/

Settings g_settings;

extern void set_default_settings();

// A dummy thing
ITextureSource *g_texturesource = NULL;

/*
	Debug streams
*/

// Connection
std::ostream *dout_con_ptr = &dummyout;
std::ostream *derr_con_ptr = &dstream_no_stderr;

// Server
std::ostream *dout_server_ptr = &dstream;
std::ostream *derr_server_ptr = &dstream;

// Client
std::ostream *dout_client_ptr = &dstream;
std::ostream *derr_client_ptr = &dstream;

/*
	gettime.h implementation
*/

u32 getTimeMs()
{
	/*
		Use imprecise system calls directly (from porting.h)
	*/
	return porting::getTimeMs();
}

int main(int argc, char *argv[])
{
	/*
		Low-level initialization
	*/

	bool disable_stderr = false;
#ifdef _WIN32
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

	initializeMaterialProperties();

	BEGIN_DEBUG_EXCEPTION_HANDLER

	// Print startup message
	dstream<<DTIME<<"minetest-c55"
			" with SER_FMT_VER_HIGHEST="<<(int)SER_FMT_VER_HIGHEST
			<<", "<<BUILD_INFO
			<<std::endl;
	
	try
	{
	
	/*
		Parse command line
	*/
	
	// List all allowed options
	core::map<std::string, ValueSpec> allowed_options;
	allowed_options.insert("help", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("config", ValueSpec(VALUETYPE_STRING,
			"Load configuration from specified file"));
	allowed_options.insert("port", ValueSpec(VALUETYPE_STRING));
	allowed_options.insert("disable-unittests", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("enable-unittests", ValueSpec(VALUETYPE_FLAG));
	allowed_options.insert("map-dir", ValueSpec(VALUETYPE_STRING));

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
	}

	// Initialize random seed
	srand(time(0));
	mysrand(time(0));

	// Initialize stuff
	
	init_mapnode();
	init_mineral();

	/*
		Run unit tests
	*/
	if((ENABLE_TESTS && cmd_args.getFlag("disable-unittests") == false)
			|| cmd_args.getFlag("enable-unittests") == true)
	{
		run_tests();
	}

	/*
		Check parameters
	*/

	std::cout<<std::endl<<std::endl;
	
	std::cout
	<<"        .__               __                   __   "<<std::endl
	<<"  _____ |__| ____   _____/  |_  ____   _______/  |_ "<<std::endl
	<<" /     \\|  |/    \\_/ __ \\   __\\/ __ \\ /  ___/\\   __\\"<<std::endl
	<<"|  Y Y  \\  |   |  \\  ___/|  | \\  ___/ \\___ \\  |  |  "<<std::endl
	<<"|__|_|  /__|___|  /\\___  >__|  \\___  >____  > |__|  "<<std::endl
	<<"      \\/        \\/     \\/          \\/     \\/        "<<std::endl
	<<std::endl;

	std::cout<<std::endl;
	
	// Port?
	u16 port = 30000;
	if(cmd_args.exists("port") && cmd_args.getU16("port") != 0)
	{
		port = cmd_args.getU16("port");
	}
	else if(g_settings.exists("port") && g_settings.getU16("port") != 0)
	{
		port = g_settings.getU16("port");
	}
	else
	{
		dstream<<"Please specify port (in config or on command line)"
				<<std::endl;
	}
	
	// Figure out path to map
	std::string map_dir = porting::path_userdata+"/world";
	if(cmd_args.exists("map-dir"))
		map_dir = cmd_args.get("map-dir");
	else if(g_settings.exists("map-dir"))
		map_dir = g_settings.get("map-dir");
	
	// Create server
	Server server(map_dir.c_str());
	server.start(port);

	// Run server
	dedicated_server_loop(server, kill);
	
	} //try
	catch(con::PeerNotFoundException &e)
	{
		dstream<<DTIME<<"Connection timed out."<<std::endl;
	}

	END_DEBUG_EXCEPTION_HANDLER

	debugstreams_deinit();
	
	return 0;
}

//END
