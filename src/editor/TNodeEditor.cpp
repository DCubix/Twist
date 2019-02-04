#include "TNodeEditor.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>

#include <filesystem>
namespace fs = std::filesystem;

#include "TUndoRedo.h"

#include "OsDialog.hpp"
#include "TAudio.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "icon_small.h"
#include "icon_big.h"
#include "about.h"
#include "play.h"
#include "stop.h"
#include "record.h"

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
#include "nodes/ButtonNode.hpp"
#include "nodes/SequencerNode.hpp"
#include "nodes/SamplerNode.hpp"

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

	m_playIcon = nullptr;
	m_stopIcon = nullptr;

	m_recordingBufferPos = 0;
	m_recordingBuffer.reserve(48000 * 90);

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
	m_guis[ButtonNode::typeID()] = Button_gui;
	m_guis[SequencerNode::typeID()] = Sequencer_gui;
	m_guis[SamplerNode::typeID()] = Sampler_gui;
	//

	NodeBuilder::registerType<ButtonNode>("Generators", TWEN_NODE_FAC {
		return new ButtonNode();
	});

	NodeBuilder::registerType<SequencerNode>("Generators", TWEN_NODE_FAC {
		return new SequencerNode(json);
	});

}

TNodeEditor::~TNodeEditor() {
	delete m_playIcon;
	delete m_stopIcon;
	delete m_recIcon;
}

void TNodeEditor::menuActionOpen(const std::string& fileName) {
	if (fileName.empty()) {
		auto filePath = osd::Dialog::file(
			osd::DialogAction::OpenFile,
			".",
			osd::Filters("Twist Synth:syn")
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
				osd::Filters("Twist Synth:syn")
			);

			if (filePath.has_value()) {
				fs::path fp = fs::u8path(filePath.value());
				if (fp.extension().empty()) {
					fp.replace_extension(".syn");
				}
				m_nodeGraph->save(fp.u8string());
				pushRecentFile(fp.u8string());
			}
		} else {
			m_nodeGraph->save(m_nodeGraph->m_fileName);
		}
	}
}

