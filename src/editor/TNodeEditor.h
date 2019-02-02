#ifndef T_NODE_EDITOR_H
#define T_NODE_EDITOR_H

#include <iostream>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>

#include "TMidi.h"
#include "TCommands.h"
#include "TNodeGraph.h"

#include "imgui.h"

#include "twen/intern/Utils.h"
#include "twen/NodeGraph.h"
#include "twen/Node.h"

#if defined(__MINGW32__) || defined(__MINGW64__)
#include "SDL2/SDL.h"
#else
#include "SDL.h"
#endif

#define MAX_RECENT_FILES 6

void midiCallback(double dt, std::vector<uint8_t>* message, void* userData);

class TNodeEditor {
public:
	TNodeEditor();

	void draw(int w, int h);

	bool snapToGrid() const { return m_snapToGrid; }
	bool exit() const { return m_exit; }

	float output();

	void closeGraph();

	RtMidiIn* midiIn() { return m_MIDIin.get(); }
	RtMidiOut* midiOut() { return m_MIDIout.get(); }

	float sampleRate;

	void menuActionExit();

private:
	TNodeGraph* newGraph();
	void drawNodeGraph(TNodeGraph* graph);
	void menuActionOpen(const std::string& fileName="");
	void menuActionSave();
	void menuActionSnapAllToGrid();
	void saveRecentFiles();
	void pushRecentFile(const std::string& str);

	TConnection m_connection;
	TNode *m_activeNode, *m_hoveredNode;

	ImVec4 m_bounds;

	bool m_openContextMenu, m_selectingNodes = false,
		m_nodeActive, m_nodeAnyActive, m_nodeOldActive,
		m_nodesMoving, m_snapToGrid = false, m_snapToGridDisabled = false, m_editingSamples = false;
	float m_oldFontWindowScale, m_currentFontWindowScale;

	Vec<TNode*> m_moving;
	Map<TNode*, TMoveCommand::Point> m_moveDeltas;

	Map<TypeIndex, TNodeGUI> m_guis;

	ImVec2 m_mainWindowSize, m_selectionStart, m_selectionEnd;

	bool m_playing = false, m_exit = false;

	Ptr<TNodeGraph> m_nodeGraph;

	Vec<Str> m_recentFiles;

	Ptr<RtMidiIn> m_MIDIin;
	Ptr<RtMidiOut> m_MIDIout;

	std::mutex m_lock;
};

#endif // T_NODE_EDITOR_H
