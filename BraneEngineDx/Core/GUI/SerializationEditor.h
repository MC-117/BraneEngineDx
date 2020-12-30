#pragma once
#ifndef _SERIALIZATIONEDITOR_H_
#define _SERIALIZATIONEDITOR_H_

#include "UIWindow.h"

class SerializationEditor : public UIWindow
{
public:
	SerializationInfo* serializationInfo = NULL;

	SerializationEditor(string name = "SerializationEditor", bool defaultShow = false);

	void setSerializtionInfo(SerializationInfo& info);

	virtual void onRenderWindow(GUIRenderInfo& info);

	static void showSerializationInfo(GUI& gui, SerializationInfo& info);
protected:
	void drawInfo(SerializationInfo* info);
};

#endif // !_SERIALIZATIONEDITOR_H_
