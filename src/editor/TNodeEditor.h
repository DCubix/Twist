#ifndef T_NODE_EDITOR_H
#define T_NODE_EDITOR_H

#include <iostream>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <fstream>

#include "nodes/TNode.h"
#include "nodes/TOutNode.hpp"

#include "imgui.h"

using TNodeCtor = TNode*(JSON&);

class TNodeEditor {
public:
	TNodeEditor();
	virtual ~TNodeEditor();
	
	void draw(int w, int h);

	void addNode(int x, int y, TNode* node);
	void deleteNode(TNode* node);
	void link(int inID, int inSlot, int outID, int outSlot);

	bool rendering() const { return m_rendering; }

	float output();

	void renderToFile(const std::string& fileName, float time);
	void saveTNG(const std::string& fileName);
	void loadTNG(const std::string& fileName);

	float sampleRate;

	TOutNode* outNode() { return m_outputNode; }
	void solveNodes();

private:
	std::map<std::string, TNodeCtor*> m_nodeFactories;

	std::vector<TNode*> m_nodes;
	std::vector<TLink*> m_links;
	TLinking m_linking;
	
	ImVec2 m_scrolling;
	ImVec4 m_bounds;

	int m_hoveredNode, m_selectedNode, m_nodeHoveredInList;
	bool m_openContextMenu;
	float m_oldFontWindowScale;

	TOutNode* m_outputNode;

	float m_signalDC, m_envelope, m_outDuration = 0;
	bool m_rendering = false;

	std::vector<TLink*> getNodeLinks(TNode* node);
	std::vector<TNode*> getNodeInputs(TNode* node);
	std::vector<TNode*> buildNodes(TNode* out);

	TNode* getNode(int id);

	std::vector<TNode*> m_solvedNodes;

	void solve();
};

#endif // T_NODE_EDITOR_H