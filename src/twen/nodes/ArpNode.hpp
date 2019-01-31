#ifndef TWEN_ARP_NODE_H
#define TWEN_ARP_NODE_H

#include <iostream>
#include "../NodeGraph.h"

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

	ArpNode(Note note, Chord chord, Direction dir, u32 oct)
		: Node(), note(note), chord(chord), direction(dir), oct(oct)
	{}

	int index(float ntime, int n) {
		int rn = (int(Utils::lerp(0, n, ntime)) % n);

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

	float sample(NodeGraph *graph) override {
		float ntime = graph->time();

		int noteIn = note;

#define INDEX(n) index(ntime, n)
		int nt = 0;
		switch (chord) {
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

		u32 outNote = nt + (12 * oct);
		return Utils::noteFrequency(outNote);
	}

	Note note;
	Chord chord;
	Direction direction;
	u32 oct;

private:
	int prevNt, prevRN = -1;
	int prevN = 0, randN = 0;
};

#endif // TWEN_ARP_NODE_H
