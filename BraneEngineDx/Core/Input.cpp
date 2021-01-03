#include "Input.h"

Input::Input(string name) : Object::Object(name)
{
	memset(keyState, 0, sizeof(keyState));
}

Input::~Input()
{
}

void Input::setHWND(HWND hwnd)
{
	_hwnd = hwnd;
}

string Input::getKeyName(char keyValue)
{
	if (keyValue < 255)
		return VKSet[keyValue];
	return "";
}

InputStateEnum Input::getKeyState(char key)
{
	if (key < 255)
		return keyState[key];
	return Up;
}

bool Input::getKeyUp(char key)
{
	if (key < 255)
		return keyState[key] == Up || keyState[key] == OnRelease;
	return false;
}

bool Input::getKeyDown(char key)
{
	if (key < 255)
		return keyState[key] == Down || keyState[key] == OnPress;
	return false;
}

bool Input::getKeyPress(char key)
{
	if (key < 255)
		return keyState[key] == OnPress;
	return false;
}

bool Input::getKeyRelease(char key)
{
	if (key < 255)
		return keyState[key] == OnRelease;
	return false;
}

Unit2Di Input::getMousePos()
{
	return mousePos;
}

Unit2Di Input::getCursorPos()
{
	return cursorPos;
}

Unit2Di Input::getPreMousePos()
{
	return mousePosPrevious;
}

Unit2Di Input::getPreCursorPos()
{
	return cursorPosPrevious;
}

Unit2Di Input::getMouseMove()
{
	if (mousePos == Unit2Di::create(-1) || mousePosPrevious == Unit2Di::create(-1))
		return{ 0, 0 };
	else
		return mousePos - mousePosPrevious;
}

Unit2Di Input::getCursorMove()
{
	if (cursorPos == Unit2Di::create(-1) || cursorPosPrevious == Unit2Di::create(-1))
		return{ 0, 0 };
	else
		return cursorPos - cursorPosPrevious;
}

int Input::getMouseWheelValue()
{
	return mouseWheelVal;
}

InputStateEnum Input::getMouseButtonState(MouseButtonEnum button)
{
	return mouseState[button];
}

bool Input::getMouseButtonUp(MouseButtonEnum button)
{
	return mouseState[button] == Up || mouseState[button] == OnRelease;
}

bool Input::getMouseButtonDown(MouseButtonEnum button)
{
	return mouseState[button] == Down || mouseState[button] == OnPress;
}

bool Input::getMouseButtonPress(MouseButtonEnum button)
{
	return mouseState[button] == OnPress;
}

bool Input::getMouseButtonRelease(MouseButtonEnum button)
{
	return mouseState[button] == OnRelease;
}

bool Input::getCursorHidden()
{
	return cursorHidden;
}

void Input::setCursorHidden(bool hidden)
{
	cursorHidden = hidden;
}

void Input::hideCursor(bool hidden)
{
	while ((cursorVisibilityCount < 0) ^ hidden)
		cursorVisibilityCount = ShowCursor(!hidden);
}

void Input::setPause(bool v)
{
	pause = v;
}

void Input::mouseInput(int button, int state, int x, int y)
{
	mouseState[button] = (InputStateEnum)state;
}

void Input::mouseMove(int x, int y)
{
}

void Input::mouseWheelInput(int dir, int x, int y)
{
	mouseWheelVal = dir;
}

void Input::keyboardInput(char key, int state)
{
	if (key == VK_LBUTTON || key == VK_MBUTTON || key == VK_RBUTTON)
		mouseInput(key / 2, state, mousePos.x, mousePos.y);
}

bool Input::readDevice(LPDIRECTINPUTDEVICE8 pDIDevice, void * pBuffer, long lSize)
{
	HRESULT hr;
	while (true)
	{
		pDIDevice->Poll();
		pDIDevice->Acquire();
		if (SUCCEEDED(hr = pDIDevice->GetDeviceState(lSize, pBuffer)))
			break;
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
			return false;
		if (FAILED(pDIDevice->Acquire()))
			return false;
	}
	return true;
}

void Input::begin()
{
	Object::begin();
	if (FAILED(DirectInput8Create(GetModuleHandle(NULL), 0x800, IID_IDirectInput8, (void**)&_directInput, NULL)))
		cout << "DirectInput init failed\n";
	else if (FAILED(_directInput->CreateDevice(GUID_SysMouse, &_mouseDevice, NULL)))
		cout << "DirectInput mouse access failed\n";
	else {
		_mouseDevice->SetDataFormat(&c_dfDIMouse);
		_mouseDevice->SetCooperativeLevel(_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	}
}

void Input::tick(float deltaTime)
{
	if (GetActiveWindow() == _hwnd) {
		DIMOUSESTATE dimouse;
		POINT cp;
		GetCursorPos(&cp);
		ScreenToClient(_hwnd, &cp);
		cursorPosPrevious = cursorPos;
		cursorPos = { cp.x, cp.y };
		mousePosPrevious = mousePos;
		readDevice(_mouseDevice, &dimouse, sizeof(dimouse));
		if (mousePos == Unit2Di::create(-1))
			mousePos = cursorPos;
		mousePos += { dimouse.lX, dimouse.lY };
		if ((mouseWheelVal = dimouse.lZ / 120) != 0)
			mouseWheelInput(mouseWheelVal, mousePos.x, mousePos.y);
		if (mousePos != mousePosPrevious)
			mouseMove(mousePos.x, mousePos.y);
		for (int i = 0; i < 255; i++) {
			int state = KEY_DOWN(i);
			if (keyState[i] != state) {
				switch (keyState[i])
				{
				case Up:
					keyState[i] = OnPress;
					break;
				case Down:
					keyState[i] = OnRelease;
					break;
				case OnPress:
					keyState[i] = state == 0 ? OnRelease : Down;
					break;
				case OnRelease:
					keyState[i] = state == 0 ? Up : OnPress;
					break;
				default:
					break;
				}
				keyboardInput(i, keyState[i]);
			}
		}
	}
	hideCursor(cursorHidden);
}

void Input::end()
{
	_mouseDevice->Unacquire();
	_mouseDevice->Release();
	_directInput->Release();
}
