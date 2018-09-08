#ifndef T_NODE_GRAPH_H
#define T_NODE_GRAPH_H

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include "TUndoRedo.h"
#include "twen/Node.h"
#include "twen/NodeGraph.h"

struct TNodeUI {
	ImVec4 bounds;
	ImRect selectionBounds;
	ImVec2 gridPos;
	bool open, selected;
	Node* node;

	ImVec2 inputPos(const Str& name, float radius, float title, bool snap=false) const {
		int s = node->inputs()[name].id;
		ImVec2 p = snap ? gridPos : ImVec2(bounds.x, bounds.y);
		float y = size().y - (title/2 + (s * (radius*2 + 3)));
		return ImVec2(p.x, p.y + y);
	}
	ImVec2 outputPos(const Str& name, float radius, float title, bool snap=false) const {
		int s = node->outputs()[name].id;
		ImVec2 p = snap ? gridPos : ImVec2(bounds.x, bounds.y);
		float y = title/2 + (s * (radius*2 + 3));
		return ImVec2(p.x + size().x, p.y + y);
	}
	ImVec2 size() const { return ImVec2(bounds.z, bounds.w); }
};

struct TLinking {
	u64 inputID;
	Str inputSlot;
	bool active;
	TNodeUI* node;
};

class TNodeEditor;
class TNodeGraph {
	friend class TNodeEditor;
public:
	TNodeGraph(NodeGraph* ang);

	TNodeUI* addNode(int x, int y, const Str& type, JSON params, u64 id, bool canundo=true);
	void deleteNode(u64 id, bool canundo=true);
	u64 link(u64 inID, const Str& inSlot, u64 outID, const Str& outSlot, bool canundo=true);
	
	void selectAll();
	void unselectAll();
	u64 getActiveNode();

	void removeLink(u64 id, bool canundo=true);

	TNodeUI* node(u64 id);
	NodeLink* link(u64 id);

	void load(const Str& fileName);
	void save(const Str& fileName);

	TNodeEditor* editor() { return m_editor; }
	void editor(TNodeEditor* ed) { m_editor = ed; }

	NodeGraph* actualNodeGraph() { return m_actualNodeGraph.get(); }

	TUndoRedo* undoRedo() { return m_undoRedo.get(); }

	Str name() const { return m_name; }

protected:
	Vec<const char*> getSampleNames();
	Ptr<NodeGraph> m_actualNodeGraph;

	TNodeEditor* m_editor;
	Ptr<TUndoRedo> m_undoRedo;

	std::mutex m_lock;

	Vec<u64> m_solvedNodes;
	Map<u64, Ptr<TNodeUI>> m_nodes;

	ImVec2 m_scrolling;

	Str m_name, m_fileName;
	bool m_open = true, m_saved = false;
};

#endif // T_NODE_GRAPH_H