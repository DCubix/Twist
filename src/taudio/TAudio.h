#ifndef TAUDIO_H
#define TAUDIO_H

#include "intern/dr_flac.h"
#include "intern/dr_wav.h"
#include "intern/stb_vorbis.h"

#include <string>
#include <cstdint>

class TAudioFile {
public:
	enum FileType {
		Invalid = 0,
		Wav,
		Ogg,
		Flac
	};

	TAudioFile();
	TAudioFile(const std::string& fileName, bool write = false, uint32_t sampleRate = 44100);
	~TAudioFile();

	uint64_t readf(float* outdata, uint32_t outsize);
	uint64_t writef(float* indata, uint32_t insize);

	uint64_t frames() const { return m_frames; }
	uint32_t sampleRate() const { return m_sampleRate; }
	uint32_t channels() const { return m_channels; }

private:
	drflac *m_flac;
	drwav *m_wav;
	stb_vorbis *m_ogg;
	FileType m_type;

	// Info
	uint64_t m_frames;
	uint32_t m_sampleRate;
	uint32_t m_channels;

	std::string m_fileName;
};

#endif // TAUDIO_H