void TNodeEditor::menuActionSaveAs() {
	if (m_nodeGraph) {
		auto filePath = osd::Dialog::file(
			osd::DialogAction::SaveFile,
			".",
			osd::Filters("Twist Synth:syn")
		);

		if (filePath.has_value()) {
			fs::path fp = fs::u8path(filePath.value());
			if (fp.extension().empty()) {
				fp.replace_extension(".syn");
			}
			m_nodeGraph->save(fp.u8string());
			pushRecentFile(fp.u8string());
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

	saveRecentFiles();
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
	std::ofstream fp(".recent");
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

		ImVec2 p1 = offset + ni->pos(0, slotRadius, m_snapToGrid, true);
		ImVec2 p2 = offset + no->pos(conn->toSlot, slotRadius, m_snapToGrid);
		if (ni->open) p1.y += nodeTitleBarBgHeight;
		if (no->open) p2.y += nodeTitleBarBgHeight;
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
				draw_list->AddBezierCurve(p1, cp1, cp2, p2, IM_COL32(250, 200, 100, 128), thick*4);
			}
		}
	}
	if (nearestConn != nullptr && io.MouseReleased[0]) {
		m_lock.lock();
		graph->disconnect(nearestConn);
		m_lock.unlock();
		nearestConn = nullptr;
	}

	// Display nodes
	std::vector<TNode*> toDelete;

	m_hoveredNode = nullptr;

	for (auto&& [nodeR, node_] : graph->m_tnodes) {
		TNode *node = node_.get();
		if (node == nullptr) continue;

		if (m_snapToGridDisabled) {
			node->bounds.x = node->gridPos.x;
			node->bounds.y = node->gridPos.y;
		}

		if (!m_snapToGrid) {
			node->gridPos.x = node->bounds.x;
			node->gridPos.y = node->bounds.y;
		}

		ImGui::PushID(node);
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

			if (node->closeable) {
				if (ImGui::Button("C", ImVec2(15, 15))) {
					m_lock.lock();
					int cx = int(node->bounds.x);
					int cy = int(node->bounds.y + node->bounds.w);
					JSON params; nodeR->save(params);
					params["open"] = true;
					m_activeNode = graph->addNode(cx, cy, nodeR->typeName(), params, 0);
					m_activeNode->node->load(params);
					m_lock.unlock();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Clone");
				}
				ImGui::SameLine();
				if (ImGui::Button("X", ImVec2(15, 15))) {
					toDelete.push_back(node);
					m_hoveredNode = nullptr;
					m_activeNode = nullptr;
					saveBackup();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Delete");
				}
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

		node->selectionBounds = ImRect(node_rect_min, node_rect_max);

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
			ImVec2 pos = offset + node->pos(in, slotRadius, m_snapToGrid);
			if (node->open) pos.y += nodeTitleBarBgHeight;

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
				m_lock.lock();
				graph->connect(m_connection.from, node, in);
				saveBackup();
				m_lock.unlock();
			}
		}

		// Output
		ImVec2 pos = offset + node->pos(0, slotRadius, m_snapToGrid, true);
		if (node->open) pos.y += nodeTitleBarBgHeight;

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
				for (auto&& [n, tnd] : graph->m_tnodes) {
					TNode* nd = tnd.get();
					if (nd->selected) {
						m_moving.push_back(nd);
						m_moveDeltas[nd] = TMoveCommand::Point{0, 0};
					}
				}
				m_nodesMoving = true;
			}

			for (auto&& [n, tnd] : graph->m_tnodes) {
				TNode* nd = tnd.get();
				if (nd->selected) {
					nd->bounds.x += io.MouseDelta.x;
					nd->bounds.y += io.MouseDelta.y;
					if (std::abs(io.MouseDelta.x) > 0.0f || std::abs(io.MouseDelta.y) > 0.0f) {
						m_moveDeltas[nd].x += io.MouseDelta.x;
						m_moveDeltas[nd].y += io.MouseDelta.y;
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
				saveBackup();
			}
		}

	}
	draw_list->ChannelsMerge();
	m_snapToGridDisabled = false;

	/// Selecting nodes
	if (!m_nodesMoving && ImGui::IsMouseClicked(0) &&
		!m_connection.active && !ImGui::IsAnyItemActive() &&
		!ImGui::IsAnyItemHovered())
	{
		if (!io.KeyCtrl) graph->unselectAll();
		m_selectionStart = io.MousePos;
		m_selectingNodes = true;
	} else if (m_selectingNodes && ImGui::IsMouseReleased(0)) {
		m_selectingNodes = false;
	}

	/// Selecting nodes
	if (m_selectingNodes && !m_editingSamples) {
		m_selectionEnd = io.MousePos;

		ImRect selRect = ImRect(m_selectionStart, m_selectionEnd);
		if (selRect.Min.x > selRect.Max.x) {
			std::swap(selRect.Min.x, selRect.Max.x);
		}
		if (selRect.Min.y > selRect.Max.y) {
			std::swap(selRect.Min.y, selRect.Max.y);
		}

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

		//selRect.Min -= m_nodeGraph->m_scrolling;
		//selRect.Max -= m_nodeGraph->m_scrolling;

		for (auto&& [k, v] : graph->m_tnodes) {
			TNode* node = v.get();
			draw_list->AddRect(
				node->selectionBounds.Min,
				node->selectionBounds.Max,
				IM_COL32(255, 0, 0, 150)
			);

			ImRect nbounds = node->selectionBounds;

			if (selRect.Overlaps(nbounds)) {
				node->selected = true;
				if (m_activeNode == nullptr) m_activeNode = node;
			} else {
				if (node->selected) {
					node->selected = false;
				}
			}
		}
	}

	// Scrolling
	bool panAction = ImGui::IsMouseDragging(2, 0.0f) ||
					(ImGui::IsMouseDragging(1, 0.0f) && io.KeyCtrl);
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && panAction) {
		graph->m_scrolling = graph->m_scrolling + ImGui::GetIO().MouseDelta;
		graph->m_saved = false;
	}

	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1) && !io.KeyCtrl) {
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
						m_lock.lock();
						graph->addNode(scene_pos.x, scene_pos.y, type.type, params);
						saveBackup();
						m_lock.unlock();
					}
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Delete nodes
	m_lock.lock();
	for (auto&& node : toDelete) {
		graph->removeNode(node);
	}
	m_lock.unlock();

}

