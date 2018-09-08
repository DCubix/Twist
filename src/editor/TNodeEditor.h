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
#include "twen/nodes/OutNode.hpp"

#include "SDL2/SDL.h"

#define MAX_RECENT_FILES 6

void midiCallback(double dt, std::vector<uint8_t>* message, void* userData);

class TNodeEditor {
public:
	TNodeEditor();

	void draw(int w, int h);

	bool rendering() const { return m_rendering; }
	bool loading() const { return m_loading; }
	bool snapToGrid() const { return m_snapToGrid; }
	bool exit() const { return m_exit; }

	float output();

	void renderToFile(const std::string& fileName, float time);
	void saveRecording(const std::string& fileName);

	void closeGraph(int id);

	RtMidiIn* midiIn() { return m_MIDIin.get(); }
	RtMidiOut* midiOut() { return m_MIDIout.get(); }

	float sampleRate;

	void menuActionExit();

private:
	TNodeGraph* newGraph();
	void drawNodeGraph(TNodeGraph* graph);
	void menuActionOpen(const std::string& fileName="");
	void menuActionSave(int id = -1);
	void menuActionSnapAllToGrid();
	void saveRecentFiles();
	void pushRecentFile(const std::string& str);

	TLinking m_linking;

	ImVec4 m_bounds;

	int m_hoveredNode, m_nodeHoveredInList, m_activeGraph = 0, m_activeNodeIndex = -1;
	bool m_openContextMenu, m_selectingNodes = false,
		m_nodeActive, m_nodeAnyActive, m_nodeOldActive,
		m_nodesMoving, m_snapToGrid = false, m_snapToGridDisabled = false;
	float m_oldFontWindowScale, m_currentFontWindowScale;

	Vec<u64> m_movingIDs;
	Map<u64, TMoveCommand::Point> m_moveDeltas;

	ImVec2 m_mainWindowSize, m_selectionStart, m_selectionEnd;

	float m_signalDC = 0.0f, m_envelope = 0.0f, m_outDuration = 0, m_recTime = 0.1f,
			m_recordingFadeTime = 0.0f, m_recordingFade = 0.0f;
	bool m_rendering = false, m_loading = false, m_playing = false, m_recording = false, m_exit=false;

	Vec<float> m_recordBuffer;
	int m_recordPointer = 0, m_recordingFadeType = 0;

	Vec<Ptr<TNodeGraph>> m_nodeGraphs;

	Vec<Str> m_recentFiles;

	Ptr<RtMidiIn> m_MIDIin;
	Ptr<RtMidiOut> m_MIDIout;

	std::mutex m_lock;
};

#endif // T_NODE_EDITOR_H
