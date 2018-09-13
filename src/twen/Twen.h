#ifndef TWEN_H
#define TWEN_H

#include "Node.h"
#include "NodeGraph.h"
#include "NodeRegistry.h"

#include "nodes/ADSRNode.hpp"
#include "nodes/ArpNode.hpp"
#include "nodes/ChorusNode.hpp"
#include "nodes/DelayLineNode.hpp"
#include "nodes/FilterNode.hpp"
#include "nodes/MathNode.hpp"
#include "nodes/MixNode.hpp"
#include "nodes/NoteNode.hpp"
#include "nodes/OscillatorNode.hpp"
#include "nodes/OutNode.hpp"
#include "nodes/RemapNode.hpp"
#include "nodes/ReverbNode.hpp"
#include "nodes/SampleNode.hpp"
#include "nodes/StorageNodes.hpp"
#include "nodes/SequencerNode.hpp"
#include "nodes/TimerNode.hpp"
#include "nodes/ValueNode.hpp"
#include "nodes/CompressorNode.hpp"

namespace Twen {
	inline void init() {
#define GET(type, v, d) (json[v].is_null() ? d : json[v].get<type>())

		NodeBuilder::registerType<ADSRNode>("Generators", TWEN_NODE_FAC {
			return new ADSRNode(
				GET(float, "sampleRate", 44100.0f),
				GET(float, "a", 0.0f),
				GET(float, "d", 0.0f),
				GET(float, "s", 1.0f),
				GET(float, "r", 0.0f)
			);
		});

		NodeBuilder::registerType<ArpNode>("Generators", TWEN_NODE_FAC {
			return new ArpNode(
				GET(int, "note", 0),
				(ArpNode::Chord) GET(int, "chord", 0),
				(ArpNode::Direction) GET(int, "dir", 0),
				GET(float, "oct", 0)
			);
		});

		NodeBuilder::registerType<ChorusNode>("Effects", TWEN_NODE_FAC {
			return new ChorusNode(
				GET(float, "sampleRate", 44100.0f),
				GET(float, "chorusRate", 0.0f),
				GET(float, "chorusDepth", 0.0f),
				GET(float, "delayTime", 0.0f)
			);
		});

		NodeBuilder::registerType<DelayLineNode>("Effects", TWEN_NODE_FAC {
			return new DelayLineNode(
				GET(float, "sampleRate", 44100.0f),
				GET(float, "feedback", 0.0f),
				GET(float, "delay", 100.0f)
			);
		});

		NodeBuilder::registerType<FilterNode>("Effects", TWEN_NODE_FAC {
			return new FilterNode(
				GET(float, "sampleRate", 44100.0f),
				GET(float, "cut", 0.0f),
				(FilterNode::Filter) GET(int, "filter", 0)
			);
		});

		NodeBuilder::registerType<MathNode>("General", TWEN_NODE_FAC {
			return new MathNode(
				(MathNode::MathOp) GET(int, "op", 0),
				GET(float, "a", 0),
				GET(float, "b", 0)
			);
		});

		NodeBuilder::registerType<MixNode>("General", TWEN_NODE_FAC {
			return new MixNode(GET(float, "fac", 0.5f));
		});

		NodeBuilder::registerType<NoteNode>("General", TWEN_NODE_FAC {
			return new NoteNode(
				(Note) GET(int, "note", 0),
				GET(float, "oct", 0)
			);
		});

		NodeBuilder::registerType<FreqNode>("General", TWEN_NODE_FAC {
			return new FreqNode();
		});

		NodeBuilder::registerType<OscillatorNode>("Generators", TWEN_NODE_FAC {
			return new OscillatorNode(
				GET(float, "sampleRate", 44100.0f),
				(Oscillator::WaveForm) GET(int, "wf", 0),
				GET(float, "freq", 220.0f),
				GET(float, "amp", 1.0f)
			);
		});

		NodeBuilder::registerType<OutNode>("General", TWEN_NODE_FAC {
			return new OutNode();
		});

		NodeBuilder::registerType<RemapNode>("General", TWEN_NODE_FAC {
			return new RemapNode(
				GET(float, "omin", 0.0f),
				GET(float, "omax", 1.0f),
				GET(float, "nmin", 0.0f),
				GET(float, "nmax", 1.0f)
			);
		});

		NodeBuilder::registerType<ReverbNode>("Effects", TWEN_NODE_FAC {
			return new ReverbNode(
				GET(float, "sampleRate", 44100.0f),
				GET(int, "preset", SF_REVERB_PRESET_DEFAULT)
			);
		});

		NodeBuilder::registerType<SampleNode>("Generators", TWEN_NODE_FAC {
			return new SampleNode(
				GET(int, "sampleID", 0)
			);
		});

		NodeBuilder::registerType<ReaderNode>("General", TWEN_NODE_FAC {
			return new ReaderNode(GET(int, "idx", 0));
		});

		NodeBuilder::registerType<WriterNode>("General", TWEN_NODE_FAC {
			return new WriterNode(GET(int, "idx", 0));
		});

		NodeBuilder::registerType<SequencerNode>("Generators", TWEN_NODE_FAC {
			SequencerNode* seq = new SequencerNode(GET(int, "key", 0));
			if (json["notes"].is_array()) {
				for (int i = 0; i < json["notes"].size(); i++) {
					seq->notes[i] = json["notes"][i];
				}
			}
			if (json["octs"].is_array()) {
				for (int i = 0; i < json["octs"].size(); i++) {
					seq->octs[i] = json["octs"][i];
				}
			}
			if (json["enabled"].is_array()) {
				for (int i = 0; i < json["enabled"].size(); i++) {
					seq->enabled[i] = json["enabled"][i].get<bool>();
				}
			}
			return seq;
		});

		NodeBuilder::registerType<TimerNode>("General", TWEN_NODE_FAC {
			return new TimerNode(
				GET(float, "sampleRate", 44100.0f),
				GET(float, "bpm", 120.0f),
				GET(float, "swing", 0.0f)
			);
		});

		NodeBuilder::registerType<ValueNode>("General", TWEN_NODE_FAC {
			return new ValueNode(
				GET(float, "value", 0.0f)
			);
		});

		NodeBuilder::registerType<CompressorNode>("Effects", TWEN_NODE_FAC {
			return new CompressorNode(
				GET(float, "sampleRate", 44100.0f),
				GET(float, "pregain", 0.0f),
				GET(float, "threshold", -24.0f),
				GET(float, "knee", 30.0f),
				GET(float, "ratio", 12.0f),
				GET(float, "attack", 0.003f),
				GET(float, "release", 0.25f)
			);
		});

#undef GET
	}
}

#endif // TWEN_H