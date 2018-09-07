#ifndef TWEN_NODE_GRAPH_H
#define TWEN_NODE_GRAPH_H

#include "intern/Utils.h"
#include "Node.h"
#include "NodeRegistry.h"

#include <mutex>

#define GLOBAL_STORAGE_SIZE 0xFF

struct RawSample {
	Vec<float> data;
	float sampleRate, duration;
	Str name;
};

class NodeGraph {
public:
	enum GraphType {
		Normal = 0,
		Module
	};

	NodeGraph() { m_globalStorage.fill(0.0f); }

	template <typename Nt>
	Nt* get(u64 id) {
		static_assert(
			std::is_base_of<Node, Nt>::value,
			"The node must be derived from 'Node'."
		);
		// static_assert(IsNode<Nt>::value, "Not a valid node type.");

		if (m_nodes.find(id) == m_nodes.end()) return nullptr;
		return (Nt*) m_nodes[id].get();
	}

	void remove(u64 id);

	template <typename Nt, typename... Args>
	Nt* create(Args&&... args) {
		static_assert(
			std::is_base_of<Node, Nt>::value,
			"The node must be derived from 'Node'."
		);
		// static_assert(IsNode<Nt>::value, "Not a valid node type.");

		Nt* nt = new Nt(args...);
		nt->m_parent = this;

		u64 id = nt->id();
		m_nodes.insert({ nt->id(), Ptr<Node>(std::move(nt)) });

		solveNodes();

		return get<Nt>(id);
		
		// if (type == TOutputsNode::type()) {
		// 	m_outputsNode = nid;
		// }
		// if (type == TInputsNode::type()) {
		// 	m_inputsNode = nid;
		// }
		// if (type == TOutNode::type()) {
		// 	m_outputNode = nid;
		// }
	}

	Node* create(const Str& typeName) {
		return NodeBuilder::createNode<Node>(typeName, JSON());
	}

	u64 link(int inID, const Str& inSlot, int outID, const Str& outSlot);

	Map<u64, Ptr<Node>>& nodes() { return m_nodes; }
	Map<u64, Ptr<NodeLink>>& links() { return m_links; }
	void removeLink(u64 id);
	NodeLink* link(u64 id);

	u64 inputsNode() const { return m_inputsNode; }
	u64 outputsNode() const { return m_outputsNode; }
	u64 outputNode() const { return m_outputNode; }

	void store(u32 loc, float value) { m_globalStorage[loc] = value; }
	float load(u32 loc) const { return m_globalStorage[loc]; }

	bool addSample(const Str& fileName);
	void removeSample(u64 id);
	u64 getSampleID(const Str& name);
	RawSample* getSample(u64 id);

	float solve();
	void solveNodes();

	void fromJSON(JSON json);

	GraphType type() const { return m_type; }
	Str name() const { return m_name; }

private:
	void addSample(const Str& fname, const Vec<float>& data, float sr, float dur);
	Vec<u64> getAllLinksRelatedToNode(u64 id);
	Vec<u64> getNodeInputs(u64 id);
	Vec<u64> buildNodes(u64 id);
	Vec<u64> buildNodes(const Vec<u64>& ids);
	void solveNodes(const Vec<u64>& solved);

	GraphType m_type;
	u64 m_outputNode = 0, m_inputsNode = 0, m_outputsNode = 0;

	Map<u64, Ptr<Node>> m_nodes;
	Map<u64, Ptr<NodeLink>> m_links;
	std::mutex m_lock;

	Vec<u64> m_solvedNodes;

	Arr<float, GLOBAL_STORAGE_SIZE> m_globalStorage;
	Map<u64, Ptr<RawSample>> m_sampleLibrary;

	Str m_name;
};

#endif // TWEN_NODE_GRAPH_H