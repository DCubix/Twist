#ifndef T_USER_INTERFACES_H
#define T_USER_INTERFACES_H

#include "twen/Node.h"
#include "twen/NodeGraph.h"
#include "TNodeInterfaces.h"

#include "twen/nodes/ADSRNode.hpp"
#include "twen/nodes/ArpNode.hpp"
#include "twen/nodes/ChorusNode.hpp"
#include "twen/nodes/DelayLineNode.hpp"
#include "twen/nodes/FilterNode.hpp"
#include "twen/nodes/MathNode.hpp"
#include "twen/nodes/MixNode.hpp"
#include "twen/nodes/NoteNode.hpp"
#include "twen/nodes/OscillatorNode.hpp"
#include "twen/nodes/OutNode.hpp"
#include "twen/nodes/RemapNode.hpp"
#include "twen/nodes/StorageNodes.hpp"
#include "twen/nodes/ValueNode.hpp"

namespace Twist {
	inline void init() {
		NodeInterfaces::registerGUI<ValueNode>(TWIST_NODE_GUI {
			ValueNode *n = dynamic_cast<ValueNode*>(node);
			ImGui::InputFloat("Value", &n->value, 0.01f, 0.1f);
		});

		NodeInterfaces::registerGUI<ADSRNode>(TWIST_NODE_GUI {
			ADSRNode *n = dynamic_cast<ADSRNode*>(node);

			ImGui::Knob("A", &n->a, 0.0f, 20.0f);
			ImGui::SameLine();
			ImGui::Knob("D", &n->d, 0.0f, 20.0f);
			ImGui::SameLine();
			ImGui::Knob("S", &n->s, 0.0f, 1.0f);
			ImGui::SameLine();
			ImGui::Knob("R", &n->r, 0.0f, 20.0f);
		});

		NodeInterfaces::registerGUI<ArpNode>(TWIST_NODE_GUI {
			ArpNode *n = dynamic_cast<ArpNode*>(node);

			static const char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
			static const char* CHORDS[] = { "maj", "min", "sus2", "sus4", "maj7", "min7", "9th", "oct" };
			static const char* DIRS[] = { "Up", "Down", "Up+Down", "Random" };

			ImGui::PushItemWidth(60);
			ImGui::Combo("Note", (int*)&n->note, NOTES, 12); ImGui::SameLine();
			ImGui::DragInt("Oct.", (int*)&n->oct, 0.01f, 0, 8);
			ImGui::Combo("Chord", (int*)&n->chord, CHORDS, 8); ImGui::SameLine();
			ImGui::Combo("Dir.", (int*)&n->direction, DIRS, 4);
		});

		NodeInterfaces::registerGUI<ChorusNode>(TWIST_NODE_GUI {
			ChorusNode *n = dynamic_cast<ChorusNode*>(node);
			ImGui::DragFloat("Rate", &n->rate, 0.1f, 0.0f, 6.0f);
			ImGui::DragFloat("Depth", &n->depth, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Delay", &n->delay, 0.1f, 0.0f, 1.0f);
		});

		NodeInterfaces::registerGUI<DelayLineNode>(TWIST_NODE_GUI {
			DelayLineNode *n = dynamic_cast<DelayLineNode*>(node);
			ImGui::DragFloat("Feedback", &n->feedBack, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Delay", &n->delay, 0.1f, 0.0f, 10.0f);
		});

		NodeInterfaces::registerGUI<FilterNode>(TWIST_NODE_GUI {
			FilterNode *n = dynamic_cast<FilterNode*>(node);

			static const char* FILTERS[] = { "Low-Pass", "High-Pass" };

			ImGui::Combo("Filter", (int*)&n->filter, FILTERS, 2);
			if (n->connected(1)) {
				ImGui::LabelText("CutOff: ", "%.2f", n->get(1));
			} else {
				ImGui::DragFloat("CutOff", &n->cutOff, 1.0f, 20.0f, 20000.0f);
			}
		});

		NodeInterfaces::registerGUI<MathNode>(TWIST_NODE_GUI {
			MathNode *n = dynamic_cast<MathNode*>(node);

			static const char* OPS[] = { "Add", "Subtract", "Multiply", "Negate", "Average" };

			ImGui::Combo("Op", (int*)&n->filter, OPS, 5);
			if (n->connected(0)) {
				ImGui::LabelText("A: ", "%.2f", n->get(0));
			} else {
				ImGui::InputFloat("A", &n->a, 0.01f, 0.1f);
			}

			if (n->connected(1)) {
				ImGui::LabelText("B: ", "%.2f", n->get(1));
			} else {
				ImGui::InputFloat("B", &n->b, 0.01f, 0.1f);
			}
		});

		NodeInterfaces::registerGUI<MixNode>(TWIST_NODE_GUI {
			MixNode *n = dynamic_cast<MixNode*>(node);

			if (n->connected(0)) {
				ImGui::LabelText("Fac.: ", "%.2f", n->get(0));
			} else {
				ImGui::DragFloat("Fac.", &n->factor, 0.01f, 0.0f, 1.0f);
			}
		});

		NodeInterfaces::registerGUI<NoteNode>(TWIST_NODE_GUI {
			NoteNode *n = dynamic_cast<NoteNode*>(node);

			static const char* NOTES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

			ImGui::PushItemWidth(60);
			ImGui::Combo("Note", (int*)&n->note, NOTES, 12); ImGui::SameLine();
			ImGui::DragInt("Oct.", (int*)&n->oct, 0.01f, 0, 8);
		});

		NodeInterfaces::registerGUI<OscillatorNode>(TWIST_NODE_GUI {
			OscillatorNode *n = dynamic_cast<OscillatorNode*>(node);

			static const char* WF[] = { "Sine", "Square", "Saw", "Triangle", "Noise" };

			ImGui::Combo("WaveForm", (int*)&n->waveForm, WF, 5);
			ImGui::DragFloat("Freq.", &n->frequency, 0.1f, 30.0f, 15000.0f);
		});

		NodeInterfaces::registerGUI<ReaderNode>(TWIST_NODE_GUI {
			ReaderNode *n = dynamic_cast<ReaderNode*>(node);
			ImGui::DragInt("Slot", (int*)&n->slot, 0.01f, 0, TWEN_GLOBAL_STORAGE_SIZE);
		});

		NodeInterfaces::registerGUI<WriterNode>(TWIST_NODE_GUI {
			WriterNode *n = dynamic_cast<WriterNode*>(node);
			ImGui::DragInt("Slot", (int*)&n->slot, 0.01f, 0, TWEN_GLOBAL_STORAGE_SIZE);
		});

		NodeInterfaces::registerGUI<RemapNode>(TWIST_NODE_GUI {
			RemapNode *n = dynamic_cast<RemapNode*>(node);

			float from[2] = {0,1}, to[2] = {0, 1};
			if (ImGui::InputFloat2("From", from)) {
				n->fromMin = from[0];
				n->fromMax = from[1];
			}
			if (ImGui::InputFloat2("To", to)) {
				n->toMin = to[0];
				n->toMax = to[1];
			}
		});

		NodeInterfaces::registerGUI<OutNode>(TWIST_NODE_GUI {});
	}
}

#endif // T_USER_INTERFACES_H
