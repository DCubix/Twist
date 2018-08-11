#ifndef T_SEQUENCER_NODE_H
#define T_SEQUENCER_NODE_H

#include "TNode.h"

#define SEQUENCER_SIZE 8
class TSequencerNode : public TNode {
public:
	TSequencerNode(float sampleRate, float bpm, float swing)
		: TNode("Sequencer", 410, 150),
			gate(false), noteIndex(0),
			stime(0.0f), sampleRate(sampleRate),
			bpm(120), out(0.0f), swing(swing)
	{
		addInput("Mod");
		addInput("Reset");
		addOutput("Freq");
		addOutput("Gate");
		std::memset(notes, 0, sizeof(Notes) * SEQUENCER_SIZE);
		std::memset(octs, 0, sizeof(int) * SEQUENCER_SIZE);
	}

	void gui() {
		static const char* NOTES[] = {
			"--\0",
			"C\0",
			"C#\0",
			"D\0",
			"D#\0",
			"E\0",
			"F\0",
			"F#\0",
			"G\0",
			"G#\0",
			"A\0",
			"A#\0",
			"B\0",
			0
		};

		ImGui::BeginHorizontal(this);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				bool pushed = false;
				if (i == noteIndex) {
					pushed = true;
					ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(120, 250, 120, 255));
				}
				if (ImGui::Button(" ", ImVec2(32, 0))) {
					noteIndex = i;
				}
				if (pushed) {
					ImGui::PopStyleColor();
				}
			}
		ImGui::EndHorizontal();

		ImGui::BeginHorizontal(this);
			ImGui::PushItemWidth(32);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				char id[4];
				id[0] = '#'; id[1] = '#'; id[2] = (i+1); id[3] = 0;
				ImGui::Combo(id, (int*) &notes[i], NOTES, 13);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();
		
		ImGui::BeginHorizontal(this);
			ImGui::PushItemWidth(32);
			for (int i = 0; i < SEQUENCER_SIZE; i++) {
				char id[5];
				id[0] = '#'; id[1] = '#'; id[2] = (i+1); id[3] = 'o'; id[4] = 0;
				ImGui::DragInt(id, &octs[i], 0.25f, -8, 8);
			}
			ImGui::PopItemWidth();
		ImGui::EndHorizontal();

		ImGui::DragFloat("Swing##sw", &swing, 0.1f, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat("BPM##bpm", &bpm, 0.5f, 40.0f, 256.0f);
	}

	void solve() {
		if (getInputOr(1, 0.0f) > 0.0f) {
			noteIndex = 0;
		}
		const float step = (1.0f / sampleRate) * 4;

		float delay = (60000.0f / bpm) / 1000.0f;
		stime += step;

		float sw = swing * delay;

		int ni = noteIndex;
		Notes note = notes[ni % SEQUENCER_SIZE];

		float delaySw = ni % 2 == 0 ? delay - sw * 0.5f : delay + sw * 0.5f;
		if (stime >= delaySw) {
			out = NOTE(note) * std::pow(2, octs[ni % SEQUENCER_SIZE]);
			noteIndex++;
			noteIndex = noteIndex % SEQUENCER_SIZE;
			stime = 0.0f;
			if (note != Notes::Silence) {
				gate = false;
			}
		} else {
			gate = true;
		}

		setOutput(0, getInput(0) + out);
		setOutput(1, gate ? 1.0f : 0.0f);
	}

	virtual void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["sampleRate"] = sampleRate;
		json["bpm"] = bpm;
		json["swing"] = swing;

		std::array<int, SEQUENCER_SIZE> notes_;
		std::array<int, SEQUENCER_SIZE> octs_;
		std::copy(std::begin(octs), std::end(octs), std::begin(octs_));
		for (int i = 0; i < SEQUENCER_SIZE; i++) notes_[i] = (int) notes[i];
		json["notes"] = notes_;
		json["octs"] = octs_;
	}

	int noteIndex;
	bool gate;
	float bpm, stime, sampleRate, out, swing = 0.0f;
	Notes notes[SEQUENCER_SIZE];
	int octs[SEQUENCER_SIZE];

	static std::string type() { return "Sequencer"; }
};

#endif // T_SEQUENCER_NODE_H