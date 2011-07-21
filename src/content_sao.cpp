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

#include "content_sao.h"
#include "collision.h"
#include "environment.h"

core::map<u16, ServerActiveObject::Factory> ServerActiveObject::m_types;

/*
	TestSAO
*/

// Prototype
TestSAO proto_TestSAO(NULL, 0, v3f(0,0,0));

TestSAO::TestSAO(ServerEnvironment *env, u16 id, v3f pos):
	ServerActiveObject(env, id, pos),
	m_timer1(0),
	m_age(0)
{
	ServerActiveObject::registerType(getType(), create);
}

ServerActiveObject* TestSAO::create(ServerEnvironment *env, u16 id, v3f pos,
		const std::string &data)
{
	return new TestSAO(env, id, pos);
}

void TestSAO::step(float dtime, bool send_recommended)
{
	m_age += dtime;
	if(m_age > 10)
	{
		m_removed = true;
		return;
	}

	m_base_position.Y += dtime * BS * 2;
	if(m_base_position.Y > 8*BS)
		m_base_position.Y = 2*BS;

	if(send_recommended == false)
		return;

	m_timer1 -= dtime;
	if(m_timer1 < 0.0)
	{
		m_timer1 += 0.125;
		//dstream<<"TestSAO: id="<<getId()<<" sending data"<<std::endl;

		std::string data;

		data += itos(0); // 0 = position
		data += " ";
		data += itos(m_base_position.X);
		data += " ";
		data += itos(m_base_position.Y);
		data += " ";
		data += itos(m_base_position.Z);

		ActiveObjectMessage aom(getId(), false, data);
		m_messages_out.push_back(aom);
	}
}


/*
	ItemSAO
*/

// Prototype
ItemSAO proto_ItemSAO(NULL, 0, v3f(0,0,0), "");

ItemSAO::ItemSAO(ServerEnvironment *env, u16 id, v3f pos,
		const std::string inventorystring):
	ServerActiveObject(env, id, pos),
	m_inventorystring(inventorystring),
	m_speed_f(0,0,0),
	m_last_sent_position(0,0,0)
{
	ServerActiveObject::registerType(getType(), create);
}

ServerActiveObject* ItemSAO::create(ServerEnvironment *env, u16 id, v3f pos,
		const std::string &data)
{
	std::istringstream is(data, std::ios::binary);
	char buf[1];
	// read version
	is.read(buf, 1);
	u8 version = buf[0];
	// check if version is supported
	if(version != 0)
		return NULL;
	std::string inventorystring = deSerializeString(is);
	dstream<<"ItemSAO::create(): Creating item \""
			<<inventorystring<<"\""<<std::endl;
	return new ItemSAO(env, id, pos, inventorystring);
}

void ItemSAO::step(float dtime, bool send_recommended)
{
	assert(m_env);

	const float interval = 0.2;
	if(m_move_interval.step(dtime, interval)==false)
		return;
	dtime = interval;
	
	core::aabbox3d<f32> box(-BS/3.,0.0,-BS/3., BS/3.,BS*2./3.,BS/3.);
	collisionMoveResult moveresult;
	// Apply gravity
	m_speed_f += v3f(0, -dtime*9.81*BS, 0);
	// Maximum movement without glitches
	f32 pos_max_d = BS*0.25;
	// Limit speed
	if(m_speed_f.getLength()*dtime > pos_max_d)
		m_speed_f *= pos_max_d / (m_speed_f.getLength()*dtime);
	v3f pos_f = getBasePosition();
	v3f pos_f_old = pos_f;
	moveresult = collisionMoveSimple(&m_env->getMap(), pos_max_d,
			box, dtime, pos_f, m_speed_f);
	
	if(send_recommended == false)
		return;

	if(pos_f.getDistanceFrom(m_last_sent_position) > 0.05*BS)
	{
		setBasePosition(pos_f);
		m_last_sent_position = pos_f;

		std::ostringstream os(std::ios::binary);
		char buf[6];
		// command (0 = update position)
		buf[0] = 0;
		os.write(buf, 1);
		// pos
		writeS32((u8*)buf, m_base_position.X*1000);
		os.write(buf, 4);
		writeS32((u8*)buf, m_base_position.Y*1000);
		os.write(buf, 4);
		writeS32((u8*)buf, m_base_position.Z*1000);
		os.write(buf, 4);
		// create message and add to list
		ActiveObjectMessage aom(getId(), false, os.str());
		m_messages_out.push_back(aom);
	}
}

