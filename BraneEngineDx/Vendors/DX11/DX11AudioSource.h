#pragma once

#include "../../Core/IAudioSource.h"

#ifdef VENDOR_USE_DX11

#ifndef _DX11AUDIOSOURCE_H_
#define _DX11AUDIOSOURCE_H_

#include "DX11.h"

class DX11AudioSource : public IAudioSource
{
public:
	DX11Context& dxContext;
	IXAudio2SourceVoice* sourceVoice = NULL;

	DX11AudioSource(DX11Context& context);
	~DX11AudioSource();

	virtual bool load(const string& path);
};

#endif // !_DX11AUDIOSOURCE_H_

#endif // VENDOR_USE_DX11
