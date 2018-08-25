#include "TNodeEditor.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "nodes/TADSRNode.hpp"
#include "nodes/TButtonNode.hpp"
#include "nodes/TChorusNode.hpp"
#include "nodes/TFilterNode.hpp"
#include "nodes/TMathNode.hpp"
#include "nodes/TMixNode.hpp"
#include "nodes/TNoteNode.hpp"
#include "nodes/TOscillatorNode.hpp"
#include "nodes/TOutNode.hpp"
#include "nodes/TRemapNode.hpp"
#include "nodes/TSequencerNode.hpp"
#include "nodes/TValueNode.hpp"
#include "nodes/TTimerNode.hpp"
#include "nodes/TReverbNode.hpp"
#include "nodes/TModuleNode.hpp"
#include "nodes/TDelayLineNode.hpp"
#include "nodes/TStorageNodes.hpp"
#include "nodes/TSampleNode.hpp"

#include "tinyfiledialogs.h"
#include "sndfile.hh"

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#define NODE_CTOR [](JSON& json) -> TNode*

#define NODE_SLOT_RADIUS(x) (5.0f * x)
#define NODE_SLOT_RADIUS2(x) (6.0f * x)
#define LINK_THICKNESS(x) (5.0f * x)
#define NODE_ROUNDING(x) (5.0f * x)
#define NODE_PADDING(x) (5.0f * x)

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
	m_linking.active = 0;
	m_linking.inputID = 0;
	m_linking.inputSlot = 0;
	m_linking.node = nullptr;
	m_openContextMenu = false;
	m_bounds.x = 0;
	m_bounds.y = 0;
	m_bounds.z = 1;
	m_bounds.w = 1;
	m_oldFontWindowScale = 0;

	m_MIDIin = std::unique_ptr<RtMidiIn>(new RtMidiIn(
		RtMidi::UNSPECIFIED, "Twist MIDI In"
	));
	m_MIDIin->openPort(0, "Twist - Main In");
	m_MIDIin->setCallback(&midiCallback, nullptr);
	m_MIDIin->ignoreTypes(false, false, false);

	m_MIDIout = std::unique_ptr<RtMidiOut>(new RtMidiOut(
		RtMidi::UNSPECIFIED, "Twist MIDI Out"
	));
	m_MIDIout->openPort(0, "Twist - Main Out");

	TNodeFactory::registerNode<TValueNode>(NODE_CTOR {
		return new TValueNode{ GET(float, "value", 0.0f) };
	});

	TNodeFactory::registerNode<TMixNode>(NODE_CTOR {
		return new TMixNode{ GET(float, "factor", 0.5f) };
	});

	TNodeFactory::registerNode<TOscillatorNode>(NODE_CTOR {
		return new TOscillatorNode{
			GET(float, "sampleRate", 44100),
			(TOsc::TWave) GET(int, "waveForm", 0),
			GET(float, "freq", 440),
			GET(float, "amp", 1)
		};
	});

	TNodeFactory::registerNode<TNoteNode>(NODE_CTOR {
		return new TNoteNode{
			(Notes) GET(int, "note", 0),
			GET(int, "oct", 0)
		};
	});

	TNodeFactory::registerNode<TChorusNode>(NODE_CTOR {
		return new TChorusNode{
			GET(float, "sampleRate", 44100),
			GET(float, "delayTime", 50),
			GET(float, "chorusRate", 1),
			GET(float, "chorusDepth", 1)
		};
	});

	TNodeFactory::registerNode<TRemapNode>(NODE_CTOR {
		return new TRemapNode{
			GET(float, "omin", 0),
			GET(float, "omax", 1),
			GET(float, "nmin", 0),
			GET(float, "nmax", 1)
		};
	});

	TNodeFactory::registerNode<TSequencerNode>(NODE_CTOR {
		TSequencerNode* seq = new TSequencerNode{};
		if (json["key"].is_number_integer()) {
			seq->key = json["key"];
		}
		if (json["notes"].is_array()) {
			for (int i = 0; i < json["notes"].size(); i++) {
				seq->notes[i] = json["notes"][i];
			}
		}
		if (json["octs"].is_array()) {
			for (int i = 0; i < json["octs"].size(); i++) {
				seq->octs[i] = json["octs"][i];
			}
		}
		if (json["enabled"].is_array()) {
			for (int i = 0; i < json["enabled"].size(); i++) {
				seq->enabled[i] = json["enabled"][i].get<bool>();
			}
		}
		return seq;
	});

	TNodeFactory::registerNode<TButtonNode>(NODE_CTOR {
		return new TButtonNode{
			GET(bool, "on", false)
		};
	});

	TNodeFactory::registerNode<TFilterNode>(NODE_CTOR {
		return new TFilterNode{
			GET(float, "sampleRate", 44100),
			GET(float, "cutOff", 20),
			(TFilterNode::TFilter) GET(int, "filter", 0)
		};
	});

	TNodeFactory::registerNode<TMathNode>(NODE_CTOR {
		return new TMathNode{
			(TMathNode::TMathNodeOp) GET(int, "op", 0),
			GET(float, "a", 0),
			GET(float, "b", 0)
		};
	});

	TNodeFactory::registerNode<TADSRNode>(NODE_CTOR {
		return new TADSRNode{
			GET(float, "sampleRate", 44100),
			GET(float, "a", 0),
			GET(float, "d", 0),
			GET(float, "s", 0),
			GET(float, "r", 0)
		};
	});

	TNodeFactory::registerNode<TTimerNode>(NODE_CTOR {
		return new TTimerNode{
			GET(float, "sampleRate", 44100),
			GET(float, "bpm", 120),
			GET(float, "swing", 0)
		};
	});

	TNodeFactory::registerNode<TReverbNode>(NODE_CTOR {
		TReverbNode* rv = new TReverbNode{
			GET(float, "sampleRate", 44100)
		};
		if (json["preset"].is_array()) {
			auto preset = json["preset"];
			rv->pset.osf = preset[0];
			rv->pset.p1 = preset[1];
			rv->pset.p2 = preset[2];
			rv->pset.p3 = preset[3];
			rv->pset.p4 = preset[4];
			rv->pset.p5 = preset[5];
			rv->pset.p6 = preset[6];
			rv->pset.p7 = preset[7];
			rv->pset.p8 = preset[8];
			rv->pset.p9 = preset[9];
			rv->pset.p10 = preset[10];
			rv->pset.p11 = preset[11];
			rv->pset.p12 = preset[12];
			rv->pset.p13 = preset[13];
			rv->pset.p14 = preset[14];
			rv->pset.p15 = preset[15];
			rv->pset.p16 = preset[16];
		}
		return rv;
	});

	TNodeFactory::registerNode<TOutNode>(NODE_CTOR {
		TOutNode* out = new TOutNode();
		out->volume = GET(float, "volume", 1.0f);
		return out;
	});

	TNodeFactory::registerNode<TInputsNode>(NODE_CTOR {
		TInputsNode* tin = new TInputsNode();
		if (json["inputs"].is_array()) {
			for (int i = 0; i < json["inputs"].size(); i++) {
				tin->addOutput(json["inputs"][i]);
			}
		}
		return tin;
	});

	TNodeFactory::registerNode<TOutputsNode>(NODE_CTOR {
		TOutputsNode* tout = new TOutputsNode();
		if (json["outputs"].is_array()) {
			for (int i = 0; i < json["outputs"].size(); i++) {
				tout->addInput(json["outputs"][i]);
			}
		}
		return tout;
	});

	TNodeFactory::registerNode<TModuleNode>(NODE_CTOR {
		TModuleNode* mod = new TModuleNode();
		mod->filePath = json["filePath"].is_string() ? json["filePath"] : "";
		mod->load(mod->filePath);
		return mod;
	});

	TNodeFactory::registerNode<TDelayLineNode>(NODE_CTOR {
		return new TDelayLineNode{
			GET(float, "sampleRate", 44100.0f),
			GET(float, "feedback", 0.0f),
			GET(int, "delay", 10)
		};
	});

	TNodeFactory::registerNode<TReaderNode>(NODE_CTOR {
		return new TReaderNode{
			GET(int, "idx", 0)
		};
	});

	TNodeFactory::registerNode<TWriterNode>(NODE_CTOR {
		return new TWriterNode{
			GET(int, "idx", 0)
		};
	});

	TNodeFactory::registerNode<TSampleNode>(NODE_CTOR {
		return new TSampleNode(
			GET(int, "sample", 0),
			GET(int, "selectedID", 0),
			GET(float, "volume", 1.0f)
		);
	});

}

void TNodeEditor::drawNodeGraph(TNodeGraph* graph) {
	if (graph == nullptr) return;

	const ImGuiIO io = ImGui::GetIO();

	// Zooming
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	// float oldFontScaleToReset = g.Font->Scale; // We'll clean up at the bottom
	// float fontScaleStored = m_oldFontWindowScale ? m_oldFontWindowScale : oldFontScaleToReset;
	// float& fontScaleToTrack = g.Font->Scale;

	// if (!io.FontAllowUserScaling) {
	// 	// Set the correct font scale (3 lines)
	// 	fontScaleToTrack = fontScaleStored;
	// 	g.FontBaseSize = io.FontGlobalScale * g.Font->Scale * g.Font->FontSize;
	// 	g.FontSize = window->CalcFontSize();

	// 	if (ImGui::IsMouseHoveringRect(window->InnerMainRect.Min, window->InnerMainRect.Max) && io.MouseWheel)   {

	// 		// Zoom / Scale window
	// 		float new_font_scale = ImClamp(fontScaleToTrack + g.IO.MouseWheel * 0.075f, 0.50f, 2.50f);
	// 		float scale = new_font_scale / fontScaleToTrack;
	// 		if (scale != 1)	{
	// 			graph->m_scrolling = graph->m_scrolling * scale;
	// 			// Set the correct font scale (3 lines), and store it
	// 			fontScaleStored = fontScaleToTrack = new_font_scale;
	// 			g.FontBaseSize = io.FontGlobalScale * g.Font->Scale * g.Font->FontSize;
	// 			g.FontSize = window->CalcFontSize();
	// 		}
	// 	}
	// }

	// fixes zooming just a bit
	// bool nodesHaveZeroSize = false;
	// m_currentFontWindowScale = !io.FontAllowUserScaling ? fontScaleStored : ImGui::GetCurrentWindow()->FontWindowScale;
	// if (m_oldFontWindowScale == 0.0f) {
	// 	m_oldFontWindowScale = m_currentFontWindowScale;
	// 	nodesHaveZeroSize = true;   // at start or after clear()
	// 	graph->m_scrolling = ImGui::GetWindowSize() * 0.5f;
	// } else if (m_oldFontWindowScale != m_currentFontWindowScale) {
	// 	nodesHaveZeroSize = true;
	// 	for (auto& e : graph->nodes())    {
	// 		TNode* node = e.second.get();
	// 		node->m_bounds.z = 0.0f;  // we must reset the size
	// 		node->m_bounds.w = 0.0f;  // we must reset the size
	// 	}
	// 	// These two lines makes the scaling work around the mouse position AFAICS
	// 	if (io.FontAllowUserScaling) {
	// 		const ImVec2 delta = (io.MousePos - ImGui::GetCursorScreenPos());
	// 		graph->m_scrolling -= (delta * m_currentFontWindowScale - delta * m_oldFontWindowScale) / m_currentFontWindowScale;
	// 	}
	// 	m_oldFontWindowScale = m_currentFontWindowScale;
	// }

	m_currentFontWindowScale = 1;

	bool openContext = false;
	ImVec2 offset = ImGui::GetCursorScreenPos() + graph->m_scrolling;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float scl = m_currentFontWindowScale;

	// Display grid
	ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
	float GRID_SZ = 64.0f * scl;
	ImVec2 win_pos = ImGui::GetCursorScreenPos();
	ImVec2 canvas_sz = ImGui::GetWindowSize();
	for (float x = fmodf(graph->m_scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
		draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
	for (float y = fmodf(graph->m_scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
		draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);

	const bool isMouseHoveringWindow = ImGui::IsMouseHoveringRect(window->InnerMainRect.Min, window->InnerMainRect.Max);
	const bool mustCheckForNearestLink = isMouseHoveringWindow && !m_linking.active && io.KeyShift;

	// Display links
	draw_list->ChannelsSplit(2);
	draw_list->ChannelsSetCurrent(0); // Background
	const float hoveredLinkDistSqrThres = 100.0f;
	int nearestLinkId=-1;
	for (int link_idx = 0; link_idx < graph->m_links.size(); link_idx++) {
		auto& link = graph->m_links[link_idx];
		
		TNode* ni = graph->node(link->inputID);
		TNode* no = graph->node(link->outputID);
		if (ni == nullptr || no == nullptr) continue;

		ImVec2 p1 = offset + ni->outputSlotPos(link->inputSlot, scl);
		ImVec2 p2 = offset + no->inputSlotPos(link->outputSlot, scl);
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

		if (mustCheckForNearestLink && nearestLinkId == -1 && cullLink.Contains(io.MousePos)) {
			const float d = GetSquaredDistanceToBezierCurve(io.MousePos, p1, cp1, cp2, p2);
			if (d < hoveredLinkDistSqrThres) {
				nearestLinkId = link_idx;
				draw_list->AddBezierCurve(p1, cp1, cp2, p2, IM_COL32(200, 200, 100, 128), thick*2);
			}
		}
	}
	if (nearestLinkId != -1 && io.MouseReleased[0]) {
		graph->removeLink(nearestLinkId);
		nearestLinkId = -1;
	}

	// Display nodes
	std::vector<int> toDelete;

	m_hoveredNode = -1;
	m_nodeActive = false;
	for (auto& e : graph->nodes()) {
		TNode* node = e.second.get();
		if (node == nullptr) continue;

		ImGui::PushID(node->id());
		ImVec2 node_rect_min = offset + ImVec2(node->m_bounds.x, node->m_bounds.y) * scl;

		// Display node contents first
		draw_list->ChannelsSetCurrent(1); // Foreground
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

			ImGui::Text("%s", node->m_title.c_str());
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
			ImGui::SameLine();

			static const ImVec2 vec2zero(0, 0);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2zero);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, vec2zero);
			ImGui::PushID("NodeButtons");
			if (ImGui::Button("X", ImVec2(15, 15))) {
				toDelete.push_back(node->id());
				m_selectedNodes.clear();
				m_hoveredNode = -1;
			}
			ImGui::PopID();
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();

			if (node->open) {
				ImGui::Spacing();
				ImGui::BeginGroup();
					node->gui();
				ImGui::EndGroup();
			}
		ImGui::EndGroup();

		// Save the size of what we have emitted and whether any of the widgets are being used
		m_nodeAnyActive = (!m_nodeOldActive && ImGui::IsAnyItemActive());

		float pad = NODE_PADDING(scl);
		ImVec2 nodeSize = ImGui::GetItemRectSize() + ImVec2(pad, pad)*2;
		node->m_bounds.z = nodeSize.x;
		node->m_bounds.w = nodeSize.y;
		ImVec2 node_rect_max = node_rect_min + node->size();
		
		node->m_selectionBounds = ImRect(node_rect_min, node_rect_max);

		// Display node box
		draw_list->ChannelsSetCurrent(0); // Background
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node##invbtn", node->size());

		if (!m_nodeActive)
			m_nodeActive = ImGui::IsItemActive() || m_nodeAnyActive;

		if (ImGui::IsItemHovered()) {
			m_hoveredNode = node->id();
			openContext |= ImGui::IsMouseClicked(1);
		}

		auto pos = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node->id());
		ImU32 node_bg_color = m_hoveredNode == node->id() || pos != m_selectedNodes.end() ? IM_COL32(85, 85, 85, 255) : IM_COL32(60, 60, 60, 255);
		draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, NODE_ROUNDING(scl));
		draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), NODE_ROUNDING(scl));
		
		const float slotRadius = NODE_SLOT_RADIUS(scl);
		const ImVec2 hsz(slotRadius*1.5f, slotRadius*1.5f);

		for (int i = 0; i < node->inputs().size(); i++) {
			const char* label = node->inputs()[i].label.c_str();
			ImVec2 pos = offset + node->inputSlotPos(i, scl);
			ImVec2 tsz = ImGui::CalcTextSize(label);
			ImVec2 off = ImVec2(-(tsz.x + slotRadius + 3), -tsz.y * 0.5f);
			ImVec2 off1 = ImVec2(-(tsz.x + slotRadius + 3), -tsz.y * 0.5f + 1);

			draw_list->AddText(pos + off1, IM_COL32(0, 0, 0, 220), label);
			draw_list->AddText(pos + off, IM_COL32(255, 255, 255, 220), label);
			draw_list->AddCircleFilled(pos, slotRadius, IM_COL32(150, 200, 150, 255));

			// End linking
			if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) &&
				ImGui::IsMouseReleased(0) &&
				m_linking.active && m_linking.node != node)
			{
				m_linking.active = false;
				graph->link(m_linking.inputID, m_linking.inputSlot, node->id(), i);
			}
		}
		
		for (int i = 0; i < node->outputs().size(); i++) {
			const char* label = node->outputs()[i].label.c_str();
			ImVec2 pos = offset + node->outputSlotPos(i, scl);
			ImVec2 tsz = ImGui::CalcTextSize(label);
			ImVec2 off = ImVec2(slotRadius + 3, -tsz.y * 0.5f);
			ImVec2 off1 = ImVec2(slotRadius + 3, -tsz.y * 0.5f + 1);

			draw_list->AddText(pos + off1, IM_COL32(0, 0, 0, 220), label);
			draw_list->AddText(pos + off, IM_COL32(255, 255, 255, 220), label);
			draw_list->AddCircleFilled(pos, slotRadius, IM_COL32(200, 150, 150, 255));

			/// Start linking process
			if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) && ImGui::IsMouseClicked(0)) {
				m_linking.active = true;
				m_linking.node = node;
				m_linking.inputID = node->id();
				m_linking.inputSlot = i;
			}

			if (m_linking.active && m_linking.node == node && m_linking.inputSlot == i) {
				ImVec2 p1 = pos;
				ImVec2 p2 = ImGui::GetIO().MousePos;
				ImVec2 cp1 = p1 + ImVec2(50, 0);
				ImVec2 cp2 = p2 - ImVec2(50, 0);

				const int col = IM_COL32(200, 200, 100, 200);
				const float r = slotRadius * 1.5f;
				draw_list->AddCircleFilled(p1, slotRadius*0.5f, col);
				draw_list->AddTriangleFilled(
					ImVec2(0, -r) + p2,
					ImVec2(r+0.5f,  0) + p2,
					ImVec2(0,  r) + p2,
					col
				);
				draw_list->AddBezierCurve(p1, cp1, cp2, p2, col, 5.0f);
			}
		}

		ImGui::PopID();
	}

	/// Selecting nodes
	if (!m_nodeActive && ImGui::IsMouseClicked(0) && !m_linking.active) {
		m_selectionStart = io.MousePos;
		m_selectingNodes = true;
	} else if (m_selectingNodes && ImGui::IsMouseReleased(0)) {
		m_selectingNodes = false;
	} else if (!m_selectingNodes && ImGui::IsMouseClicked(0) && !m_linking.active) {
		if (m_nodeActive) {
			if (io.KeyCtrl && std::find(m_selectedNodes.begin(), m_selectedNodes.end(), m_hoveredNode) == m_selectedNodes.end()) {
				m_selectedNodes.push_back(m_hoveredNode);
			} else {
				if (m_selectedNodes.size() == 1)
					m_selectedNodes.clear();
			}
		} else {
			m_selectedNodes.clear();
		}
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

		for (auto& e : graph->nodes()) {
			TNode* node = e.second.get();
			ImRect nbounds = node->m_selectionBounds;

			if (selRect.Overlaps(nbounds)) {
				if (std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node->id()) == m_selectedNodes.end()) {
					m_selectedNodes.push_back(node->id());
				}
			} else {
				auto pos = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node->id());
				if (pos != m_selectedNodes.end()) {
					m_selectedNodes.erase(pos);
				}
			}
		}
	}

	draw_list->ChannelsMerge();

	// Moving selected nodes
	if (!m_selectingNodes && ImGui::IsMouseDragging(0) && !m_linking.active) {
		if (m_selectedNodes.empty() && m_hoveredNode != -1) {
			m_selectedNodes.push_back(m_hoveredNode);
		}
		for (int id : m_selectedNodes) {
			TNode* node = graph->nodes()[id].get();
			if (node == nullptr) continue;
			node->m_bounds.x += ImGui::GetIO().MouseDelta.x / scl;
			node->m_bounds.y += ImGui::GetIO().MouseDelta.y / scl;
			graph->m_saved = false;
		}
	}

	// Scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f)) {
		graph->m_scrolling = graph->m_scrolling + ImGui::GetIO().MouseDelta;
		graph->m_saved = false;
	}

	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1)) {
		m_hoveredNode = -1;
		m_selectedNodes.clear();
		openContext = true;
	}
	if (openContext) {
		ImGui::OpenPopup("context_menu");
		if (m_hoveredNode != -1) {
			m_selectedNodes.push_back(m_hoveredNode);
		}
	}

	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("context_menu")) {
		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
		if (m_selectedNodes.empty()) {
			for (auto fn : TNodeFactory::factories) {
				if ((fn.first == TInputsNode::type() ||
					 fn.first == TOutputsNode::type()) &&
					graph->type() != TNodeGraph::Module)
				{
					continue;
				}
				if (ImGui::MenuItem(fn.first.c_str())) {
					graph->addNode(scene_pos.x, scene_pos.y, fn.first);
				}
			}
		} else {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Delete nodes
	for (int i : toDelete) {
		graph->deleteNode(i);
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

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				newGraph();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Open (*.tng)", "Ctrl+O")) {
				const static char* FILTERS[] = { "*.tng\0" };
				const char* filePath = tinyfd_openFileDialog(
					"Open",
					"",
					1,
					FILTERS,
					"Twist Node-Graph",
					0
				);
				if (filePath) {
					TNodeGraph* graph = newGraph();
					graph->load(std::string(filePath));
				}
			}
			if (ImGui::MenuItem("Save (*.tng)", "Ctrl+S")) {
				if (!m_nodeGraphs.empty()) {
					if (m_nodeGraphs[m_activeGraph]->m_fileName.empty()) {
						const static char* FILTERS[] = { "*.tng\0" };
						const char* filePath = tinyfd_saveFileDialog(
							"Save",
							"",
							1,
							FILTERS,
							"Twist Node-Graph"
						);
						if (filePath) {
							m_nodeGraphs[m_activeGraph]->save(std::string(filePath));
						}
					} else {
						m_nodeGraphs[m_activeGraph]->save(m_nodeGraphs[m_activeGraph]->m_fileName);
					}
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ImGui::SetNextWindowSize(ImVec2(w, h-18), 0);
	ImGui::SetNextWindowPos(ImVec2(0, 18), 0);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	const ImGuiIO io = ImGui::GetIO();

	int flags = ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse |
				ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::Begin("", nullptr, flags)) {
		static float sz0 = 200;
		static float sz1 = w - 250;
		ImGui::Splitter(true, 3.0f, &sz0, &sz1, 100, 300);
		if (ImGui::BeginChild("side_bar", ImVec2(sz0, -1), true, flags)) {
			if (!m_nodeGraphs.empty()) {
				ImGui::BeginGroup();
				ImGui::Text("Controls");
				if (ImGui::Button(m_playing ? "Stop" : "Play") && !m_nodeGraphs.empty()) {
					m_playing = !m_playing;
				}
				ImGui::EndGroup();
				if (ImGui::CollapsingHeader("Properties")) {
					static const char* GRAPH_TYPES[] = {
						"Normal\0",
						"Module\0",
						0
					};
					ImGui::InputText(
						"Name##graphName",
						m_nodeGraphs[m_activeGraph]->m_name.data(),
						m_nodeGraphs[m_activeGraph]->m_name.size()
					);
					ImGui::Combo(
						"Type##graphType",
						(int*)&m_nodeGraphs[m_activeGraph]->m_type,
						GRAPH_TYPES,
						2
					);
				}
				if (ImGui::CollapsingHeader("Recording")) {
					if (ImGui::Button(m_recording ? "Stop" : "Record", ImVec2(ImGui::GetWindowWidth(), 20))) {
						m_recording = !m_recording;
					}
					if (!m_recording) {
						if (ImGui::Button("Save", ImVec2(ImGui::GetWindowWidth(), 20))) {
							const static char* F[] = { 
								"*.ogg\0"
							};
							const char* filePath = tinyfd_saveFileDialog(
								"Save Recording",
								"",
								1,
								F,
								"Audio File (*.ogg)"
							);
							if (filePath) {
								saveRecording(std::string(filePath));
							}
						}
						if (ImGui::DragFloat("Dur. (s)##maxdur", &m_recTime, 0.1f, 0.1f, 60.0f)) {
							m_recordBuffer.resize((int)(m_recTime * sampleRate));
							m_recordPointer = 0;
						}

						static const char* FADE_TYPES[] = {
							"None\0",
							"In\0",
							"Out\0",
							"In/Out\0",
							0
						};
						ImGui::DragFloat("Fade (s)", &m_recordingFadeTime, 0.1f, 0.0f, m_recTime);
						ImGui::Combo("Fade Type", &m_recordingFadeType, FADE_TYPES, 4);
					} else {
						int sec = (int)(float(m_recordPointer) / sampleRate) % 60;
						int minute = sec / 60;
						ImGui::Text("RECORDING... %02d:%02d", minute, sec);
					}
				}
				if (ImGui::CollapsingHeader("Nodes")) {
					std::vector<const char*> nodeNames;
					std::vector<int> nodeIDs;
					std::vector<ImVec4> nodeBounds;
					nodeNames.reserve(m_nodeGraphs[m_activeGraph]->nodes().size());
					nodeIDs.reserve(m_nodeGraphs[m_activeGraph]->nodes().size());
					nodeBounds.reserve(m_nodeGraphs[m_activeGraph]->nodes().size());
					for (auto& nodep : m_nodeGraphs[m_activeGraph]->nodes()) {
						nodeNames.push_back(nodep.second->m_type.c_str());
						nodeIDs.push_back(nodep.second->m_id);
						nodeBounds.push_back(nodep.second->m_bounds);
					}

					static int selectedNode = 0;
					ImGui::PushItemWidth(-1);
					if (ImGui::ListBox("##node_list", &selectedNode, nodeNames.data(), nodeNames.size())) {
						m_selectedNodes.clear();
						m_selectedNodes.push_back(nodeIDs[selectedNode]);
						m_nodeGraphs[m_activeGraph]->m_scrolling.x = -nodeBounds[selectedNode].x + m_mainWindowSize.x * 0.5f - nodeBounds[selectedNode].z * 0.5f;
						m_nodeGraphs[m_activeGraph]->m_scrolling.y = -nodeBounds[selectedNode].y + m_mainWindowSize.y * 0.5f - nodeBounds[selectedNode].w * 0.5f;
					}
					ImGui::PopItemWidth();
				}
				if (ImGui::CollapsingHeader("Sample Library")) {
					static int selectedSample = -1;
					auto items = m_nodeGraphs[m_activeGraph]->getSampleNames();
					ImGui::ListBox("##sampleLib", &selectedSample, items.data(), items.size(), 8);
					ImGui::SameLine();

					ImGui::BeginGroup();
					float w = ImGui::GetContentRegionAvailWidth();
					if (ImGui::Button("Load", ImVec2(w, 18))) {
						const static char* FILTERS[] = {
							"*.wav\0",
							"*.aif\0",
							"*.flac\0",
							"*.ogg\0"
						};
						const char* filePath = tinyfd_openFileDialog(
							"Load Sample",
							"",
							4,
							FILTERS,
							"Audio Files (*.wav; *.aif; *.flac; *.ogg)",
							0
						);
						if (filePath) {
							if (!m_nodeGraphs[m_activeGraph]->addSample(std::string(filePath))) {
								tinyfd_messageBox("Error", "Ivalid sample. It must have <= 10 seconds.", "ok", "error", 1);
							}
						}
					}
					if (ImGui::Button("Delete", ImVec2(w, 18))) {
						int sid = m_nodeGraphs[m_activeGraph]->getSampleID(std::string(items[selectedSample]));
						m_nodeGraphs[m_activeGraph]->removeSample(sid);
						selectedSample = -1;
					}
					ImGui::EndGroup();
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		if (ImGui::BeginChild("scrolling_region", ImVec2(-1, -1), true, flags) && !m_nodeGraphs.empty()) {
			ImGui::BeginTabBar("##main_tabs", ImGuiTabBarFlags_SizingPolicyFit);
			for (int i = 0; i < m_nodeGraphs.size(); i++) {
				std::stringstream stm;
				stm << m_nodeGraphs[i]->name();
				stm << "##";
				stm << i;

				int flags = !m_nodeGraphs[i]->m_saved ? ImGuiTabItemFlags_UnsavedDocument : 0;
				
				const bool wasOpen = m_nodeGraphs[i]->m_open;
				if (ImGui::TabItem(stm.str().c_str(), &m_nodeGraphs[i]->m_open, flags)) {
					m_activeGraph = i;
				}

				if (wasOpen && !m_nodeGraphs[i]->m_open) {
					if (m_nodeGraphs[i]->m_saved) {
						closeGraph(i);
						break;
					} else {
						int res = tinyfd_messageBox(
							"Warning!",
							"You have unsaved changes! Continue?",
							"yesno",
							"warning",
							0
						);
						if (res == 1) {
							closeGraph(i);
							break;
						}
						m_nodeGraphs[i]->m_open = true;
					}
				}
			}
			ImGui::EndTabBar();
			
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200));
			ImGui::BeginChild("scrolling_region_", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
			ImGui::PushItemWidth(120.0f);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			m_mainWindowSize = ImGui::GetWindowSize();
			drawNodeGraph(m_nodeGraphs[m_activeGraph].get());

			if (m_linking.active && ImGui::IsMouseReleased(0)) {
				m_linking.active = false;
				m_linking.node = nullptr;
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
}

void TNodeEditor::closeGraph(int id) {
	m_playing = false;
	m_recording = false;
	m_nodeGraphs[id]->m_solvedNodes.clear();
	m_nodeGraphs.erase(m_nodeGraphs.begin() + id);
	if (m_activeGraph > m_nodeGraphs.size()-1) {
		m_activeGraph = m_nodeGraphs.size()-1;
	}
}

TNodeGraph* TNodeEditor::newGraph() {
	std::unique_ptr<TNodeGraph> graph = std::unique_ptr<TNodeGraph>(new TNodeGraph());

	std::stringstream stm;
	stm << "Untitled Node Graph";
	stm << m_nodeGraphs.size();
	graph->m_name = stm.str();
	graph->m_editor = this;

	m_nodeGraphs.push_back(std::move(graph));
	return m_nodeGraphs.back().get();
}

float TNodeEditor::output() {
	if (m_loading) return 0.0f;

	float sample = 0.0f;
	if (m_playing || m_recording) {
		sample = m_nodeGraphs[m_activeGraph]->solve();
	}

	const float ATTACK_TIME  = 5.0f / 1000.0f;
	const float RELEASE_TIME = 200.0f / 1000.0f;

	float attack  = 1.0f - std::exp(-1.0f / (ATTACK_TIME * sampleRate));
	float release = 1.0f - std::exp(-1.0f / (RELEASE_TIME * sampleRate));

	m_signalDC = tmath::lerp(m_signalDC, sample, 0.5f / sampleRate);
	sample -= m_signalDC;

	float absSignal = std::abs(sample);
	if (absSignal > m_envelope) {
		m_envelope = tmath::lerp(m_envelope, absSignal, attack);
	} else {
		m_envelope = tmath::lerp(m_envelope, absSignal, release);
	}
	m_envelope = std::max(m_envelope, 1.0f);

	float finalOut = std::min(std::max((sample * 0.6f / (m_envelope+0.0001f)), -1.0f), 1.0f);

	if (m_recording) {
		const int fadeTime = int(m_recordingFadeTime * sampleRate);
		const float fadeDelta = 1.0f / fadeTime;
		switch (m_recordingFadeType) {
			case 0: m_recordingFade = 1.0f; break;
			case 1:
				if (m_recordPointer >= 0 && m_recordPointer < fadeTime)
					m_recordingFade += fadeDelta;
				break;
			case 2:
				if (m_recordPointer >= m_recordBuffer.size() - fadeTime)
					m_recordingFade -= fadeDelta;
				else
					m_recordingFade = 1.0f;
				break;
			case 3: {
				if (m_recordPointer >= 0 && m_recordPointer < fadeTime)
					m_recordingFade += fadeDelta;
				else if (m_recordPointer >= m_recordBuffer.size() - fadeTime)
					m_recordingFade -= fadeDelta;
				else
					m_recordingFade = 1.0f;
			} break;
		}
		m_recordBuffer[m_recordPointer++] = finalOut * m_recordingFade;
		if (m_recordPointer >= m_recordBuffer.size()) {
			m_recording = false;
			m_recordPointer = 0;
			m_recordingFade = 1;
		}
	}

	return finalOut;
}

void TNodeEditor::saveRecording(const std::string& fileName) {
	m_rendering = true;

	int fmt = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
	SndfileHandle snd = SndfileHandle(fileName, SFM_WRITE, fmt, 1, int(sampleRate));
	snd.writef(m_recordBuffer.data(), m_recordBuffer.size());

	m_rendering = false;
}

void TNodeEditor::renderToFile(const std::string& fileName, float time) {
	m_rendering = true;
	const int sampleCount = int(time * sampleRate);
	
	float data[sampleCount];
	for (int i = 0; i < sampleCount; i++) {
		data[i] = output();
	}

	int fmt = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
	SndfileHandle snd = SndfileHandle(fileName, SFM_WRITE, fmt, 1, int(sampleRate));
	snd.writef(data, sampleCount);

	m_rendering = false;
}

void midiCallback(double dt, std::vector<uint8_t>* message, void* userData) {
	unsigned int nBytes = message->size();
	for ( unsigned int i=0; i<nBytes; i++ )
		std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
	if ( nBytes > 0 )
		std::cout << "stamp = " << dt << std::endl;
}