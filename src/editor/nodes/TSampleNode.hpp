#ifndef T_SAMPLE_NODE_H
#define T_SAMPLE_NODE_H

#include <iostream>

#include "TNode.h"
#include "../../TGen.h"

#include "../tinyfiledialogs.h"
#include "sndfile.hh"

class TSampleNode : public TNode {
public:
	TSampleNode() : TNode("Sample", 0, 0) {
		addInput("Gate");
		addInput("Amp");
		addOutput("Out");

		sample.resize(1);
	}

	void gui() {
		if (ImGui::Button("Load##loadSmp")) {
			const static char* F[] = { 
				"*.wav\0",
				"*.aif\0",
				"*.flac\0",
				"*.ogg\0"
			};
			const char* filePath = tinyfd_openFileDialog(
				"Load Sample",
				"",
				4,
				F,
				"Audio Files (*.wav; *.aif; *.flac; *.ogg)",
				0
			);
			if (filePath) {
				SndfileHandle snd = SndfileHandle(filePath);
				int maxSecs = 10;
				if (snd.samplerate() > 44100) {
					maxSecs = 5;
				}
				if (snd.frames() < snd.samplerate() * maxSecs) {
					sampleRate = snd.samplerate();
					duration = float(double(snd.frames()) / sampleRate);

					sample.resize(snd.frames());

					std::vector<float> samplesRaw;
					samplesRaw.resize(snd.frames() * snd.channels());

					snd.readf(samplesRaw.data(), samplesRaw.size());

					sample.resize(snd.frames());
					for (int i = 0; i < snd.frames(); i++) {
						sample[i] = samplesRaw[i];
					}

					currTime = 0;
				} else {
					tinyfd_messageBox("Error", "This sample is way too big.", "ok", "error", 1);
				}
			}
		}
		ImGui::AudioView("##audio_view0", 120, sample.data(), sample.size(), int(currTime * sampleRate));
	}

	void solve() {
		bool gate = getInputOr(0, 0.0f) > 0.0f ? true : false;

		if (gate) {
			state = TADSR::Attack;
		} else if (state != TADSR::Idle) {
			state = TADSR::Decay;
		}

		switch (state) {
			case TADSR::Idle: break;
			case TADSR::Attack: {
				if (currTime < duration) {
					int sp = int(currTime * sampleRate);
					setOutput(0, sample[sp]);
				}

				currTime += 1.0f / sampleRate;
				if (currTime >= duration) {
					state = TADSR::Decay;
				}
			} break;
			case TADSR::Decay: {
				currTime = 0;
				state = TADSR::Idle;
			} break;
			default: break;
		}

	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sample"] = sample;
		json["sampleRate"] = sampleRate;
		json["duration"] = duration;
	}

	static std::string type() { return "Sample"; }

	float sampleRate, duration = 0, currTime = 0;

	std::vector<float> sample;

	bool playing = false;
	int state = TADSR::Idle;
};

#endif // T_SAMPLE_NODE_H