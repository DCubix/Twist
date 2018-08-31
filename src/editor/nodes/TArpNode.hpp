#ifndef T_ARP_NODE_H
#define T_ARP_NODE_H

#include <iostream>
#include "TNode.h"
#include "../../TGen.h"

class TArpNode : public TNode {
public:
	enum TChord {
		Major = 0,
		Minor,
		Sus2,
		Sus4,
		Major7,
		Minor7,
		Nineth,
		Octave,
		ChordTypeCount
	};

	enum TDirection {
		Up = 0,
		Down,
		UpDown,
		Random,
		DirectionCount
	};

	TArpNode(Notes note, int oct, TChord ct, TDirection dir)
		: TNode("Arp", 0, 0),
		note(note), chordType(ct), direction(dir), oct(oct)
	{
		addInput("Time");
		addOutput("Freq");
		addOutput("Gate");
		addOutput("Nt");
	}

	void gui() {
		static const char* CHORDT[] = {
			"maj\0",
			"min\0",
			"sus2\0",
			"sus4\0",
			"maj7\0",
			"min7\0",
			"9th\0",
			"Oct\0",
			0
		};

		static const char* DIRT[] = {
			"Up\0",
			"Down\0",
			"Up+Down\0",
			"Random\0",
			0
		};

		static const char* NOTES[] = {
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

		ImGui::PushItemWidth(50);
		ImGui::Combo("Chd.", (int*)&chordType, CHORDT, ChordTypeCount);
		ImGui::SameLine();
		ImGui::Combo("Dir.", (int*)&direction, DIRT, DirectionCount);

		ImGui::Combo("Note", (int*)&note, NOTES, 12, -1);
		ImGui::SameLine();
		ImGui::DragInt("Oct", &oct, 0.1f, -5, 5);
		ImGui::PopItemWidth();
	}

	int index(float ntime, int n) {
		int rn = (int(tmath::lerp(0, n, ntime)) % n);
		if (prevRN != rn) {
			gate = false;
			prevRN = rn;
		} else {
			gate = true;
		}

		switch (direction) {
			case Up: return rn;
			case Down: return n - 1 - rn;
			case UpDown: {
				float cy = tmath::cyclef(ntime * 2.0f);
				float nm = tmath::remap(cy * (n * 2), 0, n*2, 0, n);
				return int(nm);
			};
			case Random: {
				if (prevN != rn) {
					randN = rand() % n;
					prevN = rn;
				}
				return randN;
			};
			default: return rn;
		}
	}

	void solve() {
		float ntime = getInputOr(0, 0);
		int noteIn = note;

#define INDEX(n) index(ntime, n)
		int nt = 0;
		switch (chordType) {
			case Major: {
				const int n[] = { 0, 4, 7 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Minor: {
				const int n[] = { 0, 3, 7 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Sus2: {
				const int n[] = { 0, 2, 7 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Sus4: {
				const int n[] = { 0, 5, 7 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Major7: {
				const int n[] = { 0, 4, 7, 11 };
				nt = n[INDEX(4)] + int(noteIn);
			} break;
			case Minor7: {
				const int n[] = { 0, 3, 7, 10 };
				nt = n[INDEX(4)] + int(noteIn);
			} break;
			case Nineth: {
				const int n[] = { 2, 4, 7 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Octave: {
				const int n[] = { 0, 12 };
				nt = n[INDEX(2)] + int(noteIn);
			} break;
			default: break;
		}

		setOutput(0, tgen::note(nt, oct));
		setOutput(1, gate ? 1 : 0);
		setOutput(2, nt*oct);
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["note"] = (int) note;
		json["oct"] = oct;
		json["direction"] = (int) direction;
		json["chordType"] = (int) chordType;
	}

	Notes note;
	TChord chordType;
	TDirection direction;
	int oct, prevNt;

	bool gate = false;
	int prevRN = -1;

	int prevN = 0, randN = 0;

	static std::string type() { return "Arp"; }
};

#endif // T_ARP_NODE_H