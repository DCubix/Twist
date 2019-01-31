#include "TNodeEditor.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>

#include "TUndoRedo.h"

#include "OsDialog.hpp"
#include "sndfile.hh"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "icon_small.h"
#include "icon_big.h"
#include "about.h"

#ifdef WINDOWS
#define PATH_SEPARATOR '\\'
#include <windows.h>
#else
#define PATH_SEPARATOR '/'
#endif

// GUIs
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
#include "nodes/ReaderNode.hpp"
#include "nodes/RemapNode.hpp"
#include "nodes/ValueNode.hpp"
#include "nodes/WriterNode.hpp"

#define NODE_SLOT_RADIUS(x) (4.0f * x)
#define NODE_SLOT_RADIUS2(x) (5.0f * x)
#define LINK_THICKNESS(x) (2.0f * x)
#define NODE_ROUNDING(x) (2.5f * x)
#define NODE_PADDING(x) (4.0f * x)

inline static float ImVec2Dot(const ImVec2& S1,const ImVec2& S2) {return (S1.x*S2.x+S1.y*S2.y);}
inline static float GetSquaredDistancePointSegment(const ImVec2& P,const ImVec2& S1,const ImVec2& S2) {
  const float l2 = (S1.x-S2.x)*(S1.x-S2.x)+(S1.y-S2.y)*(S1.y-S2.y);
  if (l2 < 0.0000001f) return (P.x-S2.x)*(P.x-S2.x)+(P.y-S2.y)*(P.y-S2.y);   // S1 == S2 case
  ImVec2 T(S2 - S1);
  const float tf = ImVec2Dot(P - S1, T)/l2;
  const float minTf = 1.f < tf ? 1.f : tf;
  const float t = 0.f > minTf ? 0.f : minTf;
  T = S1 + T*t;  // T = Projection on the segment
  return (P.x-T.x)*(P.x-T.x)+(P.y-T.y)*(P.y-T.y);
}

inline static float GetSquaredDistanceToBezierCurve(const ImVec2& point,const ImVec2& p1,const ImVec2& p2,const ImVec2& p3,const ImVec2& p4)   {
	static const int num_segments = 2;   // Num Sampling Points In between p1 and p4
	static bool firstRun = true;
	static ImVec4 weights[num_segments];

	if (firstRun)    {
		// static init here
		IM_ASSERT(num_segments>0);    // This are needed for computing distances: cannot be zero
		firstRun = false;
		for (int i = 1; i <= num_segments; i++) {
			float t = (float)i/(float)(num_segments+1);
			float u = 1.0f - t;
			weights[i-1].x=u*u*u;
			weights[i-1].y=3*u*u*t;
			weights[i-1].z=3*u*t*t;
			weights[i-1].w=t*t*t;
		}
	}

	float minSquaredDistance=FLT_MAX,tmp;   // FLT_MAX is probably in <limits.h>
	ImVec2 L = p1,tp;
	for (int i = 0; i < num_segments; i++)  {
		const ImVec4& W=weights[i];
		tp.x = W.x*p1.x + W.y*p2.x + W.z*p3.x + W.w*p4.x;
		tp.y = W.x*p1.y + W.y*p2.y + W.z*p3.y + W.w*p4.y;

	tmp = GetSquaredDistancePointSegment(point,L,tp);
	if (minSquaredDistance>tmp) minSquaredDistance=tmp;
	L=tp;
	}
	tp = p4;
	tmp = GetSquaredDistancePointSegment(point,L,tp);
	if (minSquaredDistance>tmp) minSquaredDistance=tmp;

	return minSquaredDistance;
}