std::string ItemSAO::getClientInitializationData()
{
	std::ostringstream os(std::ios::binary);
	char buf[6];
	// version
	buf[0] = 0;
	os.write(buf, 1);
	// pos
	writeS32((u8*)buf, m_base_position.X*1000);
	os.write(buf, 4);
	writeS32((u8*)buf, m_base_position.Y*1000);
	os.write(buf, 4);
	writeS32((u8*)buf, m_base_position.Z*1000);
	os.write(buf, 4);
	// inventorystring
	os<<serializeString(m_inventorystring);
	return os.str();
}

std::string ItemSAO::getStaticData()
{
	dstream<<__FUNCTION_NAME<<std::endl;
	std::ostringstream os(std::ios::binary);
	char buf[1];
	// version
	buf[0] = 0;
	os.write(buf, 1);
	// inventorystring
	os<<serializeString(m_inventorystring);
	return os.str();
}

InventoryItem * ItemSAO::createInventoryItem()
{
	try{
		std::istringstream is(m_inventorystring, std::ios_base::binary);
		InventoryItem *item = InventoryItem::deSerialize(is);
		dstream<<__FUNCTION_NAME<<": m_inventorystring=\""
				<<m_inventorystring<<"\" -> item="<<item
				<<std::endl;
		return item;
	}
	catch(SerializationError &e)
	{
		dstream<<__FUNCTION_NAME<<": serialization error: "
				<<"m_inventorystring=\""<<m_inventorystring<<"\""<<std::endl;
		return NULL;
	}
}


/*
	RatSAO
*/

// Prototype
RatSAO proto_RatSAO(NULL, 0, v3f(0,0,0));

RatSAO::RatSAO(ServerEnvironment *env, u16 id, v3f pos):
	ServerActiveObject(env, id, pos),
	m_is_active(false),
	m_speed_f(0,0,0)
{
	ServerActiveObject::registerType(getType(), create);

	m_oldpos = v3f(0,0,0);
	m_last_sent_position = v3f(0,0,0);
	m_yaw = 0;
	m_counter1 = 0;
	m_counter2 = 0;
	m_age = 0;
	m_touching_ground = false;
}

ServerActiveObject* RatSAO::create(ServerEnvironment *env, u16 id, v3f pos,
		const std::string &data)
{
	std::istringstream is(data, std::ios::binary);
	char buf[1];
	// read version
	is.read(buf, 1);
	u8 version = buf[0];
	// check if version is supported
	if(version != 0)
		return NULL;
	return new RatSAO(env, id, pos);
}

