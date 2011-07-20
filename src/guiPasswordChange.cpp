/*
Part of Minetest-c55
Copyright (C) 2011 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2011 Ciaran Gultnieks <ciaran@ciarang.com>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "guiPasswordChange.h"
#include "debug.h"
#include "serialization.h"
#include <string>

#include "gettext.h"

const int ID_oldPassword = 256;
const int ID_newPassword1 = 257;
const int ID_newPassword2 = 258;
const int ID_change = 259;
const int ID_message = 260;

GUIPasswordChange::GUIPasswordChange(gui::IGUIEnvironment* env,
		gui::IGUIElement* parent, s32 id,
		IMenuManager *menumgr,
		Client* client
):
	GUIModalMenu(env, parent, id, menumgr),
	m_client(client)
{
}

GUIPasswordChange::~GUIPasswordChange()
{
	removeChildren();
}

void GUIPasswordChange::removeChildren()
{
	{
		gui::IGUIElement *e = getElementFromId(ID_oldPassword);
		if(e != NULL)
			e->remove();
	}
	{
		gui::IGUIElement *e = getElementFromId(ID_newPassword1);
		if(e != NULL)
			e->remove();
	}
	{
		gui::IGUIElement *e = getElementFromId(ID_newPassword2);
		if(e != NULL)
			e->remove();
	}
	{
		gui::IGUIElement *e = getElementFromId(ID_change);
		if(e != NULL)
			e->remove();
	}
}

void GUIPasswordChange::regenerateGui(v2u32 screensize)
{
	/*
		Remove stuff
	*/
	removeChildren();
	
	/*
		Calculate new sizes and positions
	*/
	core::rect<s32> rect(
			screensize.X/2 - 580/2,
			screensize.Y/2 - 300/2,
			screensize.X/2 + 580/2,
			screensize.Y/2 + 300/2
	);
	
	DesiredRect = rect;
	recalculateAbsolutePosition(false);

	v2s32 size = rect.getSize();
	v2s32 topleft_client(40, 0);
	v2s32 size_client = size - v2s32(40, 0);

	/*
		Add stuff
	*/
	s32 ypos = 50;
	{
		core::rect<s32> rect(0, 0, 110, 20);
		rect += topleft_client + v2s32(35, ypos+6);
		Environment->addStaticText(chartowchar_t(gettext("Old Password")), rect, false, true, this, -1);
	}
	{
		core::rect<s32> rect(0, 0, 230, 30);
		rect += topleft_client + v2s32(160, ypos);
		gui::IGUIEditBox *e = 
		Environment->addEditBox(L"", rect, true, this, ID_oldPassword);
		Environment->setFocus(e);
		e->setPasswordBox(true);
	}
	ypos += 50;
	{
		core::rect<s32> rect(0, 0, 110, 20);
		rect += topleft_client + v2s32(35, ypos+6);
		Environment->addStaticText(chartowchar_t(gettext("New Password")), rect, false, true, this, -1);
	}
	{
		core::rect<s32> rect(0, 0, 230, 30);
		rect += topleft_client + v2s32(160, ypos);
		gui::IGUIEditBox *e = 
		Environment->addEditBox(L"", rect, true, this, ID_newPassword1);
		e->setPasswordBox(true);
	}
	ypos += 50;
	{
		core::rect<s32> rect(0, 0, 110, 20);
		rect += topleft_client + v2s32(35, ypos+6);
		Environment->addStaticText(chartowchar_t(gettext("Confirm Password")), rect, false, true, this, -1);
	}
	{
		core::rect<s32> rect(0, 0, 230, 30);
		rect += topleft_client + v2s32(160, ypos);
		gui::IGUIEditBox *e = 
		Environment->addEditBox(L"", rect, true, this, ID_newPassword2);
		e->setPasswordBox(true);
	}

	ypos += 50;
	{
		core::rect<s32> rect(0, 0, 140, 30);
		rect = rect + v2s32(size.X/2-140/2, ypos);
		Environment->addButton(rect, this, ID_change, chartowchar_t(gettext("Change")));
	}

	ypos += 50;
	{
		core::rect<s32> rect(0, 0, 300, 20);
		rect += topleft_client + v2s32(35, ypos);
		IGUIElement *e = 
		Environment->addStaticText(chartowchar_t(gettext("Passwords do not match!")), rect, false, true, this, ID_message);
		e->setVisible(false);
	}

}

void GUIPasswordChange::drawMenu()
{
	gui::IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;
	video::IVideoDriver* driver = Environment->getVideoDriver();
	
	video::SColor bgcolor(140,0,0,0);
	driver->draw2DRectangle(bgcolor, AbsoluteRect, &AbsoluteClippingRect);

	gui::IGUIElement::draw();
}

bool GUIPasswordChange::acceptInput()
{
		std::wstring oldpass;
		std::wstring newpass;
		gui::IGUIElement *e;
		e = getElementFromId(ID_oldPassword);
		if(e != NULL)
			oldpass = e->getText();
		e = getElementFromId(ID_newPassword1);
		if(e != NULL)
			newpass = e->getText();
		e = getElementFromId(ID_newPassword2);
		if(e != NULL && newpass != e->getText())
		{
			e = getElementFromId(ID_message);
			if(e != NULL)
				e->setVisible(true);
			return false;
		}
		m_client->sendChangePassword(oldpass, newpass);
		return true;
}

bool GUIPasswordChange::OnEvent(const SEvent& event)
{
	if(event.EventType==EET_KEY_INPUT_EVENT)
	{
		if(event.KeyInput.Key==KEY_ESCAPE && event.KeyInput.PressedDown)
		{
			quitMenu();
			return true;
		}
		if(event.KeyInput.Key==KEY_RETURN && event.KeyInput.PressedDown)
		{
			if(acceptInput())
				quitMenu();
			return true;
		}
	}
	if(event.EventType==EET_GUI_EVENT)
	{
		if(event.GUIEvent.EventType==gui::EGET_ELEMENT_FOCUS_LOST
				&& isVisible())
		{
			if(!canTakeFocus(event.GUIEvent.Element))
			{
				dstream<<"GUIPasswordChange: Not allowing focus change."
						<<std::endl;
				// Returning true disables focus change
				return true;
			}
		}
		if(event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED)
		{
			switch(event.GUIEvent.Caller->getID())
			{
			case ID_change:
				if(acceptInput())
					quitMenu();
				return true;
			}
		}
		if(event.GUIEvent.EventType==gui::EGET_EDITBOX_ENTER)
		{
			switch(event.GUIEvent.Caller->getID())
			{
			case ID_oldPassword:
			case ID_newPassword1:
			case ID_newPassword2:
				if(acceptInput())
					quitMenu();
				return true;
			}
		}
	}

	return Parent ? Parent->OnEvent(event) : false;
}

