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

#ifndef MODALMENU_HEADER
#define MODALMENU_HEADER

#include "common_irrlicht.h"

//TODO: Change GUIElement private
class GUIModalMenu : public gui::IGUIElement
{
public:
	GUIModalMenu(gui::IGUIEnvironment* env,
			gui::IGUIElement* parent, s32 id,
			int *active_menu_count):
		IGUIElement(gui::EGUIET_ELEMENT, env, parent, id,
				core::rect<s32>(0,0,100,100))
	{
		m_active_menu_count = active_menu_count;
		m_allow_focus_removal = false;
		m_screensize_old = v2u32(0,0);

		setVisible(true);
		Environment->setFocus(this);
		(*m_active_menu_count)++;
	}
	virtual ~GUIModalMenu()
	{
		(*m_active_menu_count)--;
	}

	bool canTakeFocus(gui::IGUIElement *e)
	{
		return (e && (e == this || isMyChild(e))) || m_allow_focus_removal;
	}

	void quitMenu()
	{
		m_allow_focus_removal = true;
		// This removes Environment's grab on us
		Environment->removeFocus(this);
		this->remove();
	}

	virtual void regenerateGui(v2u32 screensize) = 0;

	virtual void drawMenu() = 0;

	void draw()
	{
		if(!IsVisible)
			return;
			
		video::IVideoDriver* driver = Environment->getVideoDriver();
		v2u32 screensize = driver->getScreenSize();
		if(screensize != m_screensize_old)
		{
			m_screensize_old = screensize;
			regenerateGui(screensize);
		}

		drawMenu();
	}
	
	virtual bool OnEvent(const SEvent& event) { return false; };
	
private:
	int *m_active_menu_count;
	bool m_allow_focus_removal;
	v2u32 m_screensize_old;
};


#endif