static bool VectorOfStringGetter(void* data, int n, const char** out_text) {
	const Vec<Str>* v = (Vec<Str>*)data;
	*out_text = v->at(n).c_str();
	return true;
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

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 12.0f));
	float menuHeight = 0.0f;
	if (ImGui::BeginMainMenuBar()) {
		/// Shortcut handling
		if (ImGui::HotKey(CTRL, SDL_SCANCODE_N)) {
			newGraph();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_O)) {
			menuActionOpen();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_S)) {
			menuActionSave();
		} else if (ImGui::HotKey(CTRL | SHIFT, SDL_SCANCODE_S)) {
			menuActionSaveAs();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_Z)) {
			if (m_nodeGraph) m_nodeGraph->undoRedo()->undo();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_Y)) {
			if (m_nodeGraph) m_nodeGraph->undoRedo()->redo();
		} else if (ImGui::HotKey(CTRL, SDL_SCANCODE_Q)) {
			menuActionExit();
		}

		if (ImGui::TwistTex == nullptr) {
			ImGui::TwistTex = new TTex(twist_small_png, twist_small_png_len);
		}

		if (ImGui::TwistBigTex == nullptr) {
			ImGui::TwistBigTex = new TTex(twist_big_png, twist_big_png_len);
		}

		if (m_playIcon == nullptr) {
			m_playIcon = new TTex(play_data, play_size);
		}

		if (m_stopIcon == nullptr) {
			m_stopIcon = new TTex(stop_data, stop_size);
		}

		if (m_recIcon == nullptr) {
			m_recIcon = new TTex(record_data, record_size);
		}

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", "Ctrl+N")) {
				newGraph();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
				menuActionOpen();
			}
			if (ImGui::BeginMenu("Open Recent...")) {
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
			if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
				menuActionSaveAs();
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
				saveBackup();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Sample Library") && m_nodeGraph) {
				m_editingSamples = true;
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

		if (m_nodeGraph) {
			ImGui::SameLine();

			ImGui::PushItemWidth(80);
			float bpm = m_nodeGraph->actualNodeGraph()->bpm();
			if (ImGui::DragFloat("BPM", &bpm, 0.1f, 40, 240)) {
				m_nodeGraph->actualNodeGraph()->bpm(bpm);
			}
			ImGui::SameLine();

			u32 bars = m_nodeGraph->actualNodeGraph()->bars();
			if (ImGui::DragInt("Bars", (int*)&bars, 0.1f, 2, 7)) {
				m_nodeGraph->actualNodeGraph()->bars(bars);
			}
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::SameLine();
			bool playPressed = ImGui::ImageButton(
				m_playing ? (ImTextureID)(m_stopIcon->id()) : (ImTextureID)(m_playIcon->id()),
				ImVec2(22, 22)
			);
			if (playPressed) {
				m_playing = !m_playing;
				reset();
			}
		}

		menuHeight = ImGui::GetWindowSize().y;
		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleVar();

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
		if (ImGui::Button("Ok", ImVec2(60, 22))) {
			ImGui::CloseCurrentPopup();
			showAbout = false;
		}
		ImGui::EndPopup();
	}

	const float recordingAreaHeight = 120;

	ImGui::SetNextWindowSize(ImVec2(w, h - menuHeight - recordingAreaHeight), 0);
	ImGui::SetNextWindowPos(ImVec2(0, menuHeight), 0);
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

	ImGui::SetNextWindowSize(ImVec2(w, recordingAreaHeight), 0);
	ImGui::SetNextWindowPos(ImVec2(0, h - recordingAreaHeight), 0);
	ImGui::SetNextWindowBgAlpha(1.0f);

	flags &= ~ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoCollapse;

	if (ImGui::Begin("Recording", nullptr, flags)) {
		if (ImGui::BeginChild("##rec_buttons", ImVec2(100, -1), true, flags)) {
			ImGui::PushItemWidth(70);
			if (m_recording) {
				float secRaw = (float(m_recordingBufferPos) / m_nodeGraph->actualNodeGraph()->sampleRate()) * 1000.0f;
				u32 secsRaw = u32(secRaw / 1000.0f);
				u32 secs = secsRaw % 60;
				u32 mins = secsRaw / 60;
				u32 millis = u32(secRaw) % 1000;
				ImGui::Text("Recorded: ");
				ImGui::Text("%02d:%02d:%02d", mins, secs, millis);
			}

			bool recClick = ImGui::Button(m_recording ? "Stop" : "Record", ImVec2(-1, 20));

			if (recClick && m_nodeGraph) {
				m_recording = !m_recording;
				if (!m_recording) {
					m_recordingBuffer = Vec<float>(
								m_recordingBuffer.begin(),
								m_recordingBuffer.begin() + m_recordingBufferPos
					);
				} else {
					const u32 maxSamples = m_nodeGraph->actualNodeGraph()->sampleRate() * MAX_SAMPLES_SECONDS;
					m_recordingBuffer.resize(maxSamples);
					std::fill(m_recordingBuffer.begin(), m_recordingBuffer.end(), 0.0f);
				}
				m_recordingBufferPos = 0;
				reset();
			}

			if (!m_recordingBuffer.empty() && !m_playing && !m_recording) {
				if (ImGui::Button("Save", ImVec2(-1, 20))) {
					auto filePath = osd::Dialog::file(
						osd::DialogAction::SaveFile,
						".",
						osd::Filters("Waveform Audio Format:wav")
					);

					if (filePath.has_value()) {
						const u32 sr = u32(m_nodeGraph->actualNodeGraph()->sampleRate());
						TAudioFile snd(filePath.value(), true, sr);
						snd.writef(m_recordingBuffer.data(), m_recordingBuffer.size());
						std::fill(m_recordingBuffer.begin(), m_recordingBuffer.end(), 0.0f);
					}
				}
			}

			ImGui::PopItemWidth();
		}
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImVec2 sz = ImGui::GetContentRegionAvail();
		if (ImGui::BeginChild("##rec_buf", ImVec2(sz.x, -1), true, flags)) {
			ImGui::AudioView(
				"##audio_view_rec_buf",
				sz.x,
				m_recordingBuffer.data(),
				m_recordingBuffer.size(),
				m_recordingBufferPos, sz.y
			);
		}
		ImGui::EndChild();

		ImGui::PopStyleVar(2);
	}
	ImGui::End();

	ImGui::PopStyleVar();

	if (m_editingSamples)
		ImGui::OpenPopup("Sample Library");
	if (ImGui::BeginPopupModal("Sample Library", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		static int selectedSample = -1;
		auto items = m_nodeGraph->actualNodeGraph()->getSampleNames();

		ImGui::ListBox("##sampleLib", &selectedSample, VectorOfStringGetter, &items, items.size(), 8);
		ImGui::SameLine();

		ImGui::BeginGroup();
		float w = ImGui::GetContentRegionAvailWidth();
		if (ImGui::Button("Load", ImVec2(w, 18))) {
			auto filePath = osd::Dialog::file(
				osd::DialogAction::OpenFile,
				".",
				osd::Filters("Audio Files:wav,ogg,aif,flac")
			);

			if (filePath.has_value()) {
				if (!m_nodeGraph->actualNodeGraph()->addSample(filePath.value())) {
					osd::Dialog::message(
						osd::MessageLevel::Error,
						osd::MessageButtons::Ok,
						"Ivalid sample. It must have <= 15 seconds."
					);
				} else { saveBackup(); }
			}
		}
		if (!items.empty()) {
			if (ImGui::Button("Delete", ImVec2(w, 18))) {
				m_nodeGraph->actualNodeGraph()->removeSample(items[selectedSample]);
				LogI("Deleted sample: ", items[selectedSample]);
				selectedSample = -1;
				saveBackup();
			}
		}
		if (ImGui::Button("Close", ImVec2(w, 18))) {
			ImGui::CloseCurrentPopup();
			m_editingSamples = false;
		}
		ImGui::EndGroup();
		ImGui::EndPopup();
	}

}