void RatSAO::step(float dtime, bool send_recommended)
{
	assert(m_env);

	if(m_is_active == false)
	{
		if(m_inactive_interval.step(dtime, 0.5)==false)
			return;
	}

	/*
		The AI
	*/

	/*m_age += dtime;
	if(m_age > 60)
	{
		// Die
		m_removed = true;
		return;
	}*/

	// Apply gravity
	m_speed_f.Y -= dtime*9.81*BS;

	/*
		Move around if some player is close
	*/
	bool player_is_close = false;
	// Check connected players
	core::list<Player*> players = m_env->getPlayers(true);
	core::list<Player*>::Iterator i;
	for(i = players.begin();
			i != players.end(); i++)
	{
		Player *player = *i;
		v3f playerpos = player->getPosition();
		if(m_base_position.getDistanceFrom(playerpos) < BS*10.0)
		{
			player_is_close = true;
			break;
		}
	}

	m_is_active = player_is_close;
	
	if(player_is_close == false)
	{
		m_speed_f.X = 0;
		m_speed_f.Z = 0;
	}
	else
	{
		// Move around
		v3f dir(cos(m_yaw/180*PI),0,sin(m_yaw/180*PI));
		f32 speed = 2*BS;
		m_speed_f.X = speed * dir.X;
		m_speed_f.Z = speed * dir.Z;

		if(m_touching_ground && (m_oldpos - m_base_position).getLength()
				< dtime*speed/2)
		{
			m_counter1 -= dtime;
			if(m_counter1 < 0.0)
			{
				m_counter1 += 1.0;
				m_speed_f.Y = 5.0*BS;
			}
		}

		{
			m_counter2 -= dtime;
			if(m_counter2 < 0.0)
			{
				m_counter2 += (float)(myrand()%100)/100*3.0;
				m_yaw += ((float)(myrand()%200)-100)/100*180;
				m_yaw = wrapDegrees(m_yaw);
			}
		}
	}
	
	m_oldpos = m_base_position;

	/*
		Move it, with collision detection
	*/

	core::aabbox3d<f32> box(-BS/3.,0.0,-BS/3., BS/3.,BS*2./3.,BS/3.);
	collisionMoveResult moveresult;
	// Maximum movement without glitches
	f32 pos_max_d = BS*0.25;
	// Limit speed
	if(m_speed_f.getLength()*dtime > pos_max_d)
		m_speed_f *= pos_max_d / (m_speed_f.getLength()*dtime);
	v3f pos_f = getBasePosition();
	v3f pos_f_old = pos_f;
	moveresult = collisionMoveSimple(&m_env->getMap(), pos_max_d,
			box, dtime, pos_f, m_speed_f);
	m_touching_ground = moveresult.touching_ground;
	
	setBasePosition(pos_f);

	if(send_recommended == false)
		return;

	if(pos_f.getDistanceFrom(m_last_sent_position) > 0.05*BS)
	{
		m_last_sent_position = pos_f;

		std::ostringstream os(std::ios::binary);
		// command (0 = update position)
		writeU8(os, 0);
		// pos
		writeV3F1000(os, m_base_position);
		// yaw
		writeF1000(os, m_yaw);
		// create message and add to list
		ActiveObjectMessage aom(getId(), false, os.str());
		m_messages_out.push_back(aom);
	}
}

std::string RatSAO::getClientInitializationData()
{
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 0);
	// pos
	writeV3F1000(os, m_base_position);
	return os.str();
}

std::string RatSAO::getStaticData()
{
	//dstream<<__FUNCTION_NAME<<std::endl;
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 0);
	return os.str();
}

InventoryItem* RatSAO::createPickedUpItem()
{
	std::istringstream is("CraftItem rat 1", std::ios_base::binary);
	InventoryItem *item = InventoryItem::deSerialize(is);
	return item;
}

/*
	Oerkki1SAO
*/

// Y is copied, X and Z change is limited
void accelerate_xz(v3f &speed, v3f target_speed, f32 max_increase)
{
	v3f d_wanted = target_speed - speed;
	d_wanted.Y = 0;
	f32 dl_wanted = d_wanted.getLength();
	f32 dl = dl_wanted;
	if(dl > max_increase)
		dl = max_increase;
	
	v3f d = d_wanted.normalize() * dl;

	speed.X += d.X;
	speed.Z += d.Z;
	speed.Y = target_speed.Y;
}

// Prototype
Oerkki1SAO proto_Oerkki1SAO(NULL, 0, v3f(0,0,0));

Oerkki1SAO::Oerkki1SAO(ServerEnvironment *env, u16 id, v3f pos):
	ServerActiveObject(env, id, pos),
	m_is_active(false),
	m_speed_f(0,0,0)
{
	ServerActiveObject::registerType(getType(), create);

	m_oldpos = v3f(0,0,0);
	m_last_sent_position = v3f(0,0,0);
	m_yaw = 0;
	m_counter1 = 0;
	m_counter2 = 0;
	m_age = 0;
	m_touching_ground = false;
	m_hp = 20;
	m_after_jump_timer = 0;
}

