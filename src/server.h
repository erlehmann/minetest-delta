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
(c) 2010 Perttu Ahola <celeron55@gmail.com>
*/

#ifndef SERVER_HEADER
#define SERVER_HEADER

#include "connection.h"
#include "environment.h"
#include "common_irrlicht.h"
#include <string>
#include "utility.h"
#include "porting.h"
#include "map.h"
#include "inventory.h"
#include "auth.h"

/*
	Some random functions
*/
v3f findSpawnPos(ServerMap &map);

/*
	A structure containing the data needed for queueing the fetching
	of blocks.
*/
struct QueuedBlockEmerge
{
	v3s16 pos;
	// key = peer_id, value = flags
	core::map<u16, u8> peer_ids;
};

/*
	This is a thread-safe class.
*/
class BlockEmergeQueue
{
public:
	BlockEmergeQueue()
	{
		m_mutex.Init();
	}

	~BlockEmergeQueue()
	{
		JMutexAutoLock lock(m_mutex);

		core::list<QueuedBlockEmerge*>::Iterator i;
		for(i=m_queue.begin(); i!=m_queue.end(); i++)
		{
			QueuedBlockEmerge *q = *i;
			delete q;
		}
	}
	
	/*
		peer_id=0 adds with nobody to send to
	*/
	void addBlock(u16 peer_id, v3s16 pos, u8 flags)
	{
		DSTACK(__FUNCTION_NAME);
	
		JMutexAutoLock lock(m_mutex);

		if(peer_id != 0)
		{
			/*
				Find if block is already in queue.
				If it is, update the peer to it and quit.
			*/
			core::list<QueuedBlockEmerge*>::Iterator i;
			for(i=m_queue.begin(); i!=m_queue.end(); i++)
			{
				QueuedBlockEmerge *q = *i;
				if(q->pos == pos)
				{
					q->peer_ids[peer_id] = flags;
					return;
				}
			}
		}
		
		/*
			Add the block
		*/
		QueuedBlockEmerge *q = new QueuedBlockEmerge;
		q->pos = pos;
		if(peer_id != 0)
			q->peer_ids[peer_id] = flags;
		m_queue.push_back(q);
	}

	// Returned pointer must be deleted
	// Returns NULL if queue is empty
	QueuedBlockEmerge * pop()
	{
		JMutexAutoLock lock(m_mutex);

		core::list<QueuedBlockEmerge*>::Iterator i = m_queue.begin();
		if(i == m_queue.end())
			return NULL;
		QueuedBlockEmerge *q = *i;
		m_queue.erase(i);
		return q;
	}

	u32 size()
	{
		JMutexAutoLock lock(m_mutex);
		return m_queue.size();
	}
	
	u32 peerItemCount(u16 peer_id)
	{
		JMutexAutoLock lock(m_mutex);

		u32 count = 0;

		core::list<QueuedBlockEmerge*>::Iterator i;
		for(i=m_queue.begin(); i!=m_queue.end(); i++)
		{
			QueuedBlockEmerge *q = *i;
			if(q->peer_ids.find(peer_id) != NULL)
				count++;
		}

		return count;
	}

private:
	core::list<QueuedBlockEmerge*> m_queue;
	JMutex m_mutex;
};

class Server;

class ServerThread : public SimpleThread
{
	Server *m_server;

public:

	ServerThread(Server *server):
		SimpleThread(),
		m_server(server)
	{
	}

	void * Thread();
};

class EmergeThread : public SimpleThread
{
	Server *m_server;

public:

	EmergeThread(Server *server):
		SimpleThread(),
		m_server(server)
	{
	}

	void * Thread();

	void trigger()
	{
		setRun(true);
		if(IsRunning() == false)
		{
			Start();
		}
	}
};

struct PlayerInfo
{
	u16 id;
	char name[PLAYERNAME_SIZE];
	v3f position;
	Address address;
	float avg_rtt;

	PlayerInfo();
	void PrintLine(std::ostream *s);
};

