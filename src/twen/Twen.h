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
#include "nodes/StorageNodes.hpp"
#include "nodes/ValueNode.hpp"

namespace Twen {
	inline void init() {
#define GET(type, v, d) (json[v].is_null() ? d : json[v].get<type>())

		NodeBuilder::registerType<ADSRNode>("Generators", TWEN_NODE_FAC {
			return new ADSRNode(
				GET(float, "a", 0.0f),
				GET(float, "d", 0.0f),
				GET(float, "s", 1.0f),
				GET(float, "r", 0.0f)
			);
		});

		NodeBuilder::registerType<ArpNode>("Generators", TWEN_NODE_FAC {
			return new ArpNode(
				(Note) GET(int, "note", 0),
				(ArpNode::Chord) GET(int, "chord", 0),
				(ArpNode::Direction) GET(int, "dir", 0),
				GET(float, "oct", 0)
			);
		});

		NodeBuilder::registerType<ChorusNode>("Effects", TWEN_NODE_FAC {
			return new ChorusNode(
				GET(float, "chorusRate", 0.0f),
				GET(float, "chorusDepth", 0.0f),
				GET(float, "delayTime", 0.0f)
			);
		});

		NodeBuilder::registerType<DelayLineNode>("Effects", TWEN_NODE_FAC {
			return new DelayLineNode(
				GET(float, "feedback", 0.0f),
				GET(float, "delay", 100.0f)
			);
		});

		NodeBuilder::registerType<FilterNode>("Effects", TWEN_NODE_FAC {
			return new FilterNode(
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

		NodeBuilder::registerType<OscillatorNode>("Generators", TWEN_NODE_FAC {
			return new OscillatorNode(
				GET(float, "freq", 220.0f),
				(OscillatorNode::WaveForm) GET(int, "wf", 0)
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

		NodeBuilder::registerType<ReaderNode>("General", TWEN_NODE_FAC {
			return new ReaderNode(GET(int, "idx", 0));
		});

		NodeBuilder::registerType<WriterNode>("General", TWEN_NODE_FAC {
			return new WriterNode(GET(int, "idx", 0));
		});

		NodeBuilder::registerType<ValueNode>("General", TWEN_NODE_FAC {
			return new ValueNode(
				GET(float, "value", 0.0f)
			);
		});

#undef GET
	}
}

#endif // TWEN_H