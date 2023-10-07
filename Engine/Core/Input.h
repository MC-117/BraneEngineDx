#pragma once
#ifndef _INPUT_H_
#define _INPUT_H_

#include "Unit.h"
#include <dinput.h>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

enum MouseButtonEnum
{
	Left, Right, Middle
};

enum InputStateEnum
{
	Up, Down, OnPress, OnRelease
};

class ENGINE_API Input
{
public:
	Input(string name = "Input");
	virtual ~Input();

	virtual void init();
	virtual void update();
	virtual void release();

	void setHWND(HWND hwnd);
	string getKeyName(char keyValue);
	InputStateEnum getKeyState(char key);
	bool getKeyUp(char key);
	bool getKeyDown(char key);
	bool getKeyPress(char key);
	bool getKeyRelease(char key);
	Unit2Di getMousePos();
	Unit2Di getCursorPos();
	Unit2Di getPreMousePos();
	Unit2Di getPreCursorPos();
	Unit2Di getMouseMove();
	Unit2Di getCursorMove();
	int getMouseWheelValue();
	InputStateEnum getMouseButtonState(MouseButtonEnum button);
	bool getMouseButtonUp(MouseButtonEnum button);
	bool getMouseButtonDown(MouseButtonEnum button);
	bool getMouseButtonPress(MouseButtonEnum button);
	bool getMouseButtonRelease(MouseButtonEnum button);
	bool getCursorHidden();
	void setCursorHidden(bool hidden);
	void setPause(bool v = true);
protected:
	string name;
	HWND _hwnd = 0;
	bool pause = false;
	LPDIRECTINPUT8 _directInput;
	LPDIRECTINPUTDEVICE8 _mouseDevice;
	Unit2Di mousePos = { -1, -1 }, mousePosPrevious = { -1, -1 };
	Unit2Di cursorPos = { -1, -1 }, cursorPosPrevious = { -1, -1 };
	InputStateEnum mouseState[3] = { Up, Up, Up };
	int mouseWheelVal = 0, cursorVisibilityCount = 0;
	bool cursorHidden = false;
	InputStateEnum keyState[255];
	const char* VKSet[255] = {
		"None",
		"Left_Mouse",
		"Right_Mouse",
		"Cancel",
		"Middle_Mouse",
		"Xbutton1",
		"Xbutton2",
		"Undefined",
		"Backspace",
		"Tab",
		"Undefined",
		"Undefined",
		"Clear",
		"Enter",
		"Undefined",
		"Undefined",
		"Shift",
		"Ctrl",
		"Alt",
		"Pause",
		"Caps_Lock",
		"Kana_Hanguel_Hangul",
		"Undefined",
		"Junja",
		"Final",
		"Hanja_Kanji",
		"Undefined",
		"Esc",
		"Convent",
		"NonConvent",
		"Accept",
		"Modechange",
		"Space",
		"Page_Up",
		"Page_Down",
		"End",
		"Home",
		"Left",
		"Up",
		"Right",
		"Down",
		"Select",
		"Print",
		"Execute",
		"Snapshot",
		"Insert",
		"Delete",
		"Help",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"LWin",
		"RWin",
		"Apps",
		"Undefined",
		"Sleep",
		"SKB_0",
		"SKB_1",
		"SKB_2",
		"SKB_3",
		"SKB_4",
		"SKB_5",
		"SKB_6",
		"SKB_7",
		"SKB_8",
		"SKB_9",
		"SKB_*",
		"SKB_+",
		"SKB_Enter",
		"SKB_-",
		"SKB_.",
		"SKB_/",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"F13",
		"F14",
		"F15",
		"F16",
		"F17",
		"F18",
		"F19",
		"F20",
		"F21",
		"F22",
		"F23",
		"F24",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Num_Lock",
		"Scroll",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"LShift",
		"RShift",
		"LCtrl",
		"RCtrl",
		"LMenu",
		"RMenu",
		"Browser_Back",
		"Browser_Forward",
		"Browser_Refresh",
		"Browser_Stop",
		"Browser_Search",
		"Browser_Favorites",
		"Browser_Home",
		"VolumeMute",
		"VolumeDown",
		"VolumeUp",
		"Media_Next",
		"Media_Prev",
		"Media_Stop",
		"Media_Play_Pause",
		"Launch_Mail",
		"Launch_Media_Select",
		"Launch_App1",
		"Launch_App2",
		"Undefined",
		"Undefined",
		";_:",
		"=_+",
		",_.",
		"-__",
		"._>",
		"/_?",
		"`_~",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"Undefined",
		"[_{",
		"\\_|",
		"]_}",
		"'_\"",
		"OEM_8",
		"Undefined",
		"OEM_S",
		"OEM_102",
		"OEM_S",
		"OEM_S",
		"Processkey",
		"OEM_S",
		"Packet",
		"Undefined",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"OEM_S",
		"Attn",
		"Crsel",
		"Exsel",
		"Ereof",
		"Play",
		"Zoom",
		"Noname",
		"Pa1",
		"OEM_Clear"
	};

	virtual void mouseInput(int button, int state, int x, int y);
	virtual void mouseMove(int x, int y);
	virtual void mouseWheelInput(int dir, int x, int y);
	virtual void keyboardInput(char key, int state);

	bool readDevice(LPDIRECTINPUTDEVICE8 pDIDevice, void *pBuffer, long lSize);
	void hideCursor(bool hidden);
};

#endif // !_INPUT_H_