ServerActiveObject* Oerkki1SAO::create(ServerEnvironment *env, u16 id, v3f pos,
		const std::string &data)
{
	std::istringstream is(data, std::ios::binary);
	// read version
	u8 version = readU8(is);
	// read hp
	u8 hp = readU8(is);
	// check if version is supported
	if(version != 0)
		return NULL;
	Oerkki1SAO *o = new Oerkki1SAO(env, id, pos);
	o->m_hp = hp;
	return o;
}

void Oerkki1SAO::step(float dtime, bool send_recommended)
{
	assert(m_env);

	if(m_is_active == false)
	{
		if(m_inactive_interval.step(dtime, 0.5)==false)
			return;
	}

	/*
		The AI
	*/

	m_age += dtime;
	if(m_age > 120)
	{
		// Die
		m_removed = true;
		return;
	}

	m_after_jump_timer -= dtime;

	v3f old_speed = m_speed_f;

	// Apply gravity
	m_speed_f.Y -= dtime*9.81*BS;

	/*
		Move around if some player is close
	*/
	bool player_is_close = false;
	bool player_is_too_close = false;
	v3f near_player_pos;
	// Check connected players
	core::list<Player*> players = m_env->getPlayers(true);
	core::list<Player*>::Iterator i;
	for(i = players.begin();
			i != players.end(); i++)
	{
		Player *player = *i;
		v3f playerpos = player->getPosition();
		f32 dist = m_base_position.getDistanceFrom(playerpos);
		if(dist < BS*1.45)
		{
			player_is_too_close = true;
			near_player_pos = playerpos;
			break;
		}
		else if(dist < BS*15.0)
		{
			player_is_close = true;
			near_player_pos = playerpos;
		}
	}

	m_is_active = player_is_close;

	v3f target_speed = m_speed_f;

	if(!player_is_close)
	{
		target_speed = v3f(0,0,0);
	}
	else
	{
		// Move around

		v3f ndir = near_player_pos - m_base_position;
		ndir.Y = 0;
		ndir.normalize();

		f32 nyaw = 180./PI*atan2(ndir.Z,ndir.X);
		if(nyaw < m_yaw - 180)
			nyaw += 360;
		else if(nyaw > m_yaw + 180)
			nyaw -= 360;
		m_yaw = 0.95*m_yaw + 0.05*nyaw;
		m_yaw = wrapDegrees(m_yaw);
		
		f32 speed = 2*BS;

		if((m_touching_ground || m_after_jump_timer > 0.0)
				&& !player_is_too_close)
		{
			v3f dir(cos(m_yaw/180*PI),0,sin(m_yaw/180*PI));
			target_speed.X = speed * dir.X;
			target_speed.Z = speed * dir.Z;
		}

		if(m_touching_ground && (m_oldpos - m_base_position).getLength()
				< dtime*speed/2)
		{
			m_counter1 -= dtime;
			if(m_counter1 < 0.0)
			{
				m_counter1 += 0.2;
				// Jump
				target_speed.Y = 5.0*BS;
				m_after_jump_timer = 1.0;
			}
		}

		{
			m_counter2 -= dtime;
			if(m_counter2 < 0.0)
			{
				m_counter2 += (float)(myrand()%100)/100*3.0;
				//m_yaw += ((float)(myrand()%200)-100)/100*180;
				m_yaw += ((float)(myrand()%200)-100)/100*90;
				m_yaw = wrapDegrees(m_yaw);
			}
		}
	}
	
	if((m_speed_f - target_speed).getLength() > BS*4 || player_is_too_close)
		accelerate_xz(m_speed_f, target_speed, dtime*BS*8);
	else
		accelerate_xz(m_speed_f, target_speed, dtime*BS*4);
	
	m_oldpos = m_base_position;

	/*
		Move it, with collision detection
	*/

	core::aabbox3d<f32> box(-BS/3.,0.0,-BS/3., BS/3.,BS*5./3.,BS/3.);
	collisionMoveResult moveresult;
	// Maximum movement without glitches
	f32 pos_max_d = BS*0.25;
	/*// Limit speed
	if(m_speed_f.getLength()*dtime > pos_max_d)
		m_speed_f *= pos_max_d / (m_speed_f.getLength()*dtime);*/
	v3f pos_f = getBasePosition();
	v3f pos_f_old = pos_f;
	moveresult = collisionMovePrecise(&m_env->getMap(), pos_max_d,
			box, dtime, pos_f, m_speed_f);
	m_touching_ground = moveresult.touching_ground;
	
	// Do collision damage
	float tolerance = BS*12;
	float factor = BS*0.5;
	v3f speed_diff = old_speed - m_speed_f;
	// Increase effect in X and Z
	speed_diff.X *= 2;
	speed_diff.Z *= 2;
	float vel = speed_diff.getLength();
	if(vel > tolerance)
	{
		f32 damage_f = (vel - tolerance)/BS*factor;
		u16 damage = (u16)(damage_f+0.5);
		doDamage(damage);
	}

	setBasePosition(pos_f);

	if(send_recommended == false && m_speed_f.getLength() < 3.0*BS)
		return;

	if(pos_f.getDistanceFrom(m_last_sent_position) > 0.05*BS)
	{
		m_last_sent_position = pos_f;

		std::ostringstream os(std::ios::binary);
		// command (0 = update position)
		writeU8(os, 0);
		// pos
		writeV3F1000(os, m_base_position);
		// yaw
		writeF1000(os, m_yaw);
		// create message and add to list
		ActiveObjectMessage aom(getId(), false, os.str());
		m_messages_out.push_back(aom);
	}
}

