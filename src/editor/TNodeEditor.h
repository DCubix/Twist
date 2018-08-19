#ifndef T_NODE_EDITOR_H
#define T_NODE_EDITOR_H

#include <iostream>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstring>
#include <fstream>
#include <memory>

#include "nodes/TNode.h"
#include "nodes/TOutNode.hpp"

#include "imgui.h"

using TNodeCtor = TNode*(JSON&);

class TNodeEditor {
public:
	TNodeEditor();
	~TNodeEditor();

	void draw(int w, int h);

	TNode* addNode(int x, int y, TNodeCtor* ctor, JSON& json);
	void deleteNode(int id);
	void link(int inID, int inSlot, int outID, int outSlot);

	bool rendering() const { return m_rendering; }
	bool loading() const { return m_loading; }

	float output();

	void renderToFile(const std::string& fileName, float time);
	void saveRecording(const std::string& fileName);
	void saveTNG(const std::string& fileName);
	void loadTNG(const std::string& fileName);

	float sampleRate;

	TOutNode* outNode() { return m_outputNode; }
	void solve();

private:
	void solveNodes();
	std::map<std::string, TNodeCtor*> m_nodeFactories;

	std::map<int, std::unique_ptr<TNode>> m_nodes;
	std::vector<std::unique_ptr<TLink>> m_links;
	TLinking m_linking;

	ImVec2 m_scrolling;
	ImVec4 m_bounds;

	int m_hoveredNode, m_selectedNode, m_nodeHoveredInList;
	bool m_openContextMenu;
	float m_oldFontWindowScale;

	TOutNode* m_outputNode;

	float m_signalDC = 0.0f, m_envelope = 0.0f, m_outDuration = 0, m_recTime = 0.1f,
			m_recordingFadeTime = 0.0f, m_recordingFade = 0.0f;
	bool m_rendering = false, m_loading = false, m_playing = false, m_recording = false;

	std::vector<float> m_recordBuffer;
	int m_recordPointer = 0, m_recordingFadeType = 0;

	std::vector<int> getAllLinksRelatedToNode(int id);
	std::vector<int> getNodeLinks(int id);
	std::vector<int> getNodeInputs(int id);
	std::vector<int> buildNodes(int id);

	TNode* getNode(int id);

	std::vector<int> m_solvedNodes;

};

#endif // T_NODE_EDITOR_H
