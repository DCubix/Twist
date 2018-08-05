#include "TNode.h"

#include <iostream>
#include <cmath>
#include <algorithm>

float lerp(float a, float b, float t) {
	return (1.0f - t) * a + b * t;
}

float remap(float value, float from1, float to1, float from2, float to2) {
	return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

TNode::~TNode() {

}

void TNode::addInput(const std::string& label) {
	m_inputs.push_back(TValue(label));
}

void TNode::addOutput(const std::string& label) {
	m_outputs.push_back(TValue(label));
}

void TNode::draw(NkContext* ctx, NkCanvas* canvas) {
	_gui(ctx, canvas);
}

void TNodeEditor::draw(NkContext* ctx) {
	const struct nk_input *in = &ctx->input;

	if (nk_begin(ctx, "Node Editor", nk_rect(0, 0, 1024, 640), NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, 24, 1);

		NkRect totalSpace = nk_window_get_content_region(ctx);
		NkCanvas* canvas = nk_window_get_canvas(ctx);

		nk_layout_space_begin(ctx, NK_STATIC, totalSpace.h, m_nodes.size());
		{
			/// Draw links
			for (TLink* link : m_links) {
				TNode* ni = m_nodes[link->inputID];
				TNode* no = m_nodes[link->outputID];
				float spacei = ni->m_bounds.h / float(ni->outputs().size() + 1);
				float spaceo = no->m_bounds.h / float(no->inputs().size() + 1);
				NkVec2 l0 = nk_layout_space_to_screen(ctx,
					nk_vec2(
						ni->m_bounds.x + ni->m_bounds.w,
						ni->m_bounds.y + 5.0f + spacei * (link->inputSlot + 1)
					)
				);
				NkVec2 l1 = nk_layout_space_to_screen(ctx,
					nk_vec2(
						no->m_bounds.x,
						no->m_bounds.y + 5.0f + spaceo * (link->outputSlot + 1)
					)
				);

				l0.x -= m_scrolling.x;
				l0.y -= m_scrolling.y;
				l1.x -= m_scrolling.x;
				l1.y -= m_scrolling.y;
				nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
					l1.x - 50.0f, l1.y, l1.x, l1.y, 3.0f, nk_rgb(100, 100, 100));
			}

			NkRect size = nk_layout_space_bounds(ctx);
			struct nk_panel *panel = nullptr;

			float x, y;
			const float gridSize = 48.0f;
			const NkColor gridColor = nk_rgb(60, 60, 60);
			for (x = std::fmod(size.x - m_scrolling.x, gridSize); x < size.w; x += gridSize)
				nk_stroke_line(canvas, x+size.x, size.y, x+size.x, size.y+size.h, 1.0f, gridColor);
			for (y = std::fmod(size.y - m_scrolling.y, gridSize); y < size.h; y += gridSize)
				nk_stroke_line(canvas, size.x, y+size.y, size.x+size.w, y+size.y, 1.0f, gridColor);

			for (TNode* node : m_nodes) {
				if (node == nullptr) continue;
				
				nk_layout_space_push(ctx,
						nk_rect(
							node->m_bounds.x - m_scrolling.x,
							node->m_bounds.y - m_scrolling.y,
							node->m_bounds.w, node->m_bounds.h
						)
				);
				
				int flags = NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE;
				if (nk_group_begin(ctx, node->m_title.c_str(), flags)) {
					panel = nk_window_get_panel(ctx);

					if (node != m_outputNode) {
						nk_layout_row_dynamic(ctx, 12, 1);
						if (nk_button_label(ctx, "Delete")) {
							deleteNode(node);
							m_selected = nullptr;
							break;
						}
					}
					// Node content
					node->draw(ctx, canvas);

					nk_group_end(ctx);
				}

				if (panel == nullptr) continue;

				/// Connection and Linking
				float space;
				NkRect bounds = nk_layout_space_rect_to_local(ctx, panel->bounds);
				bounds.x += m_scrolling.x;
				bounds.y += m_scrolling.y;
				node->m_bounds = bounds;

				const struct nk_color TEXT = nk_rgb(180, 180, 180);
				const struct nk_color TEXT_S = nk_rgb(50, 50, 50);

				/// Outputs
				space = panel->bounds.h / float(node->outputs().size() + 1);
				for (int i = 0; i < node->outputs().size(); i++) {
					NkRect circle;
					circle.x = panel->bounds.x + panel->bounds.w - 5;
					circle.y = panel->bounds.y + space * (i + 1);
					circle.w = 10; circle.h = 10;
					nk_fill_circle(canvas, circle, nk_rgb(170, 100, 100));

					const struct nk_style* style = &ctx->style;
					nk_draw_text(
						canvas,
						nk_rect(circle.x + 12, circle.y+1, 100, 12),
						node->outputs()[i].label.c_str(),
						node->outputs()[i].label.size(),
						style->font,
						TEXT_S, TEXT_S
					);
					nk_draw_text(
						canvas,
						nk_rect(circle.x + 12, circle.y, 100, 12),
						node->outputs()[i].label.c_str(),
						node->outputs()[i].label.size(),
						style->font,
						TEXT, TEXT
					);

					/// Start linking process
					if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true)) {
						m_linking.active = true;
						m_linking.node = node;
						m_linking.inputID = node->id();
						m_linking.inputSlot = i;
					}

					/// Draw curve from linked node slot to mouse position
					if (m_linking.active && m_linking.node == node && m_linking.inputSlot == i) {
						NkVec2 l0 = nk_vec2(circle.x + 5, circle.y + 5);
						NkVec2 l1 = in->mouse.pos;
						nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
							l1.x - 50.0f, l1.y, l1.x, l1.y, 3.0f, nk_rgb(120, 120, 120));
					}
				}

				/// Inputs
				space = panel->bounds.h / float(node->inputs().size() + 1);
				for (int i = 0; i < node->inputs().size(); i++) {
					NkRect circle;
					circle.x = panel->bounds.x - 5;
					circle.y = panel->bounds.y + space * (i + 1);
					circle.w = 10; circle.h = 10;
					nk_fill_circle(canvas, circle, nk_rgb(100, 170, 100));

					const char* text = node->inputs()[i].label.c_str();
					int len = node->inputs()[i].label.size();

					const struct nk_style* style = &ctx->style;
					const struct nk_user_font* f = style->font;
					
					int tw = f->width(f->userdata, f->height, text, len);
						tw += (2.0f);

					nk_draw_text(
						canvas,
						nk_rect(circle.x - tw, circle.y+1, 100, 12),
						text, len,
						style->font,
						TEXT_S, TEXT_S
					);
					nk_draw_text(
						canvas,
						nk_rect(circle.x - tw, circle.y, 100, 12),
						text, len,
						style->font,
						TEXT, TEXT
					);

					/// Unlink
					if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
						nk_input_is_mouse_hovering_rect(in, circle) &&
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
						if (eid != -1) m_links.erase(m_links.begin() + eid);
					}

					if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
						nk_input_is_mouse_hovering_rect(in, circle) &&
						m_linking.active && m_linking.node != node)
					{
						m_linking.active = false;
						link(m_linking.inputID, m_linking.inputSlot, node->id(), i);
					}
				}
			}

			/// Reset linking connection
			if (m_linking.active && nk_input_is_mouse_released(in, NK_BUTTON_LEFT)) {
				m_linking.active = false;
				m_linking.node = nullptr;
			}

			/// Selection
			if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nk_layout_space_bounds(ctx))) {
				m_selected = nullptr;
				for (TNode* node : m_nodes) {
					if (node == nullptr) continue;
					NkRect b = nk_layout_space_rect_to_screen(ctx, node->m_bounds);
					b.x -= m_scrolling.x;
					b.y -= m_scrolling.y;
					if (nk_input_is_mouse_hovering_rect(in, b))
						m_selected = node;
				}
			}

			// Add menu
			if (nk_contextual_begin(ctx, 0, nk_vec2(100, 400), nk_window_get_bounds(ctx))) {
				nk_layout_row_dynamic(ctx, 24, 1);
				int x = in->mouse.pos.x + m_scrolling.x;
				int y = in->mouse.pos.y + m_scrolling.y;
				if (nk_contextual_item_label(ctx, "Value", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TValueNode());
				}
				//
				if (nk_contextual_item_label(ctx, "Add", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TMathNode(TMathNode::Add));
				}
				if (nk_contextual_item_label(ctx, "Subtract", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TMathNode(TMathNode::Sub));
				}
				if (nk_contextual_item_label(ctx, "Multiply", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TMathNode(TMathNode::Mul));
				}
				if (nk_contextual_item_label(ctx, "Power", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TMathNode(TMathNode::Pow));
				}
				//
				if (nk_contextual_item_label(ctx, "Vector4", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TVec4Node());
				}
				if (nk_contextual_item_label(ctx, "Oscillator", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TOscillatorNode(sampleRate));
				}
				if (nk_contextual_item_label(ctx, "ADSR", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TADSRNode(sampleRate));
				}
				if (nk_contextual_item_label(ctx, "Mix", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TMixNode());
				}
				if (nk_contextual_item_label(ctx, "Note", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TNoteNode());
				}
				if (nk_contextual_item_label(ctx, "Filter", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TFilterNode(sampleRate));
				}
				if (nk_contextual_item_label(ctx, "Button", NK_TEXT_ALIGN_LEFT)) {
					addNode(x, y, new TButtonNode());
				}
				nk_contextual_end(ctx);
			}
		}
		nk_layout_space_end(ctx);

		/// Window content scrolling
		if (nk_input_is_mouse_hovering_rect(in, nk_window_get_bounds(ctx)) &&
			nk_input_is_mouse_down(in, NK_BUTTON_MIDDLE)) {
			m_scrolling.x -= in->mouse.delta.x;
			m_scrolling.y -= in->mouse.delta.y;
		}
	}
	nk_end(ctx);
}

