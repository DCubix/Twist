#ifndef TWEN_ARP_NODE_H
#define TWEN_ARP_NODE_H

#include <iostream>
#include "../Node.h"

class ArpNode : public Node {
	TWEN_NODE(ArpNode, "Arp")
public:
	enum Chord {
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

	enum Direction {
		Up = 0,
		Down,
		UpDown,
		Random,
		DirectionCount
	};

	ArpNode(u32 note, u32 chord, u32 dir, float oct) : Node() {
		addInput("Time");
		addOutput("Gate");
		addOutput("Nt");

		addParam("Type", { "maj", "min", "sus2", "sus4", "maj7", "min7", "9th", "oct" }, chord, true, 50);
		addParam("Note", { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, note, false, 50);
		addParam("Oct", 0.0f, 5.0f, oct, 1.0f, NodeParam::IntRange, true, 50);
		addParam("Dir", { "Up", "Down", "Up+Down", "Random" }, dir, false, 50);
	}

	int index(float ntime, int n) {
		int rn = (int(Utils::lerp(0, n, ntime)) % n);
		if (prevRN != rn) {
			gate = false;
			prevRN = rn;
		} else {
			gate = true;
		}

		Direction direction = (Direction) paramOption(3);
		switch (direction) {
			case Up: return rn;
			case Down: return n - 1 - rn;
			case UpDown: {
				float cy = Utils::cyclef(ntime * 2.0f);
				float nm = Utils::remap(cy * (n * 2), 0, n*2, 0, n);
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
		float ntime = in(0);

		Chord chordType = (Chord) paramOption(0);
		Note note = (Note) paramOption(1);
		Direction direction = (Direction) paramOption(3);

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

		if (gate) {
			outs(0).set(1.0f);
		} else {
			outs(0).set(0.0f);
		}
		outs(1).set(nt + (12 * param(2)));
	}

private:
	bool gate = false;
	int prevNt, prevRN = -1;
	int prevN = 0, randN = 0;
};

#endif // TWEN_ARP_NODE_H
