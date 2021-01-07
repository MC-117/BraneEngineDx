#pragma once
#ifndef _IAUDIOSOURCE_H_
#define _IAUDIOSOURCE_H_

#include "Unit.h"

class IAudioSource
{
public:
	virtual bool load(const string& path) = 0;
};

#endif // !_IAUDIOSOURCE_H_