TNodeEditor::TNodeEditor() {
	m_connection.from = nullptr;
	m_connection.active = false;

	m_openContextMenu = false;
	m_bounds.x = 0;
	m_bounds.y = 0;
	m_bounds.z = 1;
	m_bounds.w = 1;
	m_oldFontWindowScale = 0;
	m_snapToGrid = true;
	m_snapToGridDisabled = false;

	// Load recent files
	std::ifstream fp(".recent");
	if (!fp.bad()) {
		std::string line;
		while (std::getline(fp, line)) {
			std::ifstream stm(line);
			if (stm.good()) {
				m_recentFiles.push_back(line);
			}
		}
		fp.close();
	}

	// Register GUIs
	m_guis[ADSRNode::typeID()] = ADSR_gui;
	m_guis[ArpNode::typeID()] = Arp_gui;
	m_guis[ChorusNode::typeID()] = Chorus_gui;
	m_guis[DelayLineNode::typeID()] = DelayLine_gui;
	m_guis[FilterNode::typeID()] = Filter_gui;
	m_guis[MathNode::typeID()] = Math_gui;
	m_guis[MixNode::typeID()] = Mix_gui;
	m_guis[NoteNode::typeID()] = Note_gui;
	m_guis[OscillatorNode::typeID()] = Oscillator_gui;
	m_guis[OutNode::typeID()] = Out_gui;
	m_guis[ReaderNode::typeID()] = Reader_gui;
	m_guis[WriterNode::typeID()] = Writer_gui;
	m_guis[RemapNode::typeID()] = Remap_gui;
	m_guis[ValueNode::typeID()] = Value_gui;
	//

	// Initialize MIDI
//	try {
//		m_MIDIin = std::unique_ptr<RtMidiIn>(new RtMidiIn(
//		RtMidi::UNSPECIFIED, "Twist MIDI In"
//		));
//		m_MIDIin->openPort(0, "Twist - Main In");
//		m_MIDIin->setCallback(&midiCallback, nullptr);
//		m_MIDIin->ignoreTypes(false, false, false);

//		m_MIDIout = std::unique_ptr<RtMidiOut>(new RtMidiOut(
//			RtMidi::UNSPECIFIED, "Twist MIDI Out"
//		));
//		m_MIDIout->openPort(0, "Twist - Main Out");
//	} catch (RtMidiError err) {
//		LogE(err.getMessage());
//	}

}

void TNodeEditor::menuActionOpen(const std::string& fileName) {
	if (fileName.empty()) {
		auto filePath = osd::Dialog::file(
			osd::DialogAction::OpenFile,
			".",
			osd::Filters("Twist Node-Graph:tng")
		);

		if (filePath.has_value()) {
			menuActionOpen(filePath.value());
			pushRecentFile(filePath.value());
		}
	} else {
		TNodeGraph* graph = newGraph();
		graph->load(fileName);
	}
}

void TNodeEditor::menuActionSave() {
	if (m_nodeGraph) {
		if (m_nodeGraph->m_fileName.empty()) {
			auto filePath = osd::Dialog::file(
				osd::DialogAction::SaveFile,
				".",
				osd::Filters("Twist Node-Graph:tng")
			);

			if (filePath.has_value()) {
				m_nodeGraph->save(filePath.value());
				pushRecentFile(filePath.value());
			}
		} else {
			m_nodeGraph->save(m_nodeGraph->m_fileName);
		}
	}
}

void TNodeEditor::menuActionExit() {
	if (m_nodeGraph) {
		if (!m_nodeGraph->m_saved) {
			if (osd::Dialog::message(
				osd::MessageLevel::Warning,
				osd::MessageButtons::YesNo,
				"You have unsaved changes. Continue?")
			) {
				m_exit = true;
			}
		} else {
			m_exit = true;
		}
	} else {
		m_exit = true;
	}

	if (m_exit) {
		saveRecentFiles();
	}
}

void TNodeEditor::menuActionSnapAllToGrid() {
	Vec<TNode*> movs;
	Map<TNode*, TMoveCommand::Point> movDel;

	for (auto&& [k, v] : m_nodeGraph->m_tnodes) {
		TNode* nd = v.get();
		nd->gridPos.x = (int(nd->bounds.x) / 8) * 8;
		nd->gridPos.y = (int(nd->bounds.y) / 8) * 8;
		movs.push_back(nd);
		movDel[nd] = TMoveCommand::Point(
			nd->gridPos.x - nd->bounds.x,
			nd->gridPos.y - nd->bounds.y
		);
	}
	m_snapToGridDisabled = true;

	m_nodeGraph->undoRedo()->performedAction<TMoveCommand>(
		m_nodeGraph.get(),
		movs,
		movDel
	);
}

void TNodeEditor::saveRecentFiles() {
	std::ofstream fp(".recent", std::ios::out | std::ios::binary);
	if (!fp.bad()) {
		for (std::string line : m_recentFiles) {
			fp << line << std::endl;
		}
		fp.close();
#ifdef WINDOWS
	int attr = GetFileAttributes(".recent");
	if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
		SetFileAttributes(".recent", attr | FILE_ATTRIBUTE_HIDDEN);
	}
#endif
	}
}

void TNodeEditor::pushRecentFile(const std::string& str) {
	if (std::find(m_recentFiles.begin(), m_recentFiles.end(), str) != m_recentFiles.end())
		return;
	m_recentFiles.push_back(str);
}

