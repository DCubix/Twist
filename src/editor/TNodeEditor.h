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

#include "nodes/TNode.h"
#include "nodes/TOutNode.hpp"

#include "imgui.h"
#include "RtMidi.h"

#include "TNodeGraph.h"

void midiCallback(double dt, std::vector<uint8_t>* message, void* userData);

struct TMIDIMessage {
	
};

class TNodeEditor {
public:
	TNodeEditor();

	void draw(int w, int h);

	bool rendering() const { return m_rendering; }
	bool loading() const { return m_loading; }

	float output();

	void renderToFile(const std::string& fileName, float time);
	void saveRecording(const std::string& fileName);

	void closeGraph(int id);

	RtMidiIn* midiIn() { return m_MIDIin.get(); }
	RtMidiOut* midiOut() { return m_MIDIout.get(); }

	float sampleRate;

private:
	TNodeGraph* newGraph();
	void drawNodeGraph(TNodeGraph* graph);

	TLinking m_linking;

	ImVec4 m_bounds;

	int m_hoveredNode, m_nodeHoveredInList, m_activeGraph = 0, m_activeNodeIndex = -1;
	bool m_openContextMenu, m_selectingNodes = false, m_nodeActive, m_nodeAnyActive, m_nodeOldActive;
	float m_oldFontWindowScale, m_currentFontWindowScale;

	ImVec2 m_mainWindowSize, m_selectionStart, m_selectionEnd;

	float m_signalDC = 0.0f, m_envelope = 0.0f, m_outDuration = 0, m_recTime = 0.1f,
			m_recordingFadeTime = 0.0f, m_recordingFade = 0.0f;
	bool m_rendering = false, m_loading = false, m_playing = false, m_recording = false;

	std::vector<float> m_recordBuffer;
	int m_recordPointer = 0, m_recordingFadeType = 0;

	std::vector<std::unique_ptr<TNodeGraph>> m_nodeGraphs;

	std::unique_ptr<RtMidiIn> m_MIDIin;
	std::unique_ptr<RtMidiOut> m_MIDIout;

	std::mutex m_lock;
};

#endif // T_NODE_EDITOR_H