u32 PIChecksum(core::list<PlayerInfo> &l);

/*
	Used for queueing and sorting block transfers in containers
	
	Lower priority number means higher priority.
*/
struct PrioritySortedBlockTransfer
{
	PrioritySortedBlockTransfer(float a_priority, v3s16 a_pos, u16 a_peer_id)
	{
		priority = a_priority;
		pos = a_pos;
		peer_id = a_peer_id;
	}
	bool operator < (PrioritySortedBlockTransfer &other)
	{
		return priority < other.priority;
	}
	float priority;
	v3s16 pos;
	u16 peer_id;
};

class RemoteClient
{
public:
	// peer_id=0 means this client has no associated peer
	// NOTE: If client is made allowed to exist while peer doesn't,
	//       this has to be set to 0 when there is no peer.
	//       Also, the client must be moved to some other container.
	u16 peer_id;
	// The serialization version to use with the client
	u8 serialization_version;
	// Version is stored in here after INIT before INIT2
	u8 pending_serialization_version;

	RemoteClient():
		m_time_from_building(9999),
		m_excess_gotblocks(0)
	{
		peer_id = 0;
		serialization_version = SER_FMT_VER_INVALID;
		pending_serialization_version = SER_FMT_VER_INVALID;
		m_nearest_unsent_d = 0;
		m_nearest_unsent_reset_timer = 0.0;
		m_nothing_to_send_counter = 0;
		m_nothing_to_send_pause_timer = 0;
	}
	~RemoteClient()
	{
	}
	
	/*
		Finds block that should be sent next to the client.
		Environment should be locked when this is called.
		dtime is used for resetting send radius at slow interval
	*/
	void GetNextBlocks(Server *server, float dtime,
			core::array<PrioritySortedBlockTransfer> &dest);

	/*
		Connection and environment should be locked when this is called.
		steps() objects of blocks not found in active_blocks, then
		adds those blocks to active_blocks
	*/
	void SendObjectData(
			Server *server,
			float dtime,
			core::map<v3s16, bool> &stepped_blocks
		);

	void GotBlock(v3s16 p);

	void SentBlock(v3s16 p);

	void SetBlockNotSent(v3s16 p);
	void SetBlocksNotSent(core::map<v3s16, MapBlock*> &blocks);

	s32 SendingCount()
	{
		return m_blocks_sending.size();
	}
	
	// Increments timeouts and removes timed-out blocks from list
	// NOTE: This doesn't fix the server-not-sending-block bug
	//       because it is related to emerging, not sending.
	//void RunSendingTimeouts(float dtime, float timeout);

	void PrintInfo(std::ostream &o)
	{
		o<<"RemoteClient "<<peer_id<<": "
				<<"m_blocks_sent.size()="<<m_blocks_sent.size()
				<<", m_blocks_sending.size()="<<m_blocks_sending.size()
				<<", m_nearest_unsent_d="<<m_nearest_unsent_d
				<<", m_excess_gotblocks="<<m_excess_gotblocks
				<<std::endl;
		m_excess_gotblocks = 0;
	}

	// Time from last placing or removing blocks
	float m_time_from_building;
	
	/*JMutex m_dig_mutex;
	float m_dig_time_remaining;
	// -1 = not digging
	s16 m_dig_tool_item;
	v3s16 m_dig_position;*/
	
	/*
		List of active objects that the client knows of.
		Value is dummy.
	*/
	core::map<u16, bool> m_known_objects;

private:
	/*
		Blocks that have been sent to client.
		- These don't have to be sent again.
		- A block is cleared from here when client says it has
		  deleted it from it's memory
		
		Key is position, value is dummy.
		No MapBlock* is stored here because the blocks can get deleted.
	*/
	core::map<v3s16, bool> m_blocks_sent;
	s16 m_nearest_unsent_d;
	v3s16 m_last_center;
	float m_nearest_unsent_reset_timer;
	