void TNodeEditor::drawNodeGraph(TNodeGraph* graph) {
	if (graph == nullptr) return;

	const ImGuiIO io = ImGui::GetIO();

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	m_currentFontWindowScale = 1;

	bool openContext = false;
	ImVec2 offset = ImGui::GetCursorScreenPos() + graph->m_scrolling;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float scl = m_currentFontWindowScale;

	// Display grid
	ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
	ImU32 GRID_COLOR_SM = IM_COL32(200, 200, 200, 20);
	float GRID_SZ = 64.0f * scl;
	float GRID_SZ_SMALL = 8.0f * scl;
	ImVec2 win_pos = ImGui::GetCursorScreenPos();
	ImVec2 canvas_sz = ImGui::GetWindowSize();

	for (float x = fmodf(graph->m_scrolling.x, GRID_SZ_SMALL); x < canvas_sz.x; x += GRID_SZ_SMALL)
		draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR_SM);
	for (float y = fmodf(graph->m_scrolling.y, GRID_SZ_SMALL); y < canvas_sz.y; y += GRID_SZ_SMALL)
		draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR_SM);

	for (float x = fmodf(graph->m_scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
		draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
	for (float y = fmodf(graph->m_scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
		draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);


	const bool isMouseHoveringWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
	const bool isLMBClicked = ImGui::IsMouseClicked(0);
	const bool isLMBDoubleClicked = ImGui::IsMouseDoubleClicked(0);
	const bool isMouseDraggingForMovingNodes = isMouseHoveringWindow && io.MouseDown[0];
	const bool isMouseDraggingForScrolling = ImGui::IsMouseDragging(2, 0.0f);
	const bool mustCheckForNearestLink = isMouseHoveringWindow && !m_connection.active && io.KeyShift;

	const float nodeTitleBarBgHeight = ImGui::GetTextLineHeightWithSpacing() + NODE_PADDING(scl);
	const float slotRadius = NODE_SLOT_RADIUS(scl);

	// Display links
	draw_list->ChannelsSplit(5);

	draw_list->ChannelsSetCurrent(0); // Background
	const float hoveredLinkDistSqrThres = 100.0f;

	Connection *nearestConn = nullptr;
	for (auto&& conn : graph->m_actualNodeGraph->connections()) {
		TNode* ni = graph->m_tnodes[conn->from].get();
		TNode* no = graph->m_tnodes[conn->to].get();
		if (ni == nullptr || no == nullptr) continue;

		ImVec2 p1 = offset + ni->outputPos(0, slotRadius, m_snapToGrid);
		p1.y += nodeTitleBarBgHeight;
		ImVec2 p2 = offset + no->inputPos(conn->toSlot, slotRadius, m_snapToGrid);
		p2.y += nodeTitleBarBgHeight;
		ImVec2 cp1 = p1 + ImVec2(50, 0);
		ImVec2 cp2 = p2 - ImVec2(50, 0);

		ImRect cullLink;
		cullLink.Min = cullLink.Max = p1;
		cullLink.Add(p2);

		float thick = LINK_THICKNESS(scl);
		cullLink.Expand(thick * 2.0f);

		draw_list->AddCircleFilled(p1, NODE_SLOT_RADIUS2(scl), IM_COL32(200, 200, 100, 255));
		draw_list->AddCircleFilled(p2, NODE_SLOT_RADIUS2(scl), IM_COL32(200, 200, 100, 255));
		draw_list->AddBezierCurve(p1, cp1, cp2, p2, IM_COL32(200, 200, 100, 255), thick);

		if (mustCheckForNearestLink && nearestConn == nullptr && cullLink.Contains(io.MousePos)) {
			const float d = GetSquaredDistanceToBezierCurve(io.MousePos, p1, cp1, cp2, p2);
			if (d < hoveredLinkDistSqrThres) {
				nearestConn = conn.get();
				draw_list->AddBezierCurve(p1, cp1, cp2, p2, IM_COL32(200, 200, 100, 128), thick*2);
			}
		}
	}
	if (nearestConn != nullptr && io.MouseReleased[0]) {
		graph->disconnect(nearestConn);
		nearestConn = nullptr;
	}

	// Display nodes
	std::vector<TNode*> toDelete;

	m_hoveredNode = nullptr;

	std::vector<TNode*> nodeList;
	nodeList.reserve(graph->m_tnodes.size());
	for (auto&& [k, v] : graph->m_tnodes) {
		if (v.get() == nullptr) continue;
		nodeList.push_back(v.get());
	}

	for (int node_id = 0; node_id < nodeList.size(); node_id++) {
		TNode* node = nodeList[node_id];
		Node* nodeR = node->node;
		if (node == nullptr) continue;

		if (m_snapToGridDisabled) {
			node->bounds.x = node->gridPos.x;
			node->bounds.y = node->gridPos.y;
		}

		if (!m_snapToGrid) {
			node->gridPos.x = node->bounds.x;
			node->gridPos.y = node->bounds.y;
		}

		ImGui::PushID(node_id);
		ImVec2 node_rect_min = offset + ImVec2(node->gridPos.x, node->gridPos.y) * scl;

		// Display node contents first
		draw_list->ChannelsSetCurrent(m_activeNode == node ? 4 : 2); // Foreground
		m_nodeOldActive = ImGui::IsAnyItemActive();

		ImGui::SetCursorScreenPos(node_rect_min + ImVec2(NODE_PADDING(scl), NODE_PADDING(scl)));
		ImGui::BeginGroup();
			ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Appearing);
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1, 1, 1, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1, 1, 1, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0));
			if (ImGui::TreeNode(node, "%s", "")) { ImGui::TreePop(); node->open = true; }
			else node->open = false;

			ImGui::PopStyleColor(3);
			ImGui::SameLine(0, 2);

			ImGui::Text("%s", nodeR->name().c_str());

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
			ImGui::SameLine();

			static const ImVec2 vec2zero(0, 0);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2zero);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, vec2zero);
			ImGui::PushID("NodeButtons");

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Enabled");
			}
			ImGui::SameLine();
