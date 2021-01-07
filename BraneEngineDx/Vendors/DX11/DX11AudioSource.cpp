#include "DX11AudioSource.h"

DX11AudioSource::DX11AudioSource(DX11Context& context) : dxContext(context)
{
}

DX11AudioSource::~DX11AudioSource()
{
}

bool DX11AudioSource::load(const string& path)
{
    WAVEFORMATEX wav;
    wav;
    dxContext.audioDevice->CreateSourceVoice(&sourceVoice, &wav);
    sourceVoice->
    return false;
}
