#ifndef TWEN_NODE_GRAPH_H
#define TWEN_NODE_GRAPH_H

#include "intern/Utils.h"
#include "Node.h"
#include "NodeRegistry.h"

#include "nodes/OutNode.hpp"

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

	NodeGraph() : m_type(GraphType::Normal) { m_globalStorage.fill(0.0f); }

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
	u64 create(Args&&... args) {
		static_assert(
			std::is_base_of<Node, Nt>::value,
			"The node must be derived from 'Node'."
		);
		// static_assert(IsNode<Nt>::value, "Not a valid node type.");

		Nt* nt = new Nt(args...);
		nt->m_parent = this;
		nt->m_name = Nt::type();

		u64 id = nt->id();
		m_nodes.insert({ nt->id(), Ptr<Node>(std::move(nt)) });

		solveNodes();

		if (Nt::type() == OutNode::type()) {
			m_outputNode = id;
		}

		return id;
	}

	Node* create(const Str& typeName, JSON params, u64 id = 0) {
		Node* node = NodeBuilder::createNode(typeName, params);
		node->m_parent = this;
		node->m_name = typeName;
		if (id > 0)
			node->m_id = id;
		
		u64 nid = node->id();
		m_nodes.insert({ nid, Ptr<Node>(std::move(node)) });

		solveNodes();

		if (node->name() == OutNode::type()) {
			m_outputNode = nid;
		}

		return get<Node>(nid);
	}

	u64 link(u64 inID, const Str& inSlot, u64 outID, const Str& outSlot);

	Map<u64, Ptr<Node>>& nodes() { return m_nodes; }
	Map<u64, Ptr<NodeLink>>& links() { return m_links; }
	void removeLink(u64 id);
	NodeLink* link(u64 id);

	// u64 inputsNode() const { return m_inputsNode; }
	// u64 outputsNode() const { return m_outputsNode; }
	u64 outputNode() const { return m_outputNode; }

	void store(u32 loc, float value) { m_globalStorage[loc] = value; }
	float load(u32 loc) const { return m_globalStorage[loc]; }

	bool addSample(const Str& fileName);
	void removeSample(u64 id);
	u64 getSampleID(const Str& name);
	RawSample* getSample(u64 id);
	Map<u64, Ptr<RawSample>>& sampleLibrary() { return m_sampleLibrary; }
	Vec<Str> getSampleNames();

	float solve();
	void solveNodes();

	void fromJSON(JSON json);
	void toJSON(JSON& json);

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
	u64 m_outputNode = 0;

	Map<u64, Ptr<Node>> m_nodes;
	Map<u64, Ptr<NodeLink>> m_links;
	std::mutex m_lock;

	Vec<u64> m_solvedNodes;

	Arr<float, GLOBAL_STORAGE_SIZE> m_globalStorage;

	Vec<Str> m_sampleNames;
	Map<u64, Ptr<RawSample>> m_sampleLibrary;

	Str m_name;
};

#endif // TWEN_NODE_GRAPH_H