void TNodeEditor::closeGraph() {
	m_playing = false;
	if (m_nodeGraph) {
		m_nodeGraph.reset();
	}
	newGraph();
}

void TNodeEditor::reset() {
	m_lock.lock();
#define CastNode(T, v) (dynamic_cast<T*>(v))
	for (auto&& [k, v] : m_nodeGraph->m_tnodes) {
		auto type = k->getType();
		if (type == SamplerNode::typeID()) {
			CastNode(SamplerNode, k)->sampleData.reset();
		} else if (type == ADSRNode::typeID()) {
			CastNode(ADSRNode, k)->adsr().reset();
		} else if (type == OscillatorNode::typeID()) {
			CastNode(OscillatorNode, k)->reset();
		}
	}
	m_nodeGraph->actualNodeGraph()->reset();
	m_lock.unlock();
}

void TNodeEditor::saveBackup() {
//	if (m_nodeGraph) {
//		m_nodeGraph->save("backup.syn");
//	}
}

TNodeGraph* TNodeEditor::newGraph() {
	TNodeGraph* graph = new TNodeGraph(new NodeGraph(), 320, 240);

	graph->m_name = "Untitled";
	graph->m_editor = this;

	m_nodeGraph = Ptr<TNodeGraph>(graph);
	return m_nodeGraph.get();
}

float TNodeEditor::output() {
	float sample = 0.0f;
	if ((m_playing || m_recording) && m_nodeGraph) {
//		TMessageBus::process();
		sample = m_nodeGraph->actualNodeGraph()->sample();
	}

	if (m_recording) {
		const u32 maxSamples = m_nodeGraph->actualNodeGraph()->sampleRate() * MAX_SAMPLES_SECONDS;
		m_recordingBuffer[m_recordingBufferPos] = sample;
		if (m_recordingBufferPos++ >= maxSamples) {
			m_recordingBufferPos = 0;
			m_recording = false;
		}
	}

	return sample;
}

//void midiCallback(double dt, std::vector<uint8_t>* message, void* userData) {
//	unsigned int nBytes = message->size();
//	if (nBytes > 3) return;

//	TRawMidiMessage rawMsg;
//	for (int i = 0; i < nBytes; i++)
//		rawMsg[i] = (*message)[i];

//	TMidiMessage msg(rawMsg);
//	TMessageBus::broadcast(msg.channel, msg.command, msg.param0, msg.param1);
//}