std::string Oerkki1SAO::getClientInitializationData()
{
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 0);
	// pos
	writeV3F1000(os, m_base_position);
	return os.str();
}

std::string Oerkki1SAO::getStaticData()
{
	//dstream<<__FUNCTION_NAME<<std::endl;
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 0);
	// hp
	writeU8(os, m_hp);
	return os.str();
}

u16 Oerkki1SAO::punch(const std::string &toolname, v3f dir)
{
	m_speed_f += dir*12*BS;

	u16 amount = 5;
	doDamage(amount);
	return 65536/100;
}

void Oerkki1SAO::doDamage(u16 d)
{
	dstream<<"oerkki damage: "<<d<<std::endl;
	
	if(d < m_hp)
	{
		m_hp -= d;
	}
	else
	{
		// Die
		m_hp = 0;
		m_removed = true;
	}

	{
		std::ostringstream os(std::ios::binary);
		// command (1 = damage)
		writeU8(os, 1);
		// amount
		writeU8(os, d);
		// create message and add to list
		ActiveObjectMessage aom(getId(), false, os.str());
		m_messages_out.push_back(aom);
	}
}

/*
	FireflySAO
*/

// Prototype
FireflySAO proto_FireflySAO(NULL, 0, v3f(0,0,0));

FireflySAO::FireflySAO(ServerEnvironment *env, u16 id, v3f pos):
	ServerActiveObject(env, id, pos),
	m_is_active(false),
	m_speed_f(0,0,0)
{
	ServerActiveObject::registerType(getType(), create);

	m_oldpos = v3f(0,0,0);
	m_last_sent_position = v3f(0,0,0);
	m_yaw = 0;
	m_counter1 = 0;
	m_counter2 = 0;
	m_age = 0;
	m_touching_ground = false;
}

ServerActiveObject* FireflySAO::create(ServerEnvironment *env, u16 id, v3f pos,
		const std::string &data)
{
	std::istringstream is(data, std::ios::binary);
	char buf[1];
	// read version
	is.read(buf, 1);
	u8 version = buf[0];
	// check if version is supported
	if(version != 0)
		return NULL;
	return new FireflySAO(env, id, pos);
}

