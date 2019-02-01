#ifndef T_NODE_GRAPH_H
#define T_NODE_GRAPH_H

#define IMGUI_INCLUDE_IMGUI_USER_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include "TUndoRedo.h"
#include "twen/Node.h"
#include "twen/NodeGraph.h"

#include <functional>

using TNodeGUI = std::function<void(Node*)>;
struct TNode {
	ImVec4 bounds;
	ImRect selectionBounds;
	ImVec2 gridPos;
	bool open, selected;
	Node *node;

	ImVec2 pos(u32 s, float radius, bool snap=false, bool right=false) const {
		ImVec2 p = snap ? gridPos : ImVec2(bounds.x, bounds.y);
		float y = open ? (s * (radius*2 + 4)) : (s * radius * 2);
		return ImVec2(right ? p.x + bounds.z : p.x, p.y + y);
	}
	ImVec2 size() const { return ImVec2(bounds.z, bounds.w); }

};

struct TConnection {
	TNode* from;
	bool active;
};

class TNodeEditor;
class TNodeGraph {
	friend class TNodeEditor;
public:
	TNodeGraph(NodeGraph* ang);

	TNode* addNode(int x, int y, const Str& type, JSON params, bool canundo=true);
	void removeNode(TNode *nd, bool canundo=true);
	Connection* connect(TNode *from, TNode *to, u32 slot, bool canundo=true);

	void selectAll();
	void unselectAll();
	TNode* getActiveNode();

	void disconnect(Connection* conn, bool canundo=true);

	void load(const Str& fileName);
	void save(const Str& fileName);

	TNodeEditor* editor() { return m_editor; }
	void editor(TNodeEditor* ed) { m_editor = ed; }

	NodeGraph* actualNodeGraph() { return m_actualNodeGraph.get(); }
	TUndoRedo* undoRedo() { return m_undoRedo.get(); }
	Str name() const { return m_name; }

	void fromJSON(JSON json);
	void toJSON(JSON& json);

protected:
	Vec<const char*> getSampleNames();
	Ptr<NodeGraph> m_actualNodeGraph;

	TNodeEditor* m_editor;
	Ptr<TUndoRedo> m_undoRedo;

	std::mutex m_lock;

	Map<Node*, Ptr<TNode>> m_tnodes;

	ImVec2 m_scrolling;

	Str m_name, m_fileName;
	bool m_open = true, m_saved = false;
};

#endif // T_NODE_GRAPH_H