//			if (ImGui::Button("C", ImVec2(15, 15))) {
//				int cx = int(node->bounds.x);
//				int cy = int(node->bounds.y + node->bounds.w);
//				JSON params; node->save(params);
//				params["open"] = true;
//				m_activeNode = graph->addNode(cx, cy, nodeR->name(), params, 0);
//			}
//			if (ImGui::IsItemHovered()) {
//				ImGui::SetTooltip("Clone");
//			}
//			ImGui::SameLine();
			if (ImGui::Button("X", ImVec2(15, 15))) {
				toDelete.push_back(node);
				m_hoveredNode = nullptr;
				m_activeNode = nullptr;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Delete");
			}

			ImGui::PopID();
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();

			if (node->open) {
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::BeginGroup();
				m_guis[nodeR->getType()](nodeR);
				ImGui::EndGroup();
			}
		ImGui::EndGroup();

		m_nodeAnyActive = (!m_nodeOldActive && ImGui::IsAnyItemActive());

		float pad = NODE_PADDING(scl);
		ImVec2 nodeSize = ImGui::GetItemRectSize() + ImVec2(pad, pad)*2;
		node->bounds.z = nodeSize.x;
		node->bounds.w = nodeSize.y;
		ImVec2 node_rect_max = node_rect_min + node->size();

		node->selectionBounds = ImRect(node_rect_min, node_rect_max-node_rect_min);

		// Display node box
		draw_list->ChannelsSetCurrent(m_activeNode == node ? 3 : 1); // Background
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node##invbtn", node->size());

		m_nodeActive = !isMouseDraggingForScrolling && ImGui::IsItemActive() && !m_selectingNodes;

		if (ImGui::IsItemHovered()) {
			m_hoveredNode = node;
		}

		if (isLMBClicked && (m_nodeAnyActive || m_nodeActive)) {
			if (!node->selected) {
				if (!io.KeyCtrl) graph->unselectAll();
				node->selected = true;
				m_activeNode = node;
			} else if (io.KeyCtrl) {
				node->selected = false;
				if (node == m_activeNode) {
					m_activeNode = graph->getActiveNode();
				}
			} else if (io.KeyShift || io.MouseDoubleClicked[0]) {
				graph->unselectAll();
				node->selected = true;
				m_activeNode = node;
			} else {
				m_activeNode = node;
			}
		}

		ImU32 node_bg_color = m_hoveredNode == node || node->selected ? IM_COL32(75, 75, 75, 255) : IM_COL32(60, 60, 60, 255);
		draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, NODE_ROUNDING(scl));