void FireflySAO::step(float dtime, bool send_recommended)
{
	assert(m_env);

	if(m_is_active == false)
	{
		if(m_inactive_interval.step(dtime, 0.5)==false)
			return;
	}

	/*
		The AI
	*/

	// Apply (less) gravity
	m_speed_f.Y -= dtime*3*BS;

	/*
		Move around if some player is close
	*/
	bool player_is_close = false;
	// Check connected players
	core::list<Player*> players = m_env->getPlayers(true);
	core::list<Player*>::Iterator i;
	for(i = players.begin();
			i != players.end(); i++)
	{
		Player *player = *i;
		v3f playerpos = player->getPosition();
		if(m_base_position.getDistanceFrom(playerpos) < BS*10.0)
		{
			player_is_close = true;
			break;
		}
	}

	m_is_active = player_is_close;
	
	if(player_is_close == false)
	{
		m_speed_f.X = 0;
		m_speed_f.Z = 0;
	}
	else
	{
		// Move around
		v3f dir(cos(m_yaw/180*PI),0,sin(m_yaw/180*PI));
		f32 speed = BS/2;
		m_speed_f.X = speed * dir.X;
		m_speed_f.Z = speed * dir.Z;

		if(m_touching_ground && (m_oldpos - m_base_position).getLength()
				< dtime*speed/2)
		{
			m_counter1 -= dtime;
			if(m_counter1 < 0.0)
			{
				m_counter1 += 1.0;
				m_speed_f.Y = 5.0*BS;
			}
		}

		{
			m_counter2 -= dtime;
			if(m_counter2 < 0.0)
			{
				m_counter2 += (float)(myrand()%100)/100*3.0;
				m_yaw += ((float)(myrand()%200)-100)/100*180;
				m_yaw = wrapDegrees(m_yaw);
			}
		}
	}
	
	m_oldpos = m_base_position;

	/*
		Move it, with collision detection
	*/

	core::aabbox3d<f32> box(-BS/3.,0.0,-BS/3., BS/3.,BS*2./3.,BS/3.);
	collisionMoveResult moveresult;
	// Maximum movement without glitches
	f32 pos_max_d = BS*0.25;
	// Limit speed
	if(m_speed_f.getLength()*dtime > pos_max_d)
		m_speed_f *= pos_max_d / (m_speed_f.getLength()*dtime);
	v3f pos_f = getBasePosition();
	v3f pos_f_old = pos_f;
	moveresult = collisionMoveSimple(&m_env->getMap(), pos_max_d,
			box, dtime, pos_f, m_speed_f);
	m_touching_ground = moveresult.touching_ground;
	
	setBasePosition(pos_f);

	if(send_recommended == false)
		return;

	if(pos_f.getDistanceFrom(m_last_sent_position) > 0.05*BS)
	{
		m_last_sent_position = pos_f;

		std::ostringstream os(std::ios::binary);
		// command (0 = update position)
		writeU8(os, 0);
		// pos
		writeV3F1000(os, m_base_position);
		// yaw
		writeF1000(os, m_yaw);
		// create message and add to list
		ActiveObjectMessage aom(getId(), false, os.str());
		m_messages_out.push_back(aom);
	}
}

std::string FireflySAO::getClientInitializationData()
{
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 0);
	// pos
	writeV3F1000(os, m_base_position);
	return os.str();
}

std::string FireflySAO::getStaticData()
{
	//dstream<<__FUNCTION_NAME<<std::endl;
	std::ostringstream os(std::ios::binary);
	// version
	writeU8(os, 0);
	return os.str();
}

InventoryItem* FireflySAO::createPickedUpItem()
{
	std::istringstream is("CraftItem firefly 1", std::ios_base::binary);
	InventoryItem *item = InventoryItem::deSerialize(is);
	return item;
}
