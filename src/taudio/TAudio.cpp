#include "TAudio.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <vector>

TAudioFile::TAudioFile()
	: m_flac(nullptr),
	  m_wav(nullptr),
	  m_ogg(nullptr),
	  m_type(Invalid)
{}

TAudioFile::TAudioFile(const std::string& fileName, bool write, uint32_t sampleRate) {
	m_fileName = fileName;
	std::string ext = fileName.substr(fileName.find_last_of('.'));
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (!write) {
		std::string id4(4, 0);
		std::string id(3, 0);
		std::ifstream fp(fileName);
		if (fp.good()) {
			fp.read(id4.data(), 4);
			fp.close();
			id = id4.substr(0, 3);
		}

		if (id4 == "RIFF" && (ext == ".wav" || ext == ".wave")) {
			m_wav = drwav_open_file(fileName.c_str());
			m_channels = m_wav->channels;
			m_sampleRate = m_wav->sampleRate;
			m_frames = m_wav->totalPCMFrameCount;
			m_type = Wav;
		} else if (id4 == "fLaC" && ext == ".flac") {
			m_flac = drflac_open_file(fileName.c_str());
			m_channels = m_flac->channels;
			m_sampleRate = m_flac->sampleRate;
			m_frames = m_flac->totalPCMFrameCount;
			m_type = Flac;
		} else if (id == "Ogg" && ext == ".ogg") {
			int error;
			m_ogg = stb_vorbis_open_filename(fileName.c_str(), &error, nullptr);
			stb_vorbis_info info = stb_vorbis_get_info(m_ogg);
			m_channels = info.channels;
			m_sampleRate = info.sample_rate;
			m_frames = stb_vorbis_stream_length_in_samples(m_ogg);
			m_type = Ogg;
		} else {
			m_type = Invalid;
		}
	} else {
		if (ext == ".wav" || ext == ".wave") {
			drwav_data_format fmt;
			fmt.container = drwav_container_riff;
			fmt.format = DR_WAVE_FORMAT_PCM;
			fmt.channels = 1;
			fmt.sampleRate = sampleRate;
			fmt.bitsPerSample = 16;
			m_wav = drwav_open_file_write(fileName.c_str(), &fmt);
			m_type = Wav;
		}
	}
}

TAudioFile::~TAudioFile() {
	switch (m_type) {
		default: break;
		case Wav: drwav_close(m_wav); break;
		case Flac: drflac_close(m_flac); break;
		case Ogg: stb_vorbis_close(m_ogg); break;
	}
}

uint64_t TAudioFile::readf(float* outdata, uint32_t outsize) {
	switch (m_type) {
		default: return 0;
		case Wav: return drwav_read_pcm_frames_f32(m_wav, outsize, outdata);
		case Flac: return drflac_read_pcm_frames_f32(m_flac, outsize, outdata);
		case Ogg: return stb_vorbis_get_samples_float_interleaved(m_ogg, m_channels, outdata, outsize);
	}
}

uint64_t TAudioFile::writef(float* indata, uint32_t insize) {
	if (m_type != Wav) return 0;
	std::vector<int16_t> samples; samples.resize(insize);
	for (uint32_t i = 0; i < insize; i++) {
		samples[i] = static_cast<int16_t>(indata[i] * 32767.0f);
	}
	return drwav_write_pcm_frames(m_wav, samples.size(), (void*) samples.data());
}
