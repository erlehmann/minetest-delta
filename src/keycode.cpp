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

#include "keycode.h"
#include "main.h" // For g_settings
#define CHECKKEY(x){if(strcmp(name, #x)==0) return irr::x;}

irr::EKEY_CODE keyname_to_keycode(const char *name)
{
	CHECKKEY(KEY_LBUTTON)
	CHECKKEY(KEY_RBUTTON)
	CHECKKEY(KEY_CANCEL)
	CHECKKEY(KEY_MBUTTON)
	CHECKKEY(KEY_XBUTTON1)
	CHECKKEY(KEY_XBUTTON2)
	CHECKKEY(KEY_BACK)
	CHECKKEY(KEY_TAB)
	CHECKKEY(KEY_CLEAR)
	CHECKKEY(KEY_RETURN)
	CHECKKEY(KEY_SHIFT)
	CHECKKEY(KEY_CONTROL)
	CHECKKEY(KEY_MENU)
	CHECKKEY(KEY_PAUSE)
	CHECKKEY(KEY_CAPITAL)
	CHECKKEY(KEY_KANA)
	CHECKKEY(KEY_HANGUEL)
	CHECKKEY(KEY_HANGUL)
	CHECKKEY(KEY_JUNJA)
	CHECKKEY(KEY_FINAL)
	CHECKKEY(KEY_HANJA)
	CHECKKEY(KEY_KANJI)
	CHECKKEY(KEY_ESCAPE)
	CHECKKEY(KEY_CONVERT)
	CHECKKEY(KEY_NONCONVERT)
	CHECKKEY(KEY_ACCEPT)
	CHECKKEY(KEY_MODECHANGE)
	CHECKKEY(KEY_SPACE)
	CHECKKEY(KEY_PRIOR)
	CHECKKEY(KEY_NEXT)
	CHECKKEY(KEY_END)
	CHECKKEY(KEY_HOME)
	CHECKKEY(KEY_LEFT)
	CHECKKEY(KEY_UP)
	CHECKKEY(KEY_RIGHT)
	CHECKKEY(KEY_DOWN)
	CHECKKEY(KEY_SELECT)
	CHECKKEY(KEY_PRINT)
	CHECKKEY(KEY_EXECUT)
	CHECKKEY(KEY_SNAPSHOT)
	CHECKKEY(KEY_INSERT)
	CHECKKEY(KEY_DELETE)
	CHECKKEY(KEY_HELP)
	CHECKKEY(KEY_KEY_0)
	CHECKKEY(KEY_KEY_1)
	CHECKKEY(KEY_KEY_2)
	CHECKKEY(KEY_KEY_3)
	CHECKKEY(KEY_KEY_4)
	CHECKKEY(KEY_KEY_5)
	CHECKKEY(KEY_KEY_6)
	CHECKKEY(KEY_KEY_7)
	CHECKKEY(KEY_KEY_8)
	CHECKKEY(KEY_KEY_9)
	CHECKKEY(KEY_KEY_A)
	CHECKKEY(KEY_KEY_B)
	CHECKKEY(KEY_KEY_C)
	CHECKKEY(KEY_KEY_D)
	CHECKKEY(KEY_KEY_E)
	CHECKKEY(KEY_KEY_F)
	CHECKKEY(KEY_KEY_G)
	CHECKKEY(KEY_KEY_H)
	CHECKKEY(KEY_KEY_I)
	CHECKKEY(KEY_KEY_J)
	CHECKKEY(KEY_KEY_K)
	CHECKKEY(KEY_KEY_L)
	CHECKKEY(KEY_KEY_M)
	CHECKKEY(KEY_KEY_N)
	CHECKKEY(KEY_KEY_O)
	CHECKKEY(KEY_KEY_P)
	CHECKKEY(KEY_KEY_Q)
	CHECKKEY(KEY_KEY_R)
	CHECKKEY(KEY_KEY_S)
	CHECKKEY(KEY_KEY_T)
	CHECKKEY(KEY_KEY_U)
	CHECKKEY(KEY_KEY_V)
	CHECKKEY(KEY_KEY_W)
	CHECKKEY(KEY_KEY_X)
	CHECKKEY(KEY_KEY_Y)
	CHECKKEY(KEY_KEY_Z)
	CHECKKEY(KEY_LWIN)
	CHECKKEY(KEY_RWIN)
	CHECKKEY(KEY_APPS)
	CHECKKEY(KEY_SLEEP)
	CHECKKEY(KEY_NUMPAD0)
	CHECKKEY(KEY_NUMPAD1)
	CHECKKEY(KEY_NUMPAD2)
	CHECKKEY(KEY_NUMPAD3)
	CHECKKEY(KEY_NUMPAD4)
	CHECKKEY(KEY_NUMPAD5)
	CHECKKEY(KEY_NUMPAD6)
	CHECKKEY(KEY_NUMPAD7)
	CHECKKEY(KEY_NUMPAD8)
	CHECKKEY(KEY_NUMPAD9)
	CHECKKEY(KEY_MULTIPLY)
	CHECKKEY(KEY_ADD)
	CHECKKEY(KEY_SEPARATOR)
	CHECKKEY(KEY_SUBTRACT)
	CHECKKEY(KEY_DECIMAL)
	CHECKKEY(KEY_DIVIDE)
	CHECKKEY(KEY_F1)
	CHECKKEY(KEY_F2)
	CHECKKEY(KEY_F3)
	CHECKKEY(KEY_F4)
	CHECKKEY(KEY_F5)
	CHECKKEY(KEY_F6)
	CHECKKEY(KEY_F7)
	CHECKKEY(KEY_F8)
	CHECKKEY(KEY_F9)
	CHECKKEY(KEY_F10)
	CHECKKEY(KEY_F11)
	CHECKKEY(KEY_F12)
	CHECKKEY(KEY_F13)
	CHECKKEY(KEY_F14)
	CHECKKEY(KEY_F15)
	CHECKKEY(KEY_F16)
	CHECKKEY(KEY_F17)
	CHECKKEY(KEY_F18)
	CHECKKEY(KEY_F19)
	CHECKKEY(KEY_F20)
	CHECKKEY(KEY_F21)
	CHECKKEY(KEY_F22)
	CHECKKEY(KEY_F23)
	CHECKKEY(KEY_F24)
	CHECKKEY(KEY_NUMLOCK)
	CHECKKEY(KEY_SCROLL)
	CHECKKEY(KEY_LSHIFT)
	CHECKKEY(KEY_RSHIFT)
	CHECKKEY(KEY_LCONTROL)
	CHECKKEY(KEY_RCONTROL)
	CHECKKEY(KEY_LMENU)
	CHECKKEY(KEY_RMENU)
	CHECKKEY(KEY_PLUS)
	CHECKKEY(KEY_COMMA)
	CHECKKEY(KEY_MINUS)
	CHECKKEY(KEY_PERIOD)
	CHECKKEY(KEY_ATTN)
	CHECKKEY(KEY_CRSEL)
	CHECKKEY(KEY_EXSEL)
	CHECKKEY(KEY_EREOF)
	CHECKKEY(KEY_PLAY)
	CHECKKEY(KEY_ZOOM)
	CHECKKEY(KEY_PA1)
	CHECKKEY(KEY_OEM_CLEAR)

	return irr::KEY_KEY_CODES_COUNT;
}

