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
		if (lnk->inputID == node->id() || lnk->outputID == node->id()) {
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
		TSequencerNode* seq = new TSequencerNode{
			GET(float, "sampleRate", 44100),
			GET(float, "bpm", 120),
			GET(float, "swing", 0)
		};
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
			(TMathNode::TMathNodeOp) GET(int, "op", 0)
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

	addNode(10, 10, new TOutNode());
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

// void TNodeEditor::draw(NkContext* ctx) {
// 	const struct nk_input *in = &ctx->input;

// 	if (nk_begin(ctx, "Node Editor", nk_rect(0, 0, 1024, 640), NK_WINDOW_NO_SCROLLBAR)) {
// 		nk_menubar_begin(ctx);
// 		nk_layout_row_dynamic(ctx, 22, 4);
// 		if (nk_menu_begin_label(ctx, "File", NK_TEXT_ALIGN_LEFT, nk_vec2(200, 200))) {
// 			nk_layout_row_dynamic(ctx, 22, 1);
// 			const static char* FILTERS[] = { "*.tng\0" };
// 			if (nk_menu_item_label(ctx, "Save node graph (*.tng)", NK_TEXT_ALIGN_LEFT)) {
// 				const char* filePath = tinyfd_saveFileDialog(
// 					"Save node graph",
// 					"",
// 					1,
// 					FILTERS,
// 					"Twist Node-Graph"
// 				);
// 				if (filePath) {
// 					saveTNG(std::string(filePath));
// 				}
// 			}
// 			if (nk_menu_item_label(ctx, "Load node graph (*.tng)", NK_TEXT_ALIGN_LEFT)) {
// 				const char* filePath = tinyfd_openFileDialog(
// 					"Load node graph",
// 					"",
// 					1,
// 					FILTERS,
// 					"Twist Node-Graph",
// 					0
// 				);
// 				if (filePath) {
// 					loadTNG(std::string(filePath));
// 				}
// 			}
// 			if (nk_menu_item_label(ctx, "Render audio (*.wav)", NK_TEXT_ALIGN_LEFT)) {
// 				const static char* F[] = { "*.wav\0" };
// 				const char* filePath = tinyfd_saveFileDialog(
// 					"Render to file",
// 					"",
// 					1,
// 					F,
// 					"WAV File"
// 				);
// 				if (filePath) {
// 					renderToFile(std::string(filePath), m_outDuration);
// 				}
// 			}
// 			nk_menu_end(ctx);
// 		}
// 		m_outDuration = nk_propertyf(ctx, "#Out Duration", 0.1f, m_outDuration, 60.0f, 0.1f, 0.1f);
// 		nk_menubar_end(ctx);

// 		nk_layout_row_dynamic(ctx, 24, 1);

// 		NkRect totalSpace = nk_window_get_content_region(ctx);
// 		NkCanvas* canvas = nk_window_get_canvas(ctx);

// 		nk_layout_space_begin(ctx, NK_STATIC, totalSpace.h, m_nodes.size());
// 		{
// 			/// Draw links
// 			for (TLink* link : m_links) {
// 				TNode* ni = getNode(link->inputID);
// 				TNode* no = getNode(link->outputID);
// 				if (ni == nullptr || no == nullptr) continue;

// 				float spacei = ni->m_bounds.h / float(ni->outputs().size() + 1);
// 				float spaceo = no->m_bounds.h / float(no->inputs().size() + 1);
// 				NkVec2 l0 = nk_layout_space_to_screen(ctx,
// 					nk_vec2(
// 						ni->m_bounds.x + ni->m_bounds.w,
// 						ni->m_bounds.y + 5.0f + spacei * (link->inputSlot + 1)
// 					)
// 				);
// 				NkVec2 l1 = nk_layout_space_to_screen(ctx,
// 					nk_vec2(
// 						no->m_bounds.x,
// 						no->m_bounds.y + 5.0f + spaceo * (link->outputSlot + 1)
// 					)
// 				);

// 				l0.x -= m_scrolling.x;
// 				l0.y -= m_scrolling.y;
// 				l1.x -= m_scrolling.x;
// 				l1.y -= m_scrolling.y;
// 				nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
// 					l1.x - 50.0f, l1.y, l1.x, l1.y, 3.0f, nk_rgb(100, 100, 100));
// 			}

// 			NkRect size = nk_layout_space_bounds(ctx);
// 			struct nk_panel *panel = nullptr;

// 			float x, y;
// 			const float gridSize = 48.0f;
// 			const NkColor gridColor = nk_rgb(60, 60, 60);
// 			for (x = std::fmod(size.x - m_scrolling.x, gridSize); x < size.w; x += gridSize)
// 				nk_stroke_line(canvas, x+size.x, size.y, x+size.x, size.y+size.h, 1.0f, gridColor);
// 			for (y = std::fmod(size.y - m_scrolling.y, gridSize); y < size.h; y += gridSize)
// 				nk_stroke_line(canvas, size.x, y+size.y, size.x+size.w, y+size.y, 1.0f, gridColor);

// 			for (TNode* node : m_nodes) {
// 				if (node == nullptr) continue;
				
// 				nk_layout_space_push(ctx,
// 						nk_rect(
// 							node->m_bounds.x - m_scrolling.x,
// 							node->m_bounds.y - m_scrolling.y,
// 							node->m_bounds.w, node->m_bounds.h
// 						)
// 				);
				
// 				int flags = NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE;
// 				if (nk_group_begin(ctx, node->m_title.c_str(), flags)) {
// 					panel = nk_window_get_panel(ctx);

// 					if (node != m_outputNode) {
// 						nk_layout_row_dynamic(ctx, 12, 1);
// 						if (nk_button_label(ctx, "Delete")) {
// 							deleteNode(node);
// 							m_selected = nullptr;
// 							break;
// 						}
// 					}
// 					// Node content
// 					node->draw(ctx, canvas);

// 					nk_group_end(ctx);
// 				} else { continue; }

// 				if (panel == nullptr) continue;

// 				/// Connection and Linking
// 				float space;
// 				NkRect bounds = nk_layout_space_rect_to_local(ctx, panel->bounds);
// 				bounds.x += m_scrolling.x;
// 				bounds.y += m_scrolling.y;
// 				node->m_bounds = bounds;

// 				const struct nk_color TEXT = nk_rgb(180, 180, 180);
// 				const struct nk_color TEXT_S = nk_rgb(50, 50, 50);

// 				/// Outputs
// 				space = panel->bounds.h / float(node->outputs().size() + 1);
// 				for (int i = 0; i < node->outputs().size(); i++) {
// 					NkRect circle;
// 					circle.x = panel->bounds.x + panel->bounds.w - 5;
// 					circle.y = panel->bounds.y + space * (i + 1);
// 					circle.w = 10; circle.h = 10;
// 					nk_fill_circle(canvas, circle, nk_rgb(170, 100, 100));

// 					const struct nk_style* style = &ctx->style;
// 					nk_draw_text(
// 						canvas,
// 						nk_rect(circle.x + 12, circle.y+1, 100, 12),
// 						node->outputs()[i].label.c_str(),
// 						node->outputs()[i].label.size(),
// 						style->font,
// 						TEXT_S, TEXT_S
// 					);
// 					nk_draw_text(
// 						canvas,
// 						nk_rect(circle.x + 12, circle.y, 100, 12),
// 						node->outputs()[i].label.c_str(),
// 						node->outputs()[i].label.size(),
// 						style->font,
// 						TEXT, TEXT
// 					);

// 					/// Start linking process
// 					if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true)) {
// 						m_linking.active = true;
// 						m_linking.node = node;
// 						m_linking.inputID = node->id();
// 						m_linking.inputSlot = i;
// 					}

// 					/// Draw curve from linked node slot to mouse position
// 					if (m_linking.active && m_linking.node == node && m_linking.inputSlot == i) {
// 						NkVec2 l0 = nk_vec2(circle.x + 5, circle.y + 5);
// 						NkVec2 l1 = in->mouse.pos;
// 						nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
// 							l1.x - 50.0f, l1.y, l1.x, l1.y, 3.0f, nk_rgb(120, 120, 120));
// 					}
// 				}

// 				/// Inputs
// 				space = panel->bounds.h / float(node->inputs().size() + 1);
// 				for (int i = 0; i < node->inputs().size(); i++) {
// 					NkRect circle;
// 					circle.x = panel->bounds.x - 5;
// 					circle.y = panel->bounds.y + space * (i + 1);
// 					circle.w = 10; circle.h = 10;
// 					nk_fill_circle(canvas, circle, nk_rgb(100, 170, 100));

// 					const char* text = node->inputs()[i].label.c_str();
// 					int len = node->inputs()[i].label.size();

// 					const struct nk_style* style = &ctx->style;
// 					const struct nk_user_font* f = style->font;
					
// 					int tw = f->width(f->userdata, f->height, text, len);
// 						tw += (2.0f);

// 					nk_draw_text(
// 						canvas,
// 						nk_rect(circle.x - tw, circle.y+1, 100, 12),
// 						text, len,
// 						style->font,
// 						TEXT_S, TEXT_S
// 					);
// 					nk_draw_text(
// 						canvas,
// 						nk_rect(circle.x - tw, circle.y, 100, 12),
// 						text, len,
// 						style->font,
// 						TEXT, TEXT
// 					);

// 					/// Unlink
// 					if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
// 						nk_input_is_mouse_hovering_rect(in, circle) &&
// 						!m_linking.active)
// 					{
// 						int eid = -1;
// 						int j = 0;
// 						for (TLink* link : m_links) {
// 							if (link->outputID == node->id() && link->outputSlot == i) {
// 								eid = j;
// 								break;
// 							}
// 							j++;
// 						}
// 						if (eid != -1) {
// 							TLink* lnk = m_links[eid];
// 							getNode(lnk->outputID)->inputs()[lnk->outputSlot].connected = false;
// 							m_links.erase(m_links.begin() + eid);
// 							delete lnk;
// 						}
// 					}

// 					if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
// 						nk_input_is_mouse_hovering_rect(in, circle) &&
// 						m_linking.active && m_linking.node != node)
// 					{
// 						m_linking.active = false;
// 						link(m_linking.inputID, m_linking.inputSlot, node->id(), i);
// 					}
// 				}
// 			}

// 			/// Reset linking connection
// 			if (m_linking.active && nk_input_is_mouse_released(in, NK_BUTTON_LEFT)) {
// 				m_linking.active = false;
// 				m_linking.node = nullptr;
// 			}

// 			/// Selection
// 			if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nk_layout_space_bounds(ctx))) {
// 				m_selected = nullptr;
// 				for (TNode* node : m_nodes) {
// 					if (node == nullptr) continue;
// 					NkRect b = nk_layout_space_rect_to_screen(ctx, node->m_bounds);
// 					b.x -= m_scrolling.x;
// 					b.y -= m_scrolling.y;
// 					if (nk_input_is_mouse_hovering_rect(in, b))
// 						m_selected = node;
// 				}
// 			}

// 			// Add menu
// 			if (nk_contextual_begin(ctx, 0, nk_vec2(100, 500), nk_window_get_bounds(ctx))) {
// 				nk_layout_row_dynamic(ctx, 24, 1);
// 				int x = in->mouse.pos.x + m_scrolling.x;
// 				int y = in->mouse.pos.y + m_scrolling.y;
// 				JSON djson;
// 				for (auto e : m_nodeFactories) {
// 					if (nk_menu_item_label(ctx, e.first.c_str(), NK_TEXT_ALIGN_LEFT)) {
// 						addNode(x, y, e.second(djson));
// 					}
// 				}
// 				nk_contextual_end(ctx);
// 			}
// 		}
// 		nk_layout_space_end(ctx);

// 		/// Window content scrolling
// 		if (nk_input_is_mouse_hovering_rect(in, nk_window_get_bounds(ctx)) &&
// 			nk_input_is_mouse_down(in, NK_BUTTON_MIDDLE)) {
// 			m_scrolling.x -= in->mouse.delta.x;
// 			m_scrolling.y -= in->mouse.delta.y;
// 		}
// 	}
// 	nk_end(ctx);
// }

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

	addNode(json["outPos"][0], json["outPos"][1], new TOutNode());
	m_outputNode = (TOutNode*) m_nodes[0];

	for (int i = 0; i < json["nodes"].size(); i++) {
		JSON& node = json["nodes"][i];
		TNode* nd = m_nodeFactories[node["type"]](node);
		addNode(node["pos"][0], node["pos"][1], nd);
	}

	for (int i = 0; i < json["links"].size(); i++) {
		JSON lnk = json["links"][i];
		if (lnk.is_null()) continue;
		link(lnk["inID"], lnk["inSlot"], lnk["outID"], lnk["outSlot"]);
	}
}