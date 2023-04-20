#include "AudioWave.h"
#include "../Utility/IOUtility.h"

size_t AudioWave::calSize() const
{
	return sizeof(Header) + sizeof(FormatChunk) + sizeof(InstrumentChunk) + sizeof(SampleChunk) +
		sizeof(Chunk) * 3 + sizeof(char) * data.size() + sizeof(SampleLoop) * loops.size();
}

bool AudioWave::loadFromStream(istream& is)
{
	Header header;
	readStream(is, header);

	uint32_t fileSize = header.size + 8;

	while (is.tellg() < fileSize) {
		Chunk chunk;
		readStream(is, chunk);
		swapBytes(chunk.id, sizeof(chunk.id));

		size_t targetPos = is.tellg() + (long long)chunk.size;

		switch (*(int*)chunk.id) {
		case 'fmt ':
			readStream(is, format);
			break;
		case 'data':
			data.resize(chunk.size);
			readStream(is, data.data(), chunk.size);
			break;
		case 'inst':
			readStream(is, instrument);
			break;
		case 'smpl':
			readStream(is, sample);
			loops.resize(sample.sampleLoops);
			readArrayStream(is, loops.data(), sample.sampleLoops);
			break;
		default:
			break;
		}
		is.seekg(targetPos, ios::beg);
		alignStream(is, 4);
	}

	return true;
}

bool AudioWave::writeToStream(ostream& os)
{
	Header header;
	memcpy(header.riff, "RIFF", sizeof(4));
	memcpy(header.wave, "WAVE", sizeof(4));
	header.size = calSize() - 8;
	writeStream(os, header);

	Chunk chunk;

	memcpy(chunk.id, "fmt ", sizeof(4));
	chunk.size = sizeof(FormatChunk);
	writeStream(os, chunk);
	writeStream(os, format);

	memcpy(chunk.id, "data", sizeof(4));
	chunk.size = sizeof(char) * data.size();
	writeStream(os, chunk);
	writeArrayStream(os, data.data(), data.size());

	memcpy(chunk.id, "inst", sizeof(4));
	chunk.size = sizeof(InstrumentChunk);
	writeStream(os, chunk);
	writeStream(os, instrument);

	memcpy(chunk.id, "smpl", sizeof(4));
	sample.sampleLoops = loops.size();
	sample.samplerDataBytes = 0;
	chunk.size = sizeof(SampleChunk) + sizeof(SampleLoop) * sample.sampleLoops;
	writeStream(os, chunk);
	writeStream(os, sample);
	writeArrayStream(os, loops.data(), loops.size());
	return true;
}

void AudioWave::reset()
{
	memset(&format, 0, sizeof(format));
	memset(&instrument, 0, sizeof(instrument));
	memset(&sample, 0, sizeof(sample));
	data.clear();
	loops.clear();
}
