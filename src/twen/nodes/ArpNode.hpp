#ifndef TWEN_ARP_NODE_H
#define TWEN_ARP_NODE_H

#include <iostream>
#include <cmath>
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
		Sharp5,
		Dim,
		Sixth,
		ChordTypeCount
	};

	enum Direction {
		Up = 0,
		Down,
		UpDown,
		Random,
		DirectionCount
	};

	inline ArpNode(Note note, Chord chord, Direction dir, u32 oct)
		: Node(), note(note), chord(chord), direction(dir), oct(oct)
	{
		addInput("In");
	}

	inline int index(int rn, int n) {
		if (prevRN != rn) {
			gate = false;
			prevRN = rn;
		} else {
			gate = true;
		}

		int i = rn % n;

		switch (direction) {
			case Up: return i;
			case Down: return n - 1 - i;
			case UpDown: {
				if (n < 2) return i;
				int j = rn % ((n - 1) * 2);
				if (j <= n - 1) return j;
				else return (n - 1) - (j - (n - 1));
			};
			case Random: {
				if (prevN != rn) {
					randN = rand() % n;
					prevN = rn;
				}
				return randN;
			};
			default: return i;
		}
	}

	inline Value sample(NodeGraph *graph) override {
		int noteIn = note;
		bool baseGate = true;
		if (connected(0)) {
			noteIn = note + int(in(0).value());
			baseGate = in(0).gate();
		}

#define INDEX(n) index(graph->index(), n)
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
			case Sharp5: {
				const int n[] = { 0, 4, 8 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Dim: {
				const int n[] = { 0, 3, 6 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			case Sixth: {
				const int n[] = { 0, 3, 9 };
				nt = n[INDEX(3)] + int(noteIn);
			} break;
			default: break;
		}

		u32 outNote = u32(nt) + (12 * oct);
		float value = Utils::noteFrequency(outNote);
		return Value(value, 1.0f, gate && baseGate);
	}

	inline void save(JSON& json) override {
		Node::save(json);
		json["note"] = int(note);
		json["chord"] = int(chord);
		json["direction"] = int(direction);
		json["oct"] = oct;
	}

	inline void load(JSON json) override {
		Node::load(json);
		note = Note(json["note"].get<int>());
		chord = Chord(json["chord"].get<int>());
		direction = Direction(json["direction"].get<int>());
		oct = json["oct"].get<u32>();
	}

	Note note;
	Chord chord;
	Direction direction;
	u32 oct;

private:
	bool gate = false;
	int prevNt, prevRN = -1;
	int prevN = 0, randN = 0;
};

#endif // TWEN_ARP_NODE_H