	/*
		Blocks that are currently on the line.
		This is used for throttling the sending of blocks.
		- The size of this list is limited to some value
		Block is added when it is sent with BLOCKDATA.
		Block is removed when GOTBLOCKS is received.
		Value is time from sending. (not used at the moment)
	*/
	core::map<v3s16, float> m_blocks_sending;

	/*
		Count of excess GotBlocks().
		There is an excess amount because the client sometimes
		gets a block so late that the server sends it again,
		and the client then sends two GOTBLOCKs.
		This is resetted by PrintInfo()
	*/
	u32 m_excess_gotblocks;
	
	// CPU usage optimization
	u32 m_nothing_to_send_counter;
	float m_nothing_to_send_pause_timer;
};

class Server : public con::PeerHandler, public MapEventReceiver,
		public InventoryManager
{
public:
	/*
		NOTE: Every public method should be thread-safe
	*/

	Server(
		std::string mapsavedir
	);
	~Server();
	void start(unsigned short port);
	void stop();
	// This is mainly a way to pass the time to the server.
	// Actual processing is done in an another thread.
	void step(float dtime);
	// This is run by ServerThread and does the actual processing
	void AsyncRunStep();
	void Receive();
	void ProcessData(u8 *data, u32 datasize, u16 peer_id);

	core::list<PlayerInfo> getPlayerInfo();

	/*u32 getDayNightRatio()
	{
		return time_to_daynight_ratio(m_time_of_day.get());
	}*/
	
	// Environment must be locked when called
	void setTimeOfDay(u32 time)
	{
		m_env.setTimeOfDay(time);
		m_time_of_day_send_timer = 0;
	}

	bool getShutdownRequested()
	{
		return m_shutdown_requested;
	}
	
	/*
		Shall be called with the environment locked.
		This is accessed by the map, which is inside the environment,
		so it shouldn't be a problem.
	*/
	void onMapEditEvent(MapEditEvent *event);

	/*
		Shall be called with the environment and the connection locked.
	*/
	Inventory* getInventory(InventoryContext *c, std::string id);
	void inventoryModified(InventoryContext *c, std::string id);

	// Connection must be locked when called
	std::wstring getStatusString();

	void requestShutdown(void)
	{
		m_shutdown_requested = true;
	}


	// Envlock and conlock should be locked when calling this
	void SendMovePlayer(Player *player);
	
	u64 getPlayerAuthPrivs(const std::string &name)
	{
		try{
			return m_authmanager.getPrivs(name);
		}
		catch(AuthNotFoundException &e)
		{
			dstream<<"WARNING: Auth not found for "<<name<<std::endl;
			return 0;
		}
	}

	void setPlayerAuthPrivs(const std::string &name, u64 privs)
	{
		try{
			return m_authmanager.setPrivs(name, privs);
		}
		catch(AuthNotFoundException &e)
		{
			dstream<<"WARNING: Auth not found for "<<name<<std::endl;
		}
	}

private:

	// con::PeerHandler implementation.
	// These queue stuff to be processed by handlePeerChanges().
	// As of now, these create and remove clients and players.
	void peerAdded(con::Peer *peer);
	void deletingPeer(con::Peer *peer, bool timeout);
	
	/*
		Static send methods
	*/
	
	static void SendHP(con::Connection &con, u16 peer_id, u8 hp);
	static void SendAccessDenied(con::Connection &con, u16 peer_id,
			const std::wstring &reason);
	
	/*
		Non-static send methods
	*/

	// Envlock and conlock should be locked when calling these
	void SendObjectData(float dtime);
	void SendPlayerInfos();
	void SendInventory(u16 peer_id);
	void SendChatMessage(u16 peer_id, const std::wstring &message);
	void BroadcastChatMessage(const std::wstring &message);
	void SendPlayerHP(Player *player);
	/*
		Send a node removal/addition event to all clients except ignore_id.
		Additionally, if far_players!=NULL, players further away than
		far_d_nodes are ignored and their peer_ids are added to far_players
	*/
	void sendRemoveNode(v3s16 p, u16 ignore_id=0,
			core::list<u16> *far_players=NULL, float far_d_nodes=100);
	void sendAddNode(v3s16 p, MapNode n, u16 ignore_id=0,
			core::list<u16> *far_players=NULL, float far_d_nodes=100);
	
	// Environment and Connection must be locked when called
	void SendBlockNoLock(u16 peer_id, MapBlock *block, u8 ver);
	
	// Sends blocks to clients
	void SendBlocks(float dtime);

	/*
		Something random
	*/
	
	void UpdateCrafting(u16 peer_id);
	
	// When called, connection mutex should be locked
	RemoteClient* getClient(u16 peer_id);
	
	/*
		Get a player from memory or creates one.
		If player is already connected, return NULL
		The password is not checked here - it is only used to
		set the password if a new player is created.

		Call with env and con locked.
	*/
	Player *emergePlayer(const char *name, const char *password, u16 peer_id);
	
	// Locks environment and connection by its own
	struct PeerChange;
	void handlePeerChange(PeerChange &c);
	void handlePeerChanges();

	u64 getPlayerPrivs(Player *player);

	/*
		Variables
	*/
	
	// Some timers
	float m_liquid_transform_timer;
	float m_print_info_timer;
	float m_objectdata_timer;
	float m_emergethread_trigger_timer;
	float m_savemap_timer;
	
	// NOTE: If connection and environment are both to be locked,
	// environment shall be locked first.

	// Environment
	ServerEnvironment m_env;
	JMutex m_env_mutex;
	
	// Connection
	con::Connection m_con;
	JMutex m_con_mutex;
	// Connected clients (behind the con mutex)
	core::map<u16, RemoteClient*> m_clients;

	// User authentication
	AuthManager m_authmanager;
	
	/*
		Threads
	*/
	
	// A buffer for time steps
	// step() increments and AsyncRunStep() run by m_thread reads it.
	float m_step_dtime;
	JMutex m_step_dtime_mutex;

	// The server mainly operates in this thread
	ServerThread m_thread;
	// This thread fetches and generates map
	EmergeThread m_emergethread;
	// Queue of block coordinates to be processed by the emerge thread
	BlockEmergeQueue m_emerge_queue;
	
	/*
		Time related stuff
	*/

	// 0-23999
	//MutexedVariable<u32> m_time_of_day;
	// Used to buffer dtime for adding to m_time_of_day
	float m_time_counter;
	// Timer for sending time of day over network
	float m_time_of_day_send_timer;
	// Uptime of server in seconds
	MutexedVariable<double> m_uptime;
	
	/*
		Peer change queue.
		Queues stuff from peerAdded() and deletingPeer() to
		handlePeerChanges()
	*/
	enum PeerChangeType
	{
		PEER_ADDED,
		PEER_REMOVED
	};
	struct PeerChange
	{
		PeerChangeType type;
		u16 peer_id;
		bool timeout;
	};
	Queue<PeerChange> m_peer_change_queue;

	/*
		Random stuff
	*/

	// Map directory
	std::string m_mapsavedir;

	bool m_shutdown_requested;
	
	/*
		Map edit event queue. Automatically receives all map edits.
		The constructor of this class registers us to receive them through
		onMapEditEvent

		NOTE: Should these be moved to actually be members of
		ServerEnvironment?
	*/

	/*
		Queue of map edits from the environment for sending to the clients
		This is behind m_env_mutex
	*/
	Queue<MapEditEvent*> m_unsent_map_edit_queue;
	/*
		Set to true when the server itself is modifying the map and does
		all sending of information by itself.
		This is behind m_env_mutex
	*/
	bool m_ignore_map_edit_events;
	/*
		If set to !=0, the incoming MapEditEvents are modified to have
		this peed id as the disabled recipient
		This is behind m_env_mutex
	*/
	u16 m_ignore_map_edit_events_peer_id;

	friend class EmergeThread;
	friend class RemoteClient;
};

/*
	Runs a simple dedicated server loop.

	Shuts down when run is set to false.
*/
void dedicated_server_loop(Server &server, bool &run);

#endif