static const char *KeyNames[] =
{ "-", "KEY_LBUTTON", "KEY_RBUTTON", "Cancel", "Middle Button", "X Button 1",
		"X Button 2", "-", "Back", "Tab", "-", "-", "Clear", "Return", "-",
		"-", "KEY_SHIFT", "Control", "Menu", "Pause", "Capital", "Kana", "-",
		"Junja", "Final", "Kanji", "-", "Escape", "Convert", "Nonconvert",
		"Accept", "Mode Change", "KEY_SPACE", "Priot", "Next", "KEY_END",
		"KEY_HOME", "Left", "Up", "Right", "Down", "Select", "KEY_PRINT",
		"Execute", "Snapshot", "Insert", "Delete", "Help", "KEY_KEY_0",
		"KEY_KEY_1", "KEY_KEY_2", "KEY_KEY_3", "KEY_KEY_4", "KEY_KEY_5",
		"KEY_KEY_6", "KEY_KEY_7", "KEY_KEY_8", "KEY_KEY_9", "-", "-", "-", "-",
		"-", "-", "-", "KEY_KEY_A", "KEY_KEY_B", "KEY_KEY_C", "KEY_KEY_D",
		"KEY_KEY_E", "KEY_KEY_F", "KEY_KEY_G", "KEY_KEY_H", "KEY_KEY_I",
		"KEY_KEY_J", "KEY_KEY_K", "KEY_KEY_L", "KEY_KEY_M", "KEY_KEY_N",
		"KEY_KEY_O", "KEY_KEY_P", "KEY_KEY_Q", "KEY_KEY_R", "KEY_KEY_S",
		"KEY_KEY_T", "KEY_KEY_U", "KEY_KEY_V", "KEY_KEY_W", "KEY_KEY_X",
		"KEY_KEY_Y", "KEY_KEY_Z", "Left Windows", "Right Windows", "Apps", "-",
		"Sleep", "KEY_NUMPAD0", "KEY_NUMPAD1", "KEY_NUMPAD2", "KEY_NUMPAD3",
		"KEY_NUMPAD4", "KEY_NUMPAD5", "KEY_NUMPAD6", "KEY_NUMPAD7",
		"KEY_NUMPAD8", "KEY_NUMPAD9", "Numpad *", "Numpad +", "Numpad /",
		"Numpad -", "Numpad .", "Numpad /", "KEY_F1", "KEY_F2", "KEY_F3",
		"KEY_F4", "KEY_F5", "KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10",
		"KEY_F11", "KEY_F12", "KEY_F13", "KEY_F14", "KEY_F15", "KEY_F16",
		"KEY_F17", "KEY_F18", "KEY_F19", "KEY_F20", "KEY_F21", "KEY_F22",
		"KEY_F23", "KEY_F24", "-", "-", "-", "-", "-", "-", "-", "-",
		"Num Lock", "Scroll Lock", "-", "-", "-", "-", "-", "-", "-", "-", "-",
		"-", "-", "-", "-", "-", "KEY_LSHIFT", "KEY_RSHIFT", "Left Control",
		"Right Control", "Left Menu", "Right Menu", "-", "-", "-", "-", "-",
		"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
		"-", "-", "Plus", "Comma", "Minus", "Period", "-", "-", "-", "-", "-",
		"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
		"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
		"-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-", "-",
		"-", "-", "-", "-", "-", "-", "-", "-", "Attn", "CrSel", "ExSel",
		"Erase OEF", "Play", "Zoom", "PA1", "OEM Clear", "-" };

std::string keycode_to_keyname(s32 keycode)
{
	return KeyNames[keycode];
}
/*
 Key config
 */

// A simple cache for quicker lookup
core::map<std::string, irr::EKEY_CODE> g_key_setting_cache;

irr::EKEY_CODE getKeySetting(const char *settingname)
{
	core::map<std::string, irr::EKEY_CODE>::Node *n;
	n = g_key_setting_cache.find(settingname);
	if (n)
		return n->getValue();
	irr::EKEY_CODE c = keyname_to_keycode(g_settings.get(settingname).c_str());
	g_key_setting_cache.insert(settingname, c);
	return c;
}

void clearKeyCache()
{
	g_key_setting_cache.clear();
}

