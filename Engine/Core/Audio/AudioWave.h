#pragma once
#include "../Config.h"

#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

struct AudioWave
{
    struct Header
    {
        char riff[4];
        int32_t size;
        char wave[4];
    };

    struct Chunk
    {
        char id[4];
        int32_t size;
    };

    struct FormatChunk
    {
        int16_t format;
        int16_t channels;
        int32_t sampleRate;
        int32_t byteRate;
        int16_t blockAlign;
        int16_t bitsPerSample;
    };

    struct SampleChunk
    {
        int32_t manufacturer;
        int32_t product;
        int32_t samplePeriod;
        int32_t midiUnityNote;
        int32_t midiPitchFraction;
        int32_t smpteFormat;
        int32_t smpteOffset;
        int32_t sampleLoops;
        int32_t samplerDataBytes;
    };

    struct SampleLoop
    {
        int32_t id;
        int32_t type;
        int32_t start;
        int32_t end;
        int32_t fraction;
        int32_t playCount;
    };

    struct InstrumentChunk
    {
        uint8_t rootNote;
        int8_t fineTune;
        int8_t gain;
        uint8_t lowNote;
        uint8_t highNote;
        uint8_t lowVelocity;
        uint8_t highVelocity;
        uint8_t pad;
    };

    FormatChunk format;
    InstrumentChunk instrument;
    SampleChunk sample;
    vector<char> data;
    vector<SampleLoop> loops;

    AudioWave() = default;
    size_t calSize() const;
    bool loadFromStream(istream& is);
    bool writeToStream(ostream& os);
    void reset();
};