//		ImGui::DrawAudioView(
//			node_rect_min.x, node_rect_min.y,
//			node_rect_max.x - node_rect_min.x,
//			nodeR->buffer().data(),
//			TWEN_NODE_BUFFER_SIZE,
//			nodeTitleBarBgHeight,
//			NODE_ROUNDING(scl),
//			node->open ? ImDrawCornerFlags_Top : ImDrawCornerFlags_All
//		);
		draw_list->AddRectFilled(
			node_rect_min,
			ImVec2(node_rect_max.x, node_rect_min.y + nodeTitleBarBgHeight),
			ImGui::GetColorU32(node->open ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg),
			NODE_ROUNDING(scl),
			node->open ? ImDrawCornerFlags_Top : ImDrawCornerFlags_All
		);

		if (node->open) {
			draw_list->AddRect(
				node_rect_min,
				ImVec2(node_rect_max.x, node_rect_min.y + nodeTitleBarBgHeight),
				IM_COL32(100, 100, 100, 255),
				NODE_ROUNDING(scl),
				ImDrawCornerFlags_Top
			);
		}

		draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), NODE_ROUNDING(scl));

		const ImVec2 hsz(slotRadius*1.5f, slotRadius*1.5f);

		for (u32 in = 0; in < nodeR->inputs().size(); in++) {
			Str name = nodeR->inNames()[in];
			const char* label = name.c_str();
			ImVec2 pos = offset + node->inputPos(in, slotRadius, m_snapToGrid);
			pos.y += nodeTitleBarBgHeight;

			ImVec2 tsz = ImGui::CalcTextSize(label);
			ImVec2 off = ImVec2(-(tsz.x + slotRadius + 3), -tsz.y * 0.5f);
			ImVec2 off1 = ImVec2(-(tsz.x + slotRadius + 3), -tsz.y * 0.5f + 1);

			draw_list->AddText(pos + off1, IM_COL32(0, 0, 0, 220), label);
			draw_list->AddText(pos + off, IM_COL32(255, 255, 255, 220), label);
			draw_list->AddCircleFilled(pos, slotRadius, IM_COL32(150, 200, 150, 255));

			// End linking
			if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) &&
				ImGui::IsMouseReleased(0) &&
				m_connection.active && m_connection.from != node)
			{
				m_connection.active = false;
				graph->connect(m_connection.from, node, in);
			}
		}

		// Output
		ImVec2 pos = offset + node->outputPos(0, slotRadius, m_snapToGrid);
		pos.y += nodeTitleBarBgHeight;
		ImVec2 tsz = ImGui::CalcTextSize("Out");
		ImVec2 off = ImVec2(slotRadius + 3, -tsz.y * 0.5f);
		ImVec2 off1 = ImVec2(slotRadius + 3, -tsz.y * 0.5f + 1);

		draw_list->AddText(pos + off1, IM_COL32(0, 0, 0, 220), "Out");
		draw_list->AddText(pos + off, IM_COL32(255, 255, 255, 220), "Out");
		draw_list->AddCircleFilled(pos, slotRadius, IM_COL32(200, 150, 150, 255));

		/// Start linking process
		if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) && ImGui::IsMouseClicked(0)) {
			m_connection.active = true;
			m_connection.from = node;
		}

		if (m_connection.active && m_connection.from == node) {
			ImVec2 p1 = pos;
			ImVec2 p2 = ImGui::GetIO().MousePos;
			ImVec2 cp1 = p1 + ImVec2(50, 0);
			ImVec2 cp2 = p2 - ImVec2(50, 0);

			const u32 col = IM_COL32(200, 200, 100, 200);
			draw_list->AddCircleFilled(p1, slotRadius*0.5f, col);
			draw_list->AddBezierCurve(p1, cp1, cp2, p2, col, LINK_THICKNESS(scl));
		}
		ImGui::PopID();

		if (m_nodeActive && node->selected && isMouseDraggingForMovingNodes && !m_connection.active) {
			if (!m_nodesMoving) {
				m_moving.clear();
				m_moveDeltas.clear();
				for (int j = 0; j < nodeList.size(); j++) {
					TNode* nd = nodeList[j];
					if (nd->selected) {
						m_moving.push_back(nd);
						m_moveDeltas[nodeList[j]] = TMoveCommand::Point{0, 0};
					}
				}
				m_nodesMoving = true;
			}

			for (int j = 0; j < nodeList.size(); j++) {
				TNode* nd = nodeList[j];
				if (nd->selected) {
					nd->bounds.x += io.MouseDelta.x;
					nd->bounds.y += io.MouseDelta.y;
					if (std::abs(io.MouseDelta.x) > 0.0f || std::abs(io.MouseDelta.y) > 0.0f) {
						m_moveDeltas[nodeList[j]].x += io.MouseDelta.x;
						m_moveDeltas[nodeList[j]].y += io.MouseDelta.y;
					}

					if (m_snapToGrid) {
						nd->gridPos.x = (int(nd->bounds.x) / 8) * 8;
						nd->gridPos.y = (int(nd->bounds.y) / 8) * 8;
					}
				}
			}
		} else {
			if (!m_nodeOldActive && m_nodesMoving && !m_moveDeltas.empty()) {
				m_nodeGraph->undoRedo()->performedAction<TMoveCommand>(
					m_nodeGraph.get(),
					m_moving,
					m_moveDeltas
				);
				m_nodesMoving = false;
			}
		}

	}
	draw_list->ChannelsMerge();
	m_snapToGridDisabled = false;

	/// Selecting nodes
	if (!m_nodesMoving && ImGui::IsMouseClicked(0) && !m_connection.active && !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()) {
		if (!io.KeyCtrl) graph->unselectAll();
		m_selectionStart = io.MousePos;
		m_selectingNodes = true;
	} else if (m_selectingNodes && ImGui::IsMouseReleased(0)) {
		m_selectingNodes = false;
	}

	/// Selecting nodes
	if (m_selectingNodes) {
		m_selectionEnd = io.MousePos;

		ImRect selRect = ImRect(m_selectionStart, m_selectionEnd);
		if (selRect.Min.x>selRect.Max.x) {float tmp = selRect.Min.x;selRect.Min.x=selRect.Max.x;selRect.Max.x=tmp;}
		if (selRect.Min.y>selRect.Max.y) {float tmp = selRect.Min.y;selRect.Min.y=selRect.Max.y;selRect.Max.y=tmp;}

		draw_list->AddRectFilled(
			selRect.Min,
			selRect.Max,
			IM_COL32(100, 100, 100, 100)
		);
		draw_list->AddRect(
			selRect.Min,
			selRect.Max,
			IM_COL32(150, 150, 150, 150)
		);

		for (auto&& [k, v] : graph->m_tnodes) {
			TNode* node = v.get();
			ImRect nbounds = node->selectionBounds;

			if (selRect.Overlaps(nbounds)) {
				node->selected = true;
				if (m_activeNode == nullptr) m_activeNode = node;
			}
		}
	}

	// Scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f)) {
		graph->m_scrolling = graph->m_scrolling + ImGui::GetIO().MouseDelta;
		graph->m_saved = false;
	}

	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1)) {
		m_hoveredNode = nullptr;
		openContext = true;
	}
	if (openContext) {
		ImGui::OpenPopup("context_menu");
	}

	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("context_menu")) {
		ImGui::Text("Add Node");

		Map<Str, std::vector<NodeFactory>> facts;
		for (auto&& fn : NodeBuilder::factories) {
			facts[fn.second.category].push_back(fn.second);
		}

		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;

		JSON params;
		for (auto&& e : facts) {
			if (ImGui::BeginMenu(e.first.c_str())) {
				for (auto&& type : e.second) {
					if (ImGui::MenuItem(type.title.c_str())) {
						graph->addNode(scene_pos.x, scene_pos.y, type.type, params);
					}
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Delete nodes
	for (auto&& node : toDelete) {
		graph->removeNode(node);
	}

}

void TNodeEditor::draw(int w, int h) {
	auto style = &ImGui::GetStyle();
	style->WindowRounding = 5.3f;
	style->GrabRounding = style->FrameRounding = 2.3f;
	style->ScrollbarRounding = 5.0f;
	style->FrameBorderSize = 1.0f;
	style->ItemSpacing.y = 6.5f;

	style->Colors[ImGuiCol_Text]                  = {0.73333335f, 0.73333335f, 0.73333335f, 1.00f};
	style->Colors[ImGuiCol_TextDisabled]          = {0.34509805f, 0.34509805f, 0.34509805f, 1.00f};
	style->Colors[ImGuiCol_WindowBg]              = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
	style->Colors[ImGuiCol_ChildBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.00f};
	style->Colors[ImGuiCol_PopupBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
	style->Colors[ImGuiCol_Border]                = {0.33333334f, 0.33333334f, 0.33333334f, 0.50f};
	style->Colors[ImGuiCol_BorderShadow]          = {0.15686275f, 0.15686275f, 0.15686275f, 0.00f};
	style->Colors[ImGuiCol_FrameBg]               = {0.16862746f, 0.16862746f, 0.16862746f, 0.54f};
	style->Colors[ImGuiCol_FrameBgHovered]        = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
	style->Colors[ImGuiCol_FrameBgActive]         = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
	style->Colors[ImGuiCol_TitleBg]               = {0.04f, 0.04f, 0.04f, 1.00f};
	style->Colors[ImGuiCol_TitleBgCollapsed]      = {0.16f, 0.29f, 0.48f, 1.00f};
	style->Colors[ImGuiCol_TitleBgActive]         = {0.00f, 0.00f, 0.00f, 0.51f};
	style->Colors[ImGuiCol_MenuBarBg]             = {0.27058825f, 0.28627452f, 0.2901961f, 0.80f};
	style->Colors[ImGuiCol_ScrollbarBg]           = {0.27058825f, 0.28627452f, 0.2901961f, 0.60f};
	style->Colors[ImGuiCol_ScrollbarGrab]         = {0.21960786f, 0.30980393f, 0.41960788f, 0.51f};
	style->Colors[ImGuiCol_ScrollbarGrabHovered]  = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
	style->Colors[ImGuiCol_ScrollbarGrabActive]   = {0.13725491f, 0.19215688f, 0.2627451f, 0.91f};
	style->Colors[ImGuiCol_CheckMark]             = {0.90f, 0.90f, 0.90f, 0.83f};
	style->Colors[ImGuiCol_SliderGrab]            = {0.70f, 0.70f, 0.70f, 0.62f};
	style->Colors[ImGuiCol_SliderGrabActive]      = {0.30f, 0.30f, 0.30f, 0.84f};
	style->Colors[ImGuiCol_Button]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.49f};
	style->Colors[ImGuiCol_ButtonHovered]         = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
	style->Colors[ImGuiCol_ButtonActive]          = {0.13725491f, 0.19215688f, 0.2627451f, 1.00f};
	style->Colors[ImGuiCol_Header]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.53f};
	style->Colors[ImGuiCol_HeaderHovered]         = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
	style->Colors[ImGuiCol_HeaderActive]          = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
	style->Colors[ImGuiCol_Separator]             = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_SeparatorHovered]      = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_SeparatorActive]       = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_ResizeGrip]            = {1.00f, 1.00f, 1.00f, 0.85f};
	style->Colors[ImGuiCol_ResizeGripHovered]     = {1.00f, 1.00f, 1.00f, 0.60f};
	style->Colors[ImGuiCol_ResizeGripActive]      = {1.00f, 1.00f, 1.00f, 0.90f};
	style->Colors[ImGuiCol_PlotLines]             = {0.61f, 0.61f, 0.61f, 1.00f};
	style->Colors[ImGuiCol_PlotLinesHovered]      = {1.00f, 0.43f, 0.35f, 1.00f};
	style->Colors[ImGuiCol_PlotHistogram]         = {0.90f, 0.70f, 0.00f, 1.00f};
	style->Colors[ImGuiCol_PlotHistogramHovered]  = {1.00f, 0.60f, 0.00f, 1.00f};
	style->Colors[ImGuiCol_TextSelectedBg]        = {0.18431373f, 0.39607847f, 0.79215693f, 0.90f};
	style->Colors[ImGuiCol_ModalWindowDarkening]  = {0.0f, 0.0f, 0.0f, 0.5f};

	bool showAbout = false;
	if (ImGui::BeginMainMenuBar()) {
		/// Shortcut handling
		if (ImGui::HotKey(CTRL, SDL_SCANCODE_N)) {
			newGraph();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_O)) {
			menuActionOpen();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_S)) {
			menuActionSave();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_Z)) {
			m_nodeGraph->undoRedo()->undo();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_Y)) {
			m_nodeGraph->undoRedo()->redo();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_Q)) {
			menuActionExit();
		}

		if (ImGui::TwistTex == nullptr) {
			ImGui::TwistTex = new TTex(twist_small_png, twist_small_png_len);
		}

		if (ImGui::TwistBigTex == nullptr) {
			ImGui::TwistBigTex = new TTex(twist_big_png, twist_big_png_len);
		}

		ImGui::Image(
			(ImTextureID)(ImGui::TwistTex->id()),
			ImVec2(16, 16)
		);
		ImGui::SameLine();

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", "Ctrl+N")) {
				newGraph();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
				menuActionOpen();
			}
			if (ImGui::BeginMenu("Open Recent...", "Ctrl+O")) {
				for (std::string file : m_recentFiles) {
					std::string fname = file;
					if (file.size() > 28) {
						fname = file.substr(file.size()-28);
						fname = fname.substr(fname.find_first_of(PATH_SEPARATOR));
						fname = std::string("...") + fname;
					}
					if (ImGui::MenuItem(fname.c_str())) {
						menuActionOpen(file);
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				menuActionSave();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
				menuActionExit();
			}
			ImGui::EndMenu();
		}
		ImGui::SameLine();
		if (ImGui::BeginMenu("Edit")) {
			bool active = m_nodeGraph != nullptr;
			bool uactive = active && m_nodeGraph->undoRedo()->canUndo();
			bool ractive = active && m_nodeGraph->undoRedo()->canRedo();
			if (ImGui::MenuItem("Undo", "Ctrl+Z", false, uactive)) {
				m_nodeGraph->undoRedo()->undo();
			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y", false, ractive)) {
				m_nodeGraph->undoRedo()->redo();
			}
			ImGui::Separator();

			bool sntgEnabled = m_nodeGraph != nullptr;
			if (ImGui::MenuItem("Snap nodes to grid", nullptr, false, sntgEnabled)) {
				menuActionSnapAllToGrid();
			}
			ImGui::EndMenu();
		}
		ImGui::SameLine();
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Snap to Grid", nullptr, &m_snapToGrid)) {
				if (!m_snapToGrid) {
					m_snapToGridDisabled = true;
				}
			}
			ImGui::EndMenu();
		}
		ImGui::SameLine();
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About")) {
				showAbout = true;
			}
			ImGui::EndMenu();
		}

		ImGui::ToggleButton("Play", &m_playing);

		ImGui::EndMainMenuBar();
	}

	if (showAbout)
		ImGui::OpenPopup("About Twist");
	if (ImGui::BeginPopupModal("About Twist", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Image(
			(ImTextureID)(ImGui::TwistBigTex->id()),
			ImVec2(96, 96)
		);
		ImGui::SameLine();
		ImGui::BeginGroup();

		std::stringstream stm;
		stm << TWIST_NAME << " v" << TWIST_VERSION;

		ImGui::Text("%s", stm.str().c_str());
		ImGui::Separator();
		ImGui::Text("%s", ABOUT_INFO.c_str());
		ImGui::Text("Web: ");
		ImGui::SameLine(0, 0);
		if (ImGui::LinkText("http://dcubix.github.io")) {
			osd::Dialog::web("http://dcubix.github.io");
		}
		ImGui::Text("E-mail: ");
		ImGui::SameLine(0, 0);
		if (ImGui::LinkText("diego95lopes@gmail.com")) {
			osd::Dialog::web("mailto:diego95lopes@gmail.com");
		}
		ImGui::EndGroup();

		ImGui::Separator();
		if (ImGui::LinkText("User Manual")) {
			osd::Dialog::web("https://github.com/DCubix/Twist/wiki/User-Manual");
		}
		ImGui::SameLine(0, 0);
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 60);
		if (ImGui::Button("Ok", ImVec2(60, 22)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSize(ImVec2(w, h-18), 0);
	ImGui::SetNextWindowPos(ImVec2(0, 18), 0);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	int flags = ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse |
				ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("main", nullptr, flags)) {
		if (ImGui::BeginChild("scrolling_region", ImVec2(-1, -1), true, flags) && m_nodeGraph) {
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(40, 40, 50, 200));
			ImGui::BeginChild("scrolling_region_", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PushItemWidth(80);

			m_mainWindowSize = ImGui::GetWindowSize();
			if (m_nodeGraph && m_nodeGraph->m_open) {
				drawNodeGraph(m_nodeGraph.get());
			}

			if (m_connection.active && ImGui::IsMouseReleased(0)) {
				m_connection.active = false;
				m_connection.from = nullptr;
			}

			ImGui::PopItemWidth();
			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);
		}
		ImGui::EndChild();

	}
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::PopStyleVar();

}

void TNodeEditor::closeGraph() {
	m_playing = false;
	if (m_nodeGraph) {
		m_nodeGraph.reset();
	}
	newGraph();
}

TNodeGraph* TNodeEditor::newGraph() {
	TNodeGraph* graph = new TNodeGraph(new NodeGraph());

	graph->m_name = "Untitled";
	graph->m_editor = this;

	m_nodeGraph = Ptr<TNodeGraph>(graph);
	return m_nodeGraph.get();
}

float TNodeEditor::output() {
	float sample = 0.0f;
	if (m_playing && m_nodeGraph) {
		TMessageBus::process();
		sample = m_nodeGraph->actualNodeGraph()->sample();
	}
	return sample;
}

void midiCallback(double dt, std::vector<uint8_t>* message, void* userData) {
	unsigned int nBytes = message->size();
	if (nBytes > 3) return;

	TRawMidiMessage rawMsg;
	for (int i = 0; i < nBytes; i++)
		rawMsg[i] = (*message)[i];

	TMidiMessage msg(rawMsg);
	TMessageBus::broadcast(msg.channel, msg.command, msg.param0, msg.param1);
}