void TNodeEditor::addNode(int x, int y, TNode* node) {
	if (node == nullptr) return;
	node->m_id = m_nodes.size();
	node->m_bounds.x = x;
	node->m_bounds.y = y;
	m_nodes.push_back(node);
}

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

void TNodeEditor::deleteNode(TNode* node) {
	for (TLink* link : getAllLinksRelatedToNode(m_links, node)) {
		m_links.erase(std::find(m_links.begin(), m_links.end(), link));
		delete link;
	}

	m_nodes[node->id()] = nullptr;
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
	m_links.push_back(link);
}

TNodeEditor::TNodeEditor() {
	m_linking.active = 0;
	m_linking.inputID = 0;
	m_linking.inputSlot = 0;
	m_linking.node = nullptr;
	m_scrolling.x = 0;
	m_scrolling.y = 0;
	m_selected = nullptr;
	m_bounds.x = 0;
	m_bounds.y = 0;
	m_bounds.w = 1;
	m_bounds.h = 1;

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
		if (lnk->outputID == node->id() && m_nodes[lnk->inputID] != nullptr) {
			ins.push_back(m_nodes[lnk->inputID]);
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

void TNodeEditor::solve() {	
	for (TNode* node : m_solvedNodes) {
		if (node == nullptr) continue;
		node->solve();
		for (TLink* link : getNodeLinks(node)) {
			TNode* tgt = m_nodes[link->outputID];
			tgt->setInput(link->outputSlot, node->getOutput(link->inputSlot));
		}
	}

	// for (TNode* node : nodes) {
	// 	std::cout << node->m_title << " -> ";
	// }
	// std::cout << std::endl;
}

float TNodeEditor::output() {
	solve();

	const float ATTACK_TIME  = 5.0f / 1000.0f;
	const float RELEASE_TIME = 200.0f / 1000.0f;

	float attack  = 1.0f - std::exp(-1.0f / (ATTACK_TIME * sampleRate));
	float release = 1.0f - std::exp(-1.0f / (RELEASE_TIME * sampleRate));

	float sample = m_outputNode->getInput(0);
	m_signalDC = lerp(m_signalDC, sample, 0.5f / sampleRate);
	sample -= m_signalDC;

	float absSignal = std::abs(sample);
	if (absSignal > m_envelope) {
		m_envelope = lerp(m_envelope, absSignal, attack);
	} else {
		m_envelope = lerp(m_envelope, absSignal, release);
	}
	m_envelope = std::max(m_envelope, 1.0f);

	return std::min(std::max((sample * 0.6f / m_envelope), -1.0f), 1.0f) * m_outputNode->volume;
}