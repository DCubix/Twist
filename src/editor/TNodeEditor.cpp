#include "TNodeEditor.h"

#include <iostream>
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

#include "tinyfiledialogs.h"
#include "tinywav.h"

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#define GET(type, v, d) (json[v].is_null() ? d : json[v].get<type>())
#define REGISTER_NODE(ntype) m_nodeFactories[ntype::type()] = [](JSON& json) -> TNode*

static std::vector<TLink*> getAllLinksRelatedToNode(const std::vector<TLink*>& lnks, TNode* node) {
	std::vector<TLink*> links;
	if (node == nullptr) return links;
	for (TLink* lnk : lnks) {
		auto pos = std::find(links.begin(), links.end(), lnk);
		if (lnk->inputID == node->id() || lnk->outputID == node->id()) {
			if (pos != links.end()) continue;
			links.push_back(lnk);
		}
	}
	return links;
}

TNodeEditor::TNodeEditor() {
	m_linking.active = 0;
	m_linking.inputID = 0;
	m_linking.inputSlot = 0;
	m_linking.node = nullptr;
	m_scrolling.x = 0;
	m_scrolling.y = 0;
	m_selectedNode = -1;
	m_nodeHoveredInList = -1;
	m_openContextMenu = false;
	m_bounds.x = 0;
	m_bounds.y = 0;
	m_bounds.z = 1;
	m_bounds.w = 1;
	m_oldFontWindowScale = 0;

	REGISTER_NODE(TValueNode) {
		return new TValueNode{ GET(float, "value", 0.0f) };
	};
	REGISTER_NODE(TMixNode) {
		return new TMixNode{ GET(float, "factor", 0.5f) };
	};
	REGISTER_NODE(TOscillatorNode) {
		return new TOscillatorNode{
			GET(float, "sampleRate", 44100),
			(TOsc::TWave) GET(int, "waveForm", 0),
			GET(float, "freq", 440),
			GET(float, "amp", 1)
		};
	};
	REGISTER_NODE(TNoteNode) {
		return new TNoteNode{
			(Notes) GET(int, "note", 0),
			GET(int, "oct", 0)
		};
	};
	REGISTER_NODE(TChorusNode) {
		return new TChorusNode{
			GET(float, "sampleRate", 44100),
			GET(float, "delayTime", 50),
			GET(float, "chorusRate", 1),
			GET(float, "chorusDepth", 1)
		};
	};
	REGISTER_NODE(TRemapNode) {
		return new TRemapNode{
			GET(float, "omin", 0),
			GET(float, "omax", 1),
			GET(float, "nmin", 0),
			GET(float, "nmax", 1)
		};
	};
	REGISTER_NODE(TSequencerNode) {
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
	};
	REGISTER_NODE(TButtonNode) {
		return new TButtonNode{
			GET(bool, "on", false)
		};
	};
	REGISTER_NODE(TFilterNode) {
		return new TFilterNode{
			GET(float, "sampleRate", 44100),
			GET(float, "cutOff", 20),
			(TFilterNode::TFilter) GET(int, "filter", 0)
		};
	};
	REGISTER_NODE(TMathNode) {
		return new TMathNode{
			(TMathNode::TMathNodeOp) GET(int, "op", 0),
			GET(float, "a", 0),
			GET(float, "b", 0)
		};
	};
	REGISTER_NODE(TADSRNode) {
		return new TADSRNode{
			GET(float, "sampleRate", 44100),
			GET(float, "a", 0),
			GET(float, "d", 0),
			GET(float, "s", 0),
			GET(float, "r", 0)
		};
	};
	REGISTER_NODE(TTimerNode) {
		return new TTimerNode{
			GET(float, "sampleRate", 44100),
			GET(float, "bpm", 120),
			GET(float, "swing", 0)
		};
	};
	REGISTER_NODE(TReverbNode) {
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
	};

	addNode(0, 0, new TOutNode());
	m_outputNode = (TOutNode*) m_nodes[0];
}

TNodeEditor::~TNodeEditor() {
	for (TNode* node : m_nodes) {
		delete node;
	}
	m_nodes.clear();
	for (TLink* link : m_links) {
		delete link;
	}
	m_links.clear();
}

void TNodeEditor::draw(int w, int h) {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				for (TNode* node : m_nodes) {
					delete node;
				}
				m_nodes.clear();

				for (TLink* link : m_links) {
					delete link;
				}
				m_links.clear();
				m_solvedNodes.clear();

				addNode(0, 0, new TOutNode());
				m_outputNode = (TOutNode*) m_nodes[0];
				m_scrolling.x = 0;
				m_scrolling.y = 0;
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
					loadTNG(std::string(filePath));
				}
			}
			if (ImGui::MenuItem("Save (*.tng)", "Ctrl+S")) {
				const static char* FILTERS[] = { "*.tng\0" };
				const char* filePath = tinyfd_saveFileDialog(
					"Save",
					"",
					1,
					FILTERS,
					"Twist Node-Graph"
				);
				if (filePath) {
					saveTNG(std::string(filePath));
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Render (*.wav)")) {
				const static char* F[] = { "*.wav\0" };
				const char* filePath = tinyfd_saveFileDialog(
					"Render",
					"",
					1,
					F,
					"WAV File"
				);
				if (filePath) {
					renderToFile(std::string(filePath), m_outDuration);
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	ImGui::SetNextWindowSize(ImVec2(w, h-20), 0);
	ImGui::SetNextWindowPos(ImVec2(0, 20), 0);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	const ImGuiIO io = ImGui::GetIO();

	int flags = ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::Begin("", nullptr, flags)) {
		if (ImGui::BeginChild("scrolling_region", ImVec2(0,0), false, flags))  {
			// Zooming
			ImGuiContext& g = *GImGui;
			ImGuiWindow* window = ImGui::GetCurrentWindow();

			float oldFontScaleToReset = g.Font->Scale; // We'll clean up at the bottom
			float fontScaleStored = m_oldFontWindowScale ? m_oldFontWindowScale : oldFontScaleToReset;
			float& fontScaleToTrack = g.Font->Scale;

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
			// 			m_scrolling = m_scrolling * scale;
			// 			// Set the correct font scale (3 lines), and store it
			// 			fontScaleStored = fontScaleToTrack = new_font_scale;
			// 			g.FontBaseSize = io.FontGlobalScale * g.Font->Scale * g.Font->FontSize;
			// 			g.FontSize = window->CalcFontSize();
			// 		}
			// 	}
			// }

			// fixes zooming just a bit
			bool nodesHaveZeroSize = false;
			const float currentFontWindowScale = !io.FontAllowUserScaling ? fontScaleStored : ImGui::GetCurrentWindow()->FontWindowScale;
			if (m_oldFontWindowScale == 0.0f) {
				m_oldFontWindowScale = currentFontWindowScale;
				nodesHaveZeroSize = true;   // at start or after clear()
				m_scrolling = ImGui::GetWindowSize() * 0.5f;
			} else if (m_oldFontWindowScale!=currentFontWindowScale) {
				nodesHaveZeroSize = true;
				for (TNode* node : m_nodes)    {
					node->m_bounds.z = 0.0f;  // we must reset the size
					node->m_bounds.w = 0.0f;  // we must reset the size
				}
				// These two lines makes the scaling work around the mouse position AFAICS
				//if (io.FontAllowUserScaling) {
					const ImVec2 delta = (io.MousePos - ImGui::GetCursorScreenPos());
					m_scrolling -= (delta * currentFontWindowScale - delta * m_oldFontWindowScale) / currentFontWindowScale;
				//}
				m_oldFontWindowScale = currentFontWindowScale;
			}

			bool openContext = false;

			const float NODE_SLOT_RADIUS = 6.0f * currentFontWindowScale;
			const float NODE_SLOT_RADIUS2 = 8.0f * currentFontWindowScale;
			const float linkThick = 5.0f * currentFontWindowScale;
			const float nodeRounding = 8.0f * currentFontWindowScale;
			const ImVec2 NODE_WINDOW_PADDING(8.0f * currentFontWindowScale, 8.0f * currentFontWindowScale);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200));
			ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
			ImGui::PushItemWidth(120.0f);

			ImVec2 offset = ImGui::GetCursorScreenPos() + m_scrolling;
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// Display grid
			ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
			float GRID_SZ = 64.0f * currentFontWindowScale;
			ImVec2 win_pos = ImGui::GetCursorScreenPos();
			ImVec2 canvas_sz = ImGui::GetWindowSize();
			for (float x = fmodf(m_scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
				draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
			for (float y = fmodf(m_scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
				draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);

			// Display links
			draw_list->ChannelsSplit(2);
			draw_list->ChannelsSetCurrent(0); // Background
			for (TLink* link : m_links) {
				TNode* ni = getNode(link->inputID);
				TNode* no = getNode(link->outputID);
				if (ni == nullptr || no == nullptr) continue;

				ImVec2 p1 = offset + ni->outputSlotPos(link->inputSlot, currentFontWindowScale);
				ImVec2 p2 = offset + no->inputSlotPos(link->outputSlot, currentFontWindowScale);

				draw_list->AddCircleFilled(p1, NODE_SLOT_RADIUS2, IM_COL32(200, 200, 100, 255));
				draw_list->AddCircleFilled(p2, NODE_SLOT_RADIUS2, IM_COL32(200, 200, 100, 255));
				draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, IM_COL32(200, 200, 100, 255), linkThick);
			}

			// Display nodes
			for (TNode* node : m_nodes) {
				const ImVec2 nodeTitleBarButtonsStartCursor = node->open ? ImGui::GetCursorPos() : ImVec2(0,0);

				ImGui::PushID(node->id());
				ImVec2 node_rect_min = offset + ImVec2(node->m_bounds.x, node->m_bounds.y) * currentFontWindowScale;

				// Display node contents first
				draw_list->ChannelsSetCurrent(1); // Foreground
				bool old_any_active = ImGui::IsAnyItemActive();

				ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
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
					if (node != m_outputNode) {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75,0.75,0.75,0.5));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75,0.75,0.75,0.77));
						ImGui::SameLine();

						static const ImVec2 vec2zero(0, 0);
						
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2zero);
						ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, vec2zero);
						if (ImGui::SmallButton("X")) {
							deleteNode(node);
							m_selectedNode = -1;
							m_hoveredNode = -1;
							m_nodeHoveredInList = -1;
							break;
						}
						ImGui::PopStyleVar(2);
						ImGui::PopStyleColor(3);
					}

					if (node->open) {
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::BeginGroup();
							node->gui();
						ImGui::EndGroup();
					}
				ImGui::EndGroup();

				// Save the size of what we have emitted and whether any of the widgets are being used
				bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
				ImVec2 nodeSize = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
				node->m_bounds.z = nodeSize.x;
				node->m_bounds.w = nodeSize.y;
				ImVec2 node_rect_max = node_rect_min + node->size();
				
				// Display node box
				draw_list->ChannelsSetCurrent(0); // Background
				ImGui::SetCursorScreenPos(node_rect_min);
				ImGui::InvisibleButton("node##invbtn", node->size());
				if (ImGui::IsItemHovered()) {
					m_hoveredNode = node->id();
					openContext |= ImGui::IsMouseClicked(1);
				}

				bool node_moving_active = ImGui::IsItemActive();
				if (node_widgets_active || node_moving_active)
					m_selectedNode = node->id();

				if (node_moving_active && ImGui::IsMouseDragging(0) && !m_linking.active) {
					node->m_bounds.x = node->m_bounds.x + ImGui::GetIO().MouseDelta.x/currentFontWindowScale;
					node->m_bounds.y = node->m_bounds.y + ImGui::GetIO().MouseDelta.y/currentFontWindowScale;
				}

				ImU32 node_bg_color = (m_nodeHoveredInList == node->id() || m_hoveredNode == node->id() || (m_nodeHoveredInList == -1 && m_selectedNode == node->id())) ? IM_COL32(75, 75, 75, 255) : IM_COL32(60, 60, 60, 255);
				draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, nodeRounding);
				draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), nodeRounding);
				
				for (int i = 0; i < node->inputs().size(); i++) {
					const char* label = node->inputs()[i].label.c_str();
					ImVec2 pos = offset + node->inputSlotPos(i, currentFontWindowScale);
					ImVec2 tsz = ImGui::CalcTextSize(label);
					ImVec2 off = ImVec2(-(tsz.x + NODE_SLOT_RADIUS + 3), -tsz.y * 0.5f);
					ImVec2 off1 = ImVec2(-(tsz.x + NODE_SLOT_RADIUS + 3), -tsz.y * 0.5f + 1);

					draw_list->AddText(pos + off1, IM_COL32(0, 0, 0, 220), label);
					draw_list->AddText(pos + off, IM_COL32(255, 255, 255, 220), label);
					draw_list->AddCircleFilled(pos, NODE_SLOT_RADIUS, IM_COL32(150, 200, 150, 210));

					// Unlink
					ImVec2 hsz(NODE_SLOT_RADIUS, NODE_SLOT_RADIUS);
					if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) &&
						ImGui::IsMouseDoubleClicked(0) &&
						!m_linking.active)
					{
						int eid = -1;
						int j = 0;
						for (TLink* link : m_links) {
							if (link->outputID == node->id() && link->outputSlot == i) {
								eid = j;
								break;
							}
							j++;
						}
						if (eid != -1) {
							TLink* lnk = m_links[eid];
							getNode(lnk->outputID)->inputs()[lnk->outputSlot].connected = false;
							m_links.erase(m_links.begin() + eid);
							delete lnk;
						}
					}

					// End linking
					if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) &&
						ImGui::IsMouseReleased(0) &&
						m_linking.active && m_linking.node != node)
					{
						m_linking.active = false;
						link(m_linking.inputID, m_linking.inputSlot, node->id(), i);
					}
				}
				
				for (int i = 0; i < node->outputs().size(); i++) {
					const char* label = node->outputs()[i].label.c_str();
					ImVec2 pos = offset + node->outputSlotPos(i, currentFontWindowScale);
					ImVec2 tsz = ImGui::CalcTextSize(label);
					ImVec2 off = ImVec2(NODE_SLOT_RADIUS + 3, -tsz.y * 0.5f);
					ImVec2 off1 = ImVec2(NODE_SLOT_RADIUS + 3, -tsz.y * 0.5f + 1);

					draw_list->AddText(pos + off1, IM_COL32(0, 0, 0, 220), label);
					draw_list->AddText(pos + off, IM_COL32(255, 255, 255, 220), label);
					draw_list->AddCircleFilled(pos, NODE_SLOT_RADIUS, IM_COL32(200, 150, 150, 210));

					/// Start linking process
					ImVec2 hsz(NODE_SLOT_RADIUS, NODE_SLOT_RADIUS);
					if (ImGui::IsMouseHoveringRect(pos - hsz, pos + hsz) && ImGui::IsMouseClicked(0)) {
						m_linking.active = true;
						m_linking.node = node;
						m_linking.inputID = node->id();
						m_linking.inputSlot = i;

					}

					if (m_linking.active && m_linking.node == node && m_linking.inputSlot == i) {
						ImVec2 l0 = pos;
						ImVec2 l1 = ImGui::GetIO().MousePos;
						draw_list->AddBezierCurve(l0, l0 + ImVec2(+50, 0), l1 + ImVec2(-50, 0), l1, IM_COL32(200, 200, 100, 255), 5.0f);
					}
				}

				ImGui::PopID();
			}
			draw_list->ChannelsMerge();

			if (m_linking.active && ImGui::IsMouseReleased(0)) {
				m_linking.active = false;
				m_linking.node = nullptr;
			}

			// Open context menu
			if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1)) {
				m_selectedNode = m_nodeHoveredInList = m_hoveredNode = -1;
				openContext = true;
			}
			if (openContext) {
				ImGui::OpenPopup("context_menu");
				if (m_nodeHoveredInList != -1)
					m_selectedNode = m_nodeHoveredInList;
				if (m_hoveredNode != -1)
					m_selectedNode = m_hoveredNode;
			}

			// Draw context menu
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
			if (ImGui::BeginPopup("context_menu")) {
				TNode* node = m_selectedNode != -1 ? getNode(m_selectedNode) : nullptr;
				ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
				if (node && node != m_outputNode) {
					ImGui::Text("%s Node", node->m_title.c_str());
					ImGui::Separator();
					if (ImGui::MenuItem("Delete", NULL, false, false)) {}
					if (ImGui::MenuItem("Duplicate", NULL, false, false)) {}
				} else {
					JSON js;
					for (auto fn : m_nodeFactories) {
						if (ImGui::MenuItem(fn.first.c_str())) {
							addNode(scene_pos.x, scene_pos.y, fn.second(js));
						}
					}
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			// Scrolling
			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
				m_scrolling = m_scrolling + ImGui::GetIO().MouseDelta;

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

void TNodeEditor::addNode(int x, int y, TNode* node) {
	if (node == nullptr) return;
	node->m_id = m_nodes.size();
	node->m_bounds.x = x;
	node->m_bounds.y = y;
	m_nodes.push_back(node);
}

void TNodeEditor::deleteNode(TNode* node) {
	for (TLink* link : getAllLinksRelatedToNode(m_links, node)) {
		m_links.erase(std::find(m_links.begin(), m_links.end(), link));
		delete link;
	}

	auto pos = std::find(m_nodes.begin(), m_nodes.end(), node);
	if (pos != m_nodes.end()) {
		m_nodes.erase(pos);
	}
	delete node;
	node = nullptr;

	solveNodes();
}

void TNodeEditor::link(int inID, int inSlot, int outID, int outSlot) {
	TLink* link = new TLink();
	link->inputID = inID;
	link->inputSlot = inSlot;
	link->outputID = outID;
	link->outputSlot = outSlot;

	getNode(outID)->inputs()[outSlot].connected = true;

	m_links.push_back(link);

	solveNodes();
}

std::vector<TLink*> TNodeEditor::getNodeLinks(TNode* node) {
	std::vector<TLink*> links;
	for (TLink* lnk : m_links) {
		if (lnk->inputID == node->id()) {
			links.push_back(lnk);
		}
	}
	return links;
}

std::vector<TNode*> TNodeEditor::getNodeInputs(TNode* node) {
	std::vector<TNode*> ins;
	for (TLink* lnk : m_links) {
		TNode* nd = getNode(lnk->inputID);
		if (lnk->outputID == node->id() && nd != nullptr) {
			ins.push_back(nd);
		}
	}
	return ins;
}

void TNodeEditor::solveNodes() {
	m_solvedNodes = buildNodes(m_outputNode);
}

std::vector<TNode*> TNodeEditor::buildNodes(TNode* out) {
	if (out == nullptr)
		return std::vector<TNode*>();

	std::vector<TNode*> nodes;
	nodes.push_back(out);

	for (TNode* in : getNodeInputs(out)) {
		if (in == nullptr) continue;
		nodes.push_back(in);

		std::vector<TNode*> rec = buildNodes(in);
		for (TNode* rnd : rec) {
			if (rnd == nullptr) continue;
			if (std::find(nodes.begin(), nodes.end(), rnd) == nodes.end()) {
				nodes.push_back(rnd);
			}
		}
	}

	std::reverse(nodes.begin(), nodes.end());

	return nodes;
}

TNode* TNodeEditor::getNode(int id) {
	for (TNode* node : m_nodes) {
		if (node->id() == id) {
			return node;
		}
	}
	return nullptr;
}

void TNodeEditor::solve() {
	if (m_loading) return;

	for (TNode* node : m_solvedNodes) {
		if (node == nullptr) continue;
		node->solve();
		for (TLink* link : getNodeLinks(node)) {
			TNode* tgt = m_nodes[link->outputID];
			tgt->setInput(link->outputSlot, node->getOutput(link->inputSlot));
		}
	}
}

float TNodeEditor::output() {
	if (m_loading) return 0.0f;

	solve();

	const float ATTACK_TIME  = 5.0f / 1000.0f;
	const float RELEASE_TIME = 200.0f / 1000.0f;

	float attack  = 1.0f - std::exp(-1.0f / (ATTACK_TIME * sampleRate));
	float release = 1.0f - std::exp(-1.0f / (RELEASE_TIME * sampleRate));

	float sample = m_outputNode->getInput(0);
	m_signalDC = tmath::lerp(m_signalDC, sample, 0.5f / sampleRate);
	sample -= m_signalDC;

	float absSignal = std::abs(sample);
	if (absSignal > m_envelope) {
		m_envelope = tmath::lerp(m_envelope, absSignal, attack);
	} else {
		m_envelope = tmath::lerp(m_envelope, absSignal, release);
	}
	m_envelope = std::max(m_envelope, 1.0f);

	return std::min(std::max((sample * 0.6f / m_envelope), -1.0f), 1.0f) * m_outputNode->volume;
}

void TNodeEditor::renderToFile(const std::string& fileName, float time) {
	m_rendering = true;
	const int sampleCount = int(time * 44100.0f);
	TinyWav tw;
	tinywav_open_write(&tw,
		1,
		44100,
		TW_FLOAT32, // the output samples will be 32-bit floats. TW_INT16 is also supported
		TW_INLINE,  // the samples will be presented inlined in a single buffer.
					// Other options include TW_INTERLEAVED and TW_SPLIT
		fileName.c_str() // the output path
	);

	float data[sampleCount];
	for (int i = 0; i < sampleCount; i++) {
		data[i] = output();
	}

	tinywav_write_f(&tw, data, sampleCount);
	tinywav_close_write(&tw);
	m_rendering = false;
}

void TNodeEditor::saveTNG(const std::string& fileName) {
	std::ofstream fp(fileName);
	JSON json;
	json["outPos"] = { m_outputNode->bounds().x, m_outputNode->bounds().y };

	int i = 0;
	for (int j = 1; j < m_nodes.size(); j++) {
		TNode* node = m_nodes[j];
		node->save(json["nodes"][i]);
		i++;
	}

	i = 0;
	for (TLink* link : m_links) {
		if (link == nullptr) continue;
		JSON& links = json["links"][i];
		links["inID"] = link->inputID;
		links["inSlot"] = link->inputSlot;
		links["outID"] = link->outputID;
		links["outSlot"] = link->outputSlot;
		i++;
	}
	fp << json.dump(1, '\t', true);
	fp.close();
}

void TNodeEditor::loadTNG(const std::string& fileName) {
	m_loading = true;

	JSON json;
	std::ifstream fp(fileName);
	fp >> json;
	fp.close();

	for (TNode* node : m_nodes) {
		delete node;
	}
	m_nodes.clear();

	for (TLink* link : m_links) {
		delete link;
	}
	m_links.clear();
	m_solvedNodes.clear();

	addNode(json["outPos"][0], json["outPos"][1], new TOutNode());
	m_outputNode = (TOutNode*) m_nodes[0];

	for (int i = 0; i < json["nodes"].size(); i++) {
		JSON& node = json["nodes"][i];
		TNode* nd = m_nodeFactories[node["type"]](node);
		addNode(node["pos"][0], node["pos"][1], nd);
		if (node["id"].is_number_integer())
			nd->m_id = node["id"];
	}

	for (int i = 0; i < json["links"].size(); i++) {
		JSON lnk = json["links"][i];
		if (lnk.is_null()) continue;
		link(lnk["inID"], lnk["inSlot"], lnk["outID"], lnk["outSlot"]);
	}

	solveNodes();

	m_loading = false;
}