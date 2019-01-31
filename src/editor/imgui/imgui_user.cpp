#include "imgui.h"
#include "imgui_user.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_PLACEMENT_NEW
#include "imgui_internal.h"

using namespace ImGui;

#include <algorithm>
#include <cstdint>

#define BLOCK_COUNT 30
#define BLOCK_HEIGHT 4
#define BLOCK_SPACE 1

#include "knob.h"
#include "vu.h"
#include "sw.h"
#include "keys.h"
#include "button.h"

namespace ImGui {
bool IsItemActiveLastFrame() {
	ImGuiContext& g = *GImGui;
	if (g.ActiveIdPreviousFrame)
		return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
	return false;
}

bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size) {
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

bool Knob(const char* label, float* p_value, float v_min, float v_max) {
	if (KnobTex == nullptr) {
		KnobTex = new TTex(out_data, out_size);
	}

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	const ImGuiID id = window->GetID(label);

	const float width = 32.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float line_height = ImGui::GetTextLineHeight();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// ImGui::BeginGroup();
	// ImGui::PushItemWidth(width);

	// char ilbl[128] = {0};
	// ImFormatString(ilbl, 128, "##drag%s", label);
	// ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_TitleBgCollapsed));
	// bool in_drag = ImGui::DragFloat(ilbl, p_value, 0.01f, v_min, v_max);
	// ImGui::PopStyleColor();

	// ImGui::PopItemWidth();
	ImGui::InvisibleButton(label, ImVec2(width, width + line_height + style.ItemInnerSpacing.y));

	bool value_changed = false;
	bool is_active = ImGui::IsItemActive();
	bool is_hovered = ImGui::IsItemActive();
	bool is_dblclick = ImGui::IsMouseDoubleClicked(0) && is_hovered;
	if (is_active && io.MouseDelta.x != 0.0f) {
		float step = (v_max - v_min) / 200.0f;
		*p_value += io.MouseDelta.x * step;
		if (*p_value < v_min) *p_value = v_min;
		if (*p_value > v_max) *p_value = v_max;
	} else if (!is_active && ImGui::IsItemActiveLastFrame()) {
		value_changed = true;
	}

	// ImGui::EndGroup();

	float t = (*p_value - v_min) / (v_max - v_min);

	const float tw = 1.0f / 10;
	const float th = 1.0f / 10;

	int index = int(t * 99);

	draw_list->PushClipRect(
		pos,
		pos + ImVec2(width, width + line_height + style.ItemInnerSpacing.y),
		true
	);

	draw_list->AddRectFilled(
		pos + ImVec2(0, 0),
		pos + ImVec2(width, width + line_height + style.ItemInnerSpacing.y),
		IM_COL32(120, 120, 120, 128),
		4.0f
	);

	draw_list->AddImage(
		(ImTextureID)(KnobTex->id()),
		pos + ImVec2(0, 0),
		pos + ImVec2(width, width),
		ImVec2(tw * (index % 10), th * (index / 10)),
		ImVec2(tw * (index % 10) + tw, th * (index / 10) + th)
	);

	ImVec2 tsz = ImGui::CalcTextSize(label);
	float centerTextX = width/2 - tsz.x/2;
	draw_list->AddText(ImVec2(pos.x + centerTextX, pos.y + width + style.ItemInnerSpacing.y+1.2f), ImGui::GetColorU32(ImGuiCol_FrameBg), label);
	draw_list->AddText(ImVec2(pos.x + centerTextX, pos.y + width + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

	draw_list->PopClipRect();

	if (is_hovered) {
		ImGui::BeginTooltip();
		ImGui::Text("%.03f", *p_value);
		ImGui::EndTooltip();
	}

	return value_changed;
}

void ToggleButton(const char* str_id, bool* v) {
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight();
	float width = 32.0f;
	float radius = height * 0.5f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	if (ImGui::IsItemClicked())
		*v = !*v;

	float t = *v ? 1.0f : 0.0f;

	ImGuiContext& g = *GImGui;
	float ANIM_SPEED = 0.08f;
	if (g.ActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
	{
		float t_anim = ImSaturate(g.ActiveIdTimer / ANIM_SPEED);
		t = *v ? (t_anim) : (1.0f - t_anim);
	}

	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
	else
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

static float remap(float value, float from1, float to1, float from2, float to2) {
	return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

float VUMeter(const char* id, float value) {
	if (VUTex == nullptr) {
		VUTex = new TTex(vu_png, vu_png_len);
	}

	const float width = float(int(VUTex->width() / 10));
	const float height = float(int(VUTex->height() / 10));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const float tw = 1.0f / 10;
	const float th = 1.0f / 10;

	int index = (int) std::floor(value * 99.0f);
	index = ImClamp(index, 0, 100);

	const ImVec2 wp = ImGui::GetCursorScreenPos();

	draw_list->PushClipRect(wp, ImVec2(width, height) + wp, true);

	float x = tw * float(index % 10);
	float y = th * float(int(index / 10));
	draw_list->AddImage(
		(ImTextureID)(VUTex->id()),
		wp,
		ImVec2(width, height) + wp,
		ImVec2(x, y),
		ImVec2(x + tw, y + th)
	);

	ImGui::InvisibleButton(id, ImVec2(width, height));

	draw_list->AddRect(
		wp,
		ImVec2(width, height) + wp,
		IM_COL32(100, 100, 100, 255)
	);

	draw_list->PopClipRect();

	return height;
}

bool LinkText(const char* text) {
	const ImVec4 LINK_COLOR = ImVec4(0.25f, 0.64f, 0.85f, 1.0f);

	ImVec2 sz = ImGui::CalcTextSize(text);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(text);
	const ImVec2 label_size = ImGui::CalcTextSize(text, NULL, true);
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, 0.0f);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	ImGuiButtonFlags flags = 0;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
	//ImGuiMouseCursor_Hand
	if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	// Render
	ImGui::RenderNavHighlight(bb, id);
	ImVec4 col = LINK_COLOR;
	ImVec2 p0 = bb.Min;
	ImVec2 p1 = bb.Max;
	if (hovered && held)
	{
		p0 += ImVec2(1, 1);
		p1 += ImVec2(1, 1);
	}
	window->DrawList->AddLine(ImVec2(p0.x + style.FramePadding.x, p1.y), ImVec2(p1.x - style.FramePadding.x, p1.y), ImGui::GetColorU32(col));
	ImGui::PushStyleColor(ImGuiCol_Text, col);
	ImGui::RenderTextClipped(p0, p1, text, NULL, &label_size, style.ButtonTextAlign, &bb);
	ImGui::PopStyleColor(1);

	return pressed;
}

void AudioView(const char* id, float width, float* values, int length, int pos, float h) {
	const int col = IM_COL32(0, 200, 100, 255);

	int pos_r = int((float(pos) / length) * width);

	const ImVec2 wp = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton(id, ImVec2(width, h*2));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const ImVec2 rect_max = ImVec2(width, h*2) + wp;

	draw_list->AddRectFilled(
		wp,
		rect_max,
		IM_COL32(0, 0, 0, 255)
	);

	draw_list->PushClipRect(wp, rect_max, true);

	draw_list->AddLine(
		ImVec2(0.0f, h) + wp,
		ImVec2(width, h) + wp,
		IM_COL32(80,80,80,255)
	);

	if (values != nullptr) {
		ImVec2 prev = ImVec2(0.0f, h) + wp;
		for (int i = 0; i < width; i++) {
			int j = int((float(i) / width) * length);
			ImVec2 pos = ImVec2(i, values[j] * h + h) + wp;
			draw_list->AddLine(prev, pos, col);
			prev = pos;
		}
	}

	draw_list->AddLine(
		ImVec2(pos_r, 0) + wp,
		ImVec2(pos_r, h*2) + wp,
		IM_COL32(200, 100, 100, 128),
		2
	);
	draw_list->AddLine(
		ImVec2(pos_r, 0) + wp,
		ImVec2(pos_r, h*2) + wp,
		IM_COL32(200, 100, 100, 255)
	);

	draw_list->PopClipRect();


	draw_list->AddRect(
		wp,
		rect_max,
		IM_COL32(100, 100, 100, 255)
	);

}

bool KeyBed(const char* id, bool* keys, int keyCount) {
	if (keyCount <= 0) return false;

	if (KBTex == nullptr) {
		KBTex = new TTex(keys_n_png, keys_n_png_len);
	}

	const int kW = 48;
	const int kH = 12;
	const int khH = kH/2;

	float height = keyCount * kH;

	const ImVec2 wp = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton(id, ImVec2(kW, height));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const float uW = 1.0f / 2;
	const float uH = 1.0f / 2;

	float y = 0;

	// White keys
	for (int i = keyCount-1; i >= 0; i--) {
		int ki = i % 12;

		bool pressed = keys[i];
		int ti = pressed ? 1 : 0;

		float ay = y;
		switch (ki) {
			case 11:
			case 9:
			case 7:
			case 5:
			case 4:
			case 2:
			case 0:
				y += kH;
				break;
			default: continue;
		}

		float uX = (ti % 2) * uW;
		float uY = int(ti / 2) * uH;
		draw_list->AddImage(
			(ImTextureID)(KBTex->id()),
			ImVec2(0, ay) + wp,
			ImVec2(kW, ay + kH) + wp,
			ImVec2(uX, uY),
			ImVec2(uX+uW, uY+uH)
		);
	}

	// Black keys
	y = khH-1;
	for (int i = keyCount-1; i >= 0; i--) {
		int ki = i % 12;

		bool pressed = keys[i];
		int ti = pressed ? 3 : 2;

		float ay = y;
		switch (ki) {
			case 10: y += kH+2; break;
			case 8:  y += kH+2-1; break;
			case 6:  y += kH*2-2; break;
			case 3:  y += kH+1; break;
			case 1:  y += kH*2-2; break;
			default: continue;
		}

		float uX = (ti % 2) * uW;
		float uY = int(ti / 2) * uH;
		draw_list->AddImage(
			(ImTextureID)(KBTex->id()),
			ImVec2(0, ay) + wp,
			ImVec2(kW, ay + kH) + wp,
			ImVec2(uX, uY),
			ImVec2(uX+uW, uY+uH)
		);
	}

	float ay = 0;
	for (int i = keyCount-1; i >= 0; i--) {
		int ki = i % 12;
		ImRect kr;
		kr.Min = ImVec2(0, ay) + wp;
		kr.Max = ImVec2(kW, ay + 7) + wp;

		if (ImGui::IsMouseDown(0) && kr.Contains(ImGui::GetMousePos())) {
			keys[i] = true;
		}
		ay += 7;
	}

	return false;
}

bool HotKey(int mod, int key) {
	ImGuiIO& io = ImGui::GetIO();

	int gmod = 0;
	if (io.KeyCtrl) gmod |= CTRL;
	if (io.KeyShift) gmod |= SHIFT;
	if (io.KeyAlt) gmod |= ALT;

	return (gmod == mod) && ImGui::IsKeyPressed(key, false);
}

bool RubberButton(const char* id) {
	if (ButtonTex == nullptr) {
		ButtonTex = new TTex(button_png, button_png_len);
	}

	const ImVec2 wp = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton(id, ImVec2(32, 32));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const float tw = 0.5f;
	const float th = 1.0f;

	int idx = 0;
	bool ret = ImGui::IsItemActive();
	if (ret) {
		idx = 1;
	}

	float x = tw * (idx % 2);

	draw_list->AddImage(
		(ImTextureID)(ButtonTex->id()),
		ImVec2(0, 0) + wp,
		ImVec2(32, 32) + wp,
		ImVec2(x, 0.0f),
		ImVec2(x + tw, th)
	);

	return ret;
}

void DrawAudioView(float x, float y, float width, float* values, int length, float h, float rad, int corners) {
	const UINT32 col = IM_COL32(0, 200, 100, 255);

	const ImVec2 wp = ImVec2(x, y);
	const ImVec2 rect_max = ImVec2(width, h) + wp;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	draw_list->AddRectFilled(
		wp,
		rect_max,
		IM_COL32(0, 0, 0, 255),
		rad,
		corners
	);

	draw_list->PushClipRect(wp, rect_max, true);

	draw_list->AddLine(
		ImVec2(0.0f, h/2) + wp,
		ImVec2(width, h/2) + wp,
		IM_COL32(80,80,80,255)
	);

	if (values != nullptr) {
		ImVec2 prev = ImVec2(0.0f, h) + wp;
		for (int i = 0; i < width; i++) {
			int j = int((float(i) / width) * length);
			float val = std::min(std::max(values[j],  -1.0f), 1.0f) * 0.5f + 0.5f;
			ImVec2 pos = ImVec2(i, val * h) + wp;
			draw_list->AddLine(prev, pos, col);
			prev = pos;
		}
	}

	draw_list->PopClipRect();

//	draw_list->AddRect(
//		wp,
//		rect_max,
//		IM_COL32(100, 100, 100, 255),
//		rad
//	);
}

}

#define TAB_SMOOTH_DRAG 0   // This work nicely but has overlapping issues (maybe render dragged tab separately, at end)

// Basic keyed storage, slow/amortized insertion, O(Log N) queries over a dense/hot buffer
template<typename T>
struct ImGuiBigStorage
{
	ImGuiStorage    Map;    // ID->Index
	ImVector<T>     Data;

	~ImGuiBigStorage() { Clear(); }
	void Clear()
	{
		for (int n = 0; n < Data.Size; n++)
			Data[n].~T();
		Map.Clear();
		Data.clear();
	}
	T*  GetOrCreateByKey(ImGuiID key)
	{
		int* p_idx = Map.GetIntRef(key, -1);
		if (*p_idx == -1)
		{
			*p_idx = Data.Size;
			Data.resize(Data.Size + 1);
			IM_PLACEMENT_NEW(&Data[*p_idx]) T();
		}
		return &Data[*p_idx];
	}
	T*  GetByKey(ImGuiID key)
	{
		int idx = Map.GetInt(key, -1);
		return (idx != -1) ? &Data[idx] : NULL;
	}
};

// sizeof() = 60~
struct ImGuiTabItem
{
	ImGuiID         Id;
	int             GlobalIndex;        // Index in ctx.Tabs[] array, this is the way to uniquely reference to a Tab (instead of using a pointer)
	int             CurrentOrder;       // Index for display. Include hidden tabs. Assigned in TabBarLayout() when expected this frame
	int             CurrentOrderVisible;// Index for display. Include hidden tabs. Assigned in TabBarLayout() when expected this frame
	int             LastFrameVisible;
	int             LastFrameSelected;  // This allows us to rebuild an ordered list of the last activated tabs with little maintenance and zero cost on activation.
	float           OffsetAnim;         // Position relative to beginning of tab
	float           OffsetTarget;       // Position relative to beginning of tab
	float           WidthContents;      // Width of actual contents, stored during TabItem() call
	float           WidthAnim;          // Width currently displayed (animating toward TabWidthDesired)
	float           WidthTarget;        // Width calculated by tab bar
	float           AppearAnim;
	bool            SkipOffsetAnim;
	bool            SkipAppearAnim;
	char            DebugName[16];

	ImGuiTabItem()
	{
		Id = 0;
		GlobalIndex = -1;
		CurrentOrder = -1;
		CurrentOrderVisible = -1;
		LastFrameVisible = LastFrameSelected -1;
		OffsetAnim = OffsetTarget = 0.0f;
		WidthContents = WidthAnim = WidthTarget = 0.0f;
		AppearAnim = 1.0f;
		SkipOffsetAnim = SkipAppearAnim = false;
		memset(DebugName, 0, sizeof(DebugName));
	}
};

// sizeof() = 96~112 bytes, +3 allocs
struct ImGuiTabBar
{
	ImGuiID             Id;
	ImGuiID             CurrSelectedTabId, NextSelectedTabId;
	int                 CurrTabCount, NextTabCount;
	int                 CurrVisibleCount;
	int                 LastFrameVisible, CurrFrameVisible;
	ImRect              BarRect;
	ImRect              ContentsRect;
	float               OffsetMax;
	float               ScrollingAnim;
	float               ScrollingTarget;
	ImGuiTabBarFlags    Flags;
	ImVector<int>       TabsOrder;
	int                 ReorderRequestTabIdx;
	int                 ReorderRequestDir;
	bool                WantLayout;
	bool                CurrOrderInsideTabsIsValid;
	bool                CurrSelectedTabIdIsAlive;

	ImGuiTabBar()
	{
		Id = 0;
		CurrSelectedTabId = NextSelectedTabId = 0;
		CurrTabCount = NextTabCount = 0;
		CurrVisibleCount = 0;
		LastFrameVisible = CurrFrameVisible = -1;
		OffsetMax = 0.0f;
		ScrollingAnim = ScrollingTarget = 0.0f;
		Flags = ImGuiTabBarFlags_None;
		ReorderRequestTabIdx = -1;
		ReorderRequestDir = 0;
		WantLayout = false;
		CurrOrderInsideTabsIsValid = false;
		CurrSelectedTabIdIsAlive = false;
	}
};

static bool ArrowButton(ImGuiID id, ImGuiDir dir, ImVec2 padding, ImGuiButtonFlags flags = 0)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	const ImGuiStyle& style = g.Style;

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImVec2(g.FontSize + padding.x * 2.0f, g.FontSize + padding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	const ImU32 col = ImGui::GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::RenderNavHighlight(bb, id);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderArrow(bb.Min + padding, dir, 1.0f);

	return pressed;
}

// FIXME: Helper for external extensions to handle multiple-context.
// Possibly: require user to call setcontext functions on various subsystems, or have hooks in main SetCurrentContext()
// Possibly: have a way for each extension to register themselves globally, so a void* can be stored in ImGuiContext that can be accessed in constant-time from a handle(=index).
struct ImGuiTabsContext
{
	ImGuiBigStorage<ImGuiTabItem>   Tabs;
	ImGuiBigStorage<ImGuiTabBar>    TabBars;
	ImVector<ImGuiTabBar*>          CurrentTabBar;

	ImGuiTabsContext() { }

	ImGuiTabItem* GetTabByIndex(int index)
	{
		return &Tabs.Data[index];
	}
	ImGuiTabItem* GetTabByOrder(ImGuiTabBar* tab_bar, int order)
	{
		return &Tabs.Data[tab_bar->TabsOrder.Data[order]];
	}
};

static ImGuiTabsContext GTabs;

//-----------------------------------------------------------------------------
// TABS
//-----------------------------------------------------------------------------

#define TAB_UNFOCUSED_LERP 1

enum
{
#if TAB_UNFOCUSED_LERP
	ImGuiCol_Tab = ImGuiCol_Header,
#else
	ImGuiCol_Tab = ImGuiCol_WindowBg,
#endif
	ImGuiCol_TabHovered = ImGuiCol_HeaderHovered,
	ImGuiCol_TabActive = ImGuiCol_HeaderActive,
#if TAB_UNFOCUSED_LERP
	ImGuiCol_TabUnfocused = ImGuiCol_COUNT + 100,
#else
	ImGuiCol_TabUnfocused = ImGuiCol_TitleBgCollapsed
#endif
};

static ImU32   TabGetColorU32(int idx)
{
#if TAB_UNFOCUSED_LERP
	if (idx == ImGuiCol_TabUnfocused)
		return ImGui::GetColorU32(ImLerp(ImGui::GetStyleColorVec4(ImGuiCol_Tab), ImGui::GetStyleColorVec4(ImGuiCol_TabHovered), 0.60f));
#endif
	return ImGui::GetColorU32(idx);
}

// FIXME: flags can be removed once we move border to style
static void RenderTabBackground(ImDrawList* draw_list, const ImRect& bb, ImU32 col)
{
	ImGuiContext& g = *GImGui;
	const float rounding = ImMin(g.FontSize * 0.35f, bb.GetWidth() * 0.5f);
	draw_list->PathLineTo(ImVec2(bb.Min.x, bb.Max.y));
	draw_list->PathArcToFast(ImVec2(bb.Min.x + rounding, bb.Min.y + rounding), rounding, 6, 9);
	draw_list->PathArcToFast(ImVec2(bb.Max.x - rounding, bb.Min.y + rounding), rounding, 9, 12);
	draw_list->PathLineTo(ImVec2(bb.Max.x, bb.Max.y));
	draw_list->AddConvexPolyFilled(draw_list->_Path.Data, draw_list->_Path.Size, col);
	if (g.Style.FrameBorderSize > 0.0f)
		draw_list->AddPolyline(draw_list->_Path.Data, draw_list->_Path.Size, ImGui::GetColorU32(ImGuiCol_Border), false, g.Style.FrameBorderSize);
	draw_list->PathClear();
}

void    ImGui::BeginTabBar(const char* str_id, ImGuiTabBarFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;
	ImGuiWindow* window = g.CurrentWindow;

	ImGuiTabsContext& ctx = GTabs;
	IM_ASSERT(ctx.CurrentTabBar.empty());                   // Cannot recurse BeginTabBar()

	// Flags
	if ((flags & ImGuiTabBarFlags_SizingPolicyMask_) == 0)
		flags |= ImGuiTabBarFlags_SizingPolicyDefault_;

	PushID(str_id);
	const ImGuiID id = window->GetID("");
	ImGuiTabBar* tab_bar = ctx.TabBars.GetOrCreateByKey(id);
	IM_ASSERT(tab_bar->CurrFrameVisible != g.FrameCount);   // Cannot call multiple times in the same frame

	ctx.CurrentTabBar.push_back(tab_bar);
	tab_bar->Id = id;
	tab_bar->Flags = flags;
	tab_bar->BarRect = ImRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(GetContentRegionAvailWidth(), g.FontSize + style.FramePadding.y * 2.0f));
	tab_bar->WantLayout = true; // Layout will be done on the first call to ItemTab()
	tab_bar->LastFrameVisible = tab_bar->CurrFrameVisible;
	tab_bar->CurrFrameVisible = g.FrameCount;

	// Clear current tab information if it wasn't submitted this frame
	if (!tab_bar->CurrSelectedTabIdIsAlive)
		tab_bar->CurrSelectedTabId = 0;

	ItemSize(tab_bar->BarRect);
	ItemAdd(tab_bar->BarRect, 0);
	window->DC.LastItemId = id; // We don't want Nav but for drag and drop we need an item id
	tab_bar->ContentsRect = ImRect(window->DC.CursorPos, window->DC.CursorPos + GetContentRegionAvail());

	// Draw separator
#if 1
	bool unfocused = !g.NavWindow || g.NavWindow->RootWindowForTitleBarHighlight != window->RootWindow;
#else
	bool unfocused = false;
#endif
	const ImU32 col = TabGetColorU32(unfocused ? ImGuiCol_TabUnfocused : ImGuiCol_TabActive);
	window->DrawList->AddLine(ImVec2(window->Pos.x, window->DC.CursorPos.y - style.ItemSpacing.y), ImVec2(window->Pos.x + window->Size.x, window->DC.CursorPos.y - style.ItemSpacing.y), col, 1.0f);
}

static int TabBarFindClosestVisibleInDirection(ImGuiTabBar* tab_bar, int order, int dir)
{
	ImGuiTabsContext& ctx = GTabs;
	for (order += dir; order >= 0 && order < tab_bar->TabsOrder.Size; order += dir)
		if (ctx.GetTabByOrder(tab_bar, order)->LastFrameVisible == tab_bar->LastFrameVisible)
			return order;
	return -1;
}

static void TabBarScrollClamp(ImGuiTabBar* tab_bar, float& scrolling)
{
	scrolling = ImMin(scrolling, tab_bar->OffsetMax - tab_bar->BarRect.GetWidth());
	scrolling = ImMax(scrolling, 0.0f);
}

static void TabBarScrollToTab(ImGuiTabBar* tab_bar, ImGuiTabItem* tab)
{
	ImGuiContext& g = *GImGui;
	float margin = g.FontSize * 1.0f; // When to scroll to make Tab N+1 visible always make a bit of N visible to suggest more scrolling area (since we don't have a scrollbar)
	float tab_x1 = tab->OffsetTarget + (tab->CurrentOrderVisible > 0 ? -margin : 0.0f);
	float tab_x2 = tab->OffsetTarget + tab->WidthTarget + (tab->CurrentOrderVisible + 1 < tab_bar->CurrVisibleCount ? margin : 0.0f);
	if (tab_bar->ScrollingTarget > tab_x1)
		tab_bar->ScrollingTarget = tab_x1;
	if (tab_bar->ScrollingTarget + tab_bar->BarRect.GetWidth() < tab_x2)
		tab_bar->ScrollingTarget = tab_x2 - tab_bar->BarRect.GetWidth();
}

static void TabBarQueueChangeTabOrder(ImGuiTabBar* tab_bar, const ImGuiTabItem* tab, int dir)
{
	IM_ASSERT(dir == -1 || dir == +1);
	IM_ASSERT(tab_bar->ReorderRequestTabIdx == -1);
	tab_bar->ReorderRequestTabIdx = tab->GlobalIndex;
	tab_bar->ReorderRequestDir = dir;
}

static ImVec2 TabBarCalcTabBaseSize(const ImVec2& label_size, bool* p_open)
{
	ImGuiContext& g = *GImGui;
	ImVec2 size = ImVec2(label_size.x + g.Style.FramePadding.x * 2.0f, label_size.y + g.Style.FramePadding.y * 2.0f);
	if (p_open != NULL)
		size.x += (g.Style.ItemInnerSpacing.x) + (g.FontSize + g.Style.FramePadding.y * 2.0f); // We use Y intentionally to fit the close button circle.
	return size;
}

// This is called only once a frame before by the first call to ItemTab()
// The reason we're not calling it in BeginTabBar() is to leave a chance to the user to call the SetTabItemClosed() functions.
static void TabBarLayout(ImGuiTabBar* tab_bar)
{
	ImGuiContext& g = *GImGui;
	ImGuiTabsContext& ctx = GTabs;
	ImGuiWindow* window = g.CurrentWindow;
	tab_bar->WantLayout = false;

	// Setup next selected tab
	ImGuiTabItem* scroll_track_selected_tab = NULL;
	if (tab_bar->NextSelectedTabId)
	{
		tab_bar->CurrSelectedTabId = tab_bar->NextSelectedTabId;
		tab_bar->NextSelectedTabId = 0;
		scroll_track_selected_tab = ctx.Tabs.GetByKey(tab_bar->CurrSelectedTabId);
	}
	tab_bar->CurrSelectedTabIdIsAlive = false;

	tab_bar->CurrTabCount = tab_bar->NextTabCount;
	tab_bar->NextTabCount = 0;

	// Process order change request (we could probably process it when requested but it's just saner to do it in a single spot).
	if (tab_bar->ReorderRequestTabIdx != -1)
	{
		IM_ASSERT(!(tab_bar->Flags & ImGuiTabBarFlags_NoReorder));
		ImGuiTabItem* tab1 = ctx.GetTabByIndex(tab_bar->ReorderRequestTabIdx);
		int tab2_order = TabBarFindClosestVisibleInDirection(tab_bar, tab1->CurrentOrder, tab_bar->ReorderRequestDir);
		if (tab2_order != -1)
		{
			ImGuiTabItem* tab2 = ctx.GetTabByOrder(tab_bar, tab2_order);
			ImSwap(tab_bar->TabsOrder.Data[tab1->CurrentOrder], tab_bar->TabsOrder.Data[tab2_order]);
			ImSwap(tab1->CurrentOrder, tab2->CurrentOrder);
			tab1->SkipOffsetAnim = true;
#if !TAB_SMOOTH_DRAG
			tab2->SkipOffsetAnim = true;
#endif
			if (tab1->Id == tab_bar->CurrSelectedTabId)
				scroll_track_selected_tab = tab1;
		}
		tab_bar->ReorderRequestTabIdx = -1;
	}

	// During layout we will search for those infos
	int selected_order = -1;
	ImGuiTabItem* most_recent_selected_tab = NULL;

	// Layout all active tabs
	const float tab_width_equal = (tab_bar->CurrTabCount > 0) ? (float)(int)((tab_bar->BarRect.GetWidth() - (tab_bar->CurrTabCount - 1) * g.Style.ItemInnerSpacing.x) / tab_bar->CurrTabCount) : 0.0f;
	float offset_x = 0.0f;
	int tab_order_visible_n = 0;
	for (int tab_order_n = 0; tab_order_n < tab_bar->TabsOrder.Size; tab_order_n++)
	{
		ImGuiTabItem* tab = ctx.GetTabByOrder(tab_bar, tab_order_n);
		tab->CurrentOrder = tab_order_n;
		tab->CurrentOrderVisible = -1;
		tab->OffsetTarget = offset_x;       // We set the offset even for invisible tabs, so can they readily reappear from here this frame, if needed.
		if (tab->LastFrameVisible != tab_bar->LastFrameVisible)
			continue;

		tab->CurrentOrderVisible = tab_order_visible_n++;
		if (tab->Id == tab_bar->CurrSelectedTabId)
			selected_order = tab->CurrentOrder;
		if (most_recent_selected_tab == NULL || most_recent_selected_tab->LastFrameSelected < tab->LastFrameSelected)
			most_recent_selected_tab = tab;
		if (scroll_track_selected_tab == NULL && g.NavJustMovedToId == tab->Id)
			scroll_track_selected_tab = tab;
		if (tab_bar->Flags & ImGuiTabBarFlags_SizingPolicyEqual)
		{
			const float TAB_MAX_WIDTH = g.FontSize * 13.0f;
			tab->WidthTarget = ImClamp(tab_width_equal, 0.0f, TAB_MAX_WIDTH);
		}
		else if (tab_bar->Flags & ImGuiTabBarFlags_SizingPolicyFit)
		{
			const float TAB_MAX_WIDTH = FLT_MAX;// 100.0f;
			tab->WidthTarget = ImMin(tab->WidthContents, TAB_MAX_WIDTH);
		}

		if (tab->SkipOffsetAnim)
		{
			tab->OffsetAnim = tab->OffsetTarget;
			tab->SkipOffsetAnim = false;
		}

		offset_x += tab->WidthTarget + g.Style.ItemInnerSpacing.x;
	}
	tab_bar->OffsetMax = ImMax(offset_x - g.Style.ItemInnerSpacing.x, 0.0f);
	tab_bar->CurrVisibleCount = tab_order_visible_n;
	tab_bar->CurrOrderInsideTabsIsValid = true;

	// Horizontal scrolling buttons
	// FIXME: This is not satisfying but I'll leave the polish for later.
	const float scrolling_speed = g.IO.DeltaTime * g.FontSize * 70.0f;
	if (tab_bar->OffsetMax > tab_bar->BarRect.GetWidth())
	{
		const ImVec2 backup_main_cursor_pos = window->DC.CursorPos;
		float buttons_width = g.FontSize * 2.0f + g.Style.ItemInnerSpacing.x;

		window->DC.CursorPos = ImVec2(tab_bar->BarRect.Max.x - buttons_width, tab_bar->BarRect.Min.y);
#if 0
		// Continuous scroll
		int scrolling_dir = 0;
		ArrowButton(window->GetID("##<"), ImGuiDir_Left, ImVec2(0.0f, g.Style.FramePadding.y));
		if (ImGui::IsItemActive())
			scrolling_dir = -1;
		ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
		ArrowButton(window->GetID("##>"), ImGuiDir_Right, ImVec2(0.0f, g.Style.FramePadding.y));
		if (ImGui::IsItemActive())
			scrolling_dir = +1;
		if (scrolling_dir != 0)
		{
			tab_bar->ScrollingAnim += scrolling_speed * scrolling_dir;
			tab_bar->ScrollingTarget += scrolling_speed * scrolling_dir;
		}
#else
		// Navigate tab by tab
		const float backup_repeat_delay = g.IO.KeyRepeatDelay;
		const float backup_repeat_rate = g.IO.KeyRepeatRate;
		int select_dir = 0;
		g.IO.KeyRepeatDelay = 0.250f;
		g.IO.KeyRepeatRate = 0.200f;
		if (ArrowButton(window->GetID("##<"), ImGuiDir_Left, ImVec2(0.0f, g.Style.FramePadding.y), ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_Repeat))
			select_dir = -1;
		ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
		if (ArrowButton(window->GetID("##>"), ImGuiDir_Right, ImVec2(0.0f, g.Style.FramePadding.y), ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_Repeat))
			select_dir = +1;
		g.IO.KeyRepeatRate = backup_repeat_rate;
		g.IO.KeyRepeatDelay = backup_repeat_delay;
		if (select_dir != 0)
		{
			int tab2_order = TabBarFindClosestVisibleInDirection(tab_bar, selected_order, select_dir);
			if (tab2_order != -1)
			{
				scroll_track_selected_tab = ctx.GetTabByOrder(tab_bar, tab2_order);
				tab_bar->CurrSelectedTabId = scroll_track_selected_tab->Id;
			}
		}
#endif
		window->DC.CursorPos = backup_main_cursor_pos;
		tab_bar->BarRect.Max.x -= buttons_width + g.Style.ItemInnerSpacing.x;
	}
	TabBarScrollClamp(tab_bar, tab_bar->ScrollingAnim);
	TabBarScrollClamp(tab_bar, tab_bar->ScrollingTarget);
	tab_bar->ScrollingAnim = ImLinearSweep(tab_bar->ScrollingAnim, tab_bar->ScrollingTarget, scrolling_speed);

	// If we have lost the selected tab, select the next most recently active one.
	if (tab_bar->CurrSelectedTabId == 0 && tab_bar->NextSelectedTabId == 0 && most_recent_selected_tab != NULL)
	{
		tab_bar->CurrSelectedTabId = most_recent_selected_tab->Id;
		scroll_track_selected_tab = most_recent_selected_tab;
	}

	if (scroll_track_selected_tab)
		TabBarScrollToTab(tab_bar, scroll_track_selected_tab);
}

// We have to essentially avoid logic here since docked tab will be submitted after the fact.
void    ImGui::EndTabBar()
{
	ImGuiTabsContext& ctx = GTabs;
	IM_ASSERT(!ctx.CurrentTabBar.empty());      // Mismatched BeginTabBar/EndTabBar

	PopID();
	ctx.CurrentTabBar.pop_back();
}

// Not sure this is really useful...
void    ImGui::SetTabItemSelected(const char* label)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiTabsContext& ctx = GTabs;
	IM_ASSERT(!ctx.CurrentTabBar.empty());  // Needs to be called between BeginTabBar() and EndTabBar()
	ImGuiTabBar* tab_bar = ctx.CurrentTabBar.back();

	const ImGuiID id = window->GetID(label);
	IM_ASSERT(tab_bar->WantLayout);         // Needs to be called between BeginTabBar() and before the first call to TabItem()
	tab_bar->NextSelectedTabId = id;
}

// This is call is 100% optional and allow to remove some one-frame glitches when a tab has been unexpectedly removed.
// To use it to need to call the function SetTabItemClosed() after BeginTabBar() and before any call to TabItem()
void    ImGui::SetTabItemClosed(const char* label)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiTabsContext& ctx = GTabs;
	IM_ASSERT(!ctx.CurrentTabBar.empty());  // Needs to be called between BeginTabBar() and EndTabBar()
	ImGuiTabBar* tab_bar = ctx.CurrentTabBar.back();

	const ImGuiID id = window->GetID(label);
	IM_ASSERT(tab_bar->WantLayout);         // Needs to be called between BeginTabBar() and before the first call to TabItem()
	if (ImGuiTabItem* tab = ctx.Tabs.GetByKey(id))
	{
		if (tab->LastFrameVisible == tab_bar->LastFrameVisible)
			tab_bar->NextTabCount--;
		tab->LastFrameVisible = -1;
		if (tab->Id == tab_bar->CurrSelectedTabId)
		{
			tab_bar->CurrSelectedTabId = tab_bar->NextSelectedTabId = 0;
			tab_bar->CurrSelectedTabIdIsAlive = false;
		}
	}
}

bool ImGui::TabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags)
{
	// Acquire tab bar data
	ImGuiTabsContext& ctx = GTabs;
	IM_ASSERT(!ctx.CurrentTabBar.empty());  // Needs to be called between BeginTabBar() and EndTabBar()
	ImGuiTabBar* tab_bar = ctx.CurrentTabBar.back();
	if (tab_bar->WantLayout)
		TabBarLayout(tab_bar);

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	//if (window->SkipItems) // FIXME-OPT
	//    return false;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	// If the user called us with *p_open == false, we early out and don't render. We make a dummy call to ItemAdd() so that attempts to use a contextual popup menu with an implicit ID won't use an older ID.
	if (p_open && !*p_open)
	{
		PushItemFlag(ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus, true);
		ItemAdd(ImRect(), id);
		PopItemFlag();
		return false;
	}
	tab_bar->NextTabCount++;

	// Acquire tab data
	ImGuiTabItem* tab = ctx.Tabs.GetOrCreateByKey(id);
	if (tab->Id == 0)
	{
		// Initialize tab
		tab->Id = id;
		tab->GlobalIndex = ctx.Tabs.Data.Size - 1;
		tab_bar->TabsOrder.push_back(tab->GlobalIndex);
	}
	ImStrncpy(tab->DebugName, label, IM_ARRAYSIZE(tab->DebugName));

	const bool tab_bar_appearing = (tab_bar->LastFrameVisible + 1 < g.FrameCount);
	const bool tab_appearing = (tab->LastFrameVisible + 1 < g.FrameCount);
	const bool tab_selected = (tab_bar->CurrSelectedTabId == id);

	// Position newly appearing tab at the end of the tab list
	if (tab_appearing && !tab_bar_appearing && !(tab_bar->Flags & ImGuiTabBarFlags_NoResetOrderOnAppearing))
	{
		tab->OffsetTarget = tab_bar->OffsetMax + g.Style.ItemInnerSpacing.x;
		if (tab->CurrentOrder != -1 && tab_bar->TabsOrder.back() != tab->GlobalIndex)
		{
			// Move tab to end of the list
			IM_ASSERT(tab_bar->TabsOrder[tab->CurrentOrder] == tab->GlobalIndex);
			memmove(&tab_bar->TabsOrder[tab->CurrentOrder], &tab_bar->TabsOrder[tab->CurrentOrder + 1], (tab_bar->TabsOrder.Size - tab->CurrentOrder - 1) * sizeof(tab_bar->TabsOrder[0]));
			tab_bar->TabsOrder.back() = tab->GlobalIndex;
			tab_bar->CurrOrderInsideTabsIsValid = false;
		}
	}

	// Update selected tab
	if (tab_appearing && !(tab_bar->Flags & ImGuiTabBarFlags_NoSelectionOnAppearing) && tab_bar->NextSelectedTabId == 0)
		tab_bar->NextSelectedTabId = id;  // New tabs gets activated
	if (tab_bar->CurrSelectedTabId == id)
	{
		tab_bar->CurrSelectedTabIdIsAlive = true;
		tab->LastFrameSelected = g.FrameCount;
	}

	// Backup current layout position
	const ImVec2 backup_main_cursor_pos = window->DC.CursorPos;

	// Calculate tab contents size
	ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 size = TabBarCalcTabBaseSize(label_size, p_open);
	tab->WidthContents = size.x;

	// Animate
	{
		// If Tab just reappeared we'll animate vertically only
		if (tab_appearing && !tab_bar_appearing)
		{
			tab->OffsetAnim = tab->OffsetTarget;
			tab->AppearAnim = 0.0f;
		}
		// If Tab Bar just reappeared we aren't animating at all.
		if (tab_appearing || tab_bar_appearing || tab->WidthAnim == 0.0f)
		{
			tab->OffsetAnim = tab->OffsetTarget;
			tab->WidthAnim = tab->WidthTarget;
		}
		if (tab_bar_appearing || tab->SkipOffsetAnim)
		{
			tab->AppearAnim = 1.0f;
			tab->SkipOffsetAnim = false;
		}

		const float speed_x = (tab_bar->Flags & ImGuiTabBarFlags_NoAnim) ? (FLT_MAX) : (g.FontSize * 80.0f * g.IO.DeltaTime);
		const float speed_y = (tab_bar->Flags & ImGuiTabBarFlags_NoAnim) ? (FLT_MAX) : (g.FontSize * 0.80f * g.IO.DeltaTime);
		tab->OffsetAnim = ImLinearSweep(tab->OffsetAnim, tab->OffsetTarget, speed_x);
		tab->WidthAnim = ImLinearSweep(tab->WidthAnim, tab->WidthTarget, speed_x);
		tab->AppearAnim = ImLinearSweep(tab->AppearAnim, 1.0f, speed_y);
	}

	tab->LastFrameVisible = g.FrameCount;
	size.x = tab->WidthAnim;

	// Layout
	window->DC.CursorPos = tab_bar->BarRect.Min + ImVec2((float)(int)tab->OffsetAnim - tab_bar->ScrollingAnim, 0.0f);
	ImVec2 pos = window->DC.CursorPos;

	ImRect bb(pos, pos + size);
	bool want_clip_rect = (bb.Max.x >= tab_bar->BarRect.Max.x) || (tab->AppearAnim < 1.0f);
	if (want_clip_rect)
		PushClipRect(ImVec2(bb.Min.x, bb.Min.y - 1), ImVec2(tab_bar->BarRect.Max.x, bb.Max.y), true);

	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
	{
		if (want_clip_rect)
			PopClipRect();
		window->DC.CursorPos = backup_main_cursor_pos;
		return tab_selected && !tab_appearing;
	}

	// Click to Select a tab
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnDragDropHold | ImGuiButtonFlags_AllowItemOverlap);
	hovered |= (g.HoveredId == id);
	if (pressed || (flags & ImGuiTabItemFlags_SetSelected))
		tab_bar->NextSelectedTabId = id;
	//if (tab_is_selected)
	//    hovered = true;

	// Allow the close button to overlap unless we are dragging (in which case we don't want any overlapping tabs to be hovered)
	if (!held)
		SetItemAllowOverlap();

	// Drag and drop
	if (held && !tab_appearing && IsMouseDragging())
	{
		// Re-order local tabs
		if (!(tab_bar->Flags & ImGuiTabBarFlags_NoReorder))
		{
			// CurrentOrderInsideTabsIsValid is only going to be invalid when a new tab appeared this frame and was pushed to the end of the list, and wasn't already at the end of the internal list.
			// It will prevent changing tab order during this frame, but it can be triggered on the next frame. With the mouse movement required to move tabs, it's unlikely someone will ever even notice.
			// The correct solution would be to refresh the tab order at least for the selected tab. Probably not worth bothering.
			if (tab_bar->CurrOrderInsideTabsIsValid)
			{
				// VisualStudio style
				const float anim_dx = (tab->OffsetTarget - tab->OffsetAnim); // Interaction always operated on target positions, ignoring animations!
				if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < bb.Min.x + anim_dx)
					TabBarQueueChangeTabOrder(tab_bar, tab, -1);
				else if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > bb.Max.x + anim_dx)
					TabBarQueueChangeTabOrder(tab_bar, tab, +1);

				// Live translate display (like Sublime). Need to comment 'tab2->SkipLayoutAnim = true' in EndTabBar() too.
				#if TAB_SMOOTH_DRAG
					bb.Translate(ImVec2((g.IO.MousePos.x - g.ActiveIdClickOffset.x) - bb.Min.x, 0.0f));
				#endif
			}
		}
	}

	bool just_closed = false;

	// Render
	ImDrawList* draw_list = window->DrawList;
	const float close_button_sz = g.FontSize * 0.5f;
	if (!tab_appearing)
	{
		// Render: very small offset to make selected tab stick out
		bb.Min.y += tab_selected ? -1.0f : 0.0f;

		// Render: offset vertically + clipping when animating (we don't have enough CPU clipping primitives to clip the CloseButton, so this temporarily adds 1 draw call)
		if (tab->AppearAnim < 1.0f)
			bb.Translate(ImVec2(0.0f, (float)(int)((1.0f - tab->AppearAnim) * size.y)));

#if 1
		bool unfocused = (tab_selected) && (!g.NavWindow || g.NavWindow->RootWindowForTitleBarHighlight != window->RootWindow);
#else
		bool unfocused = false;
#endif

		// Render tab shape
		const ImU32 col = TabGetColorU32((hovered && held) ? ImGuiCol_TabActive : hovered ? ImGuiCol_TabHovered : tab_selected ? (unfocused ? ImGuiCol_TabUnfocused : ImGuiCol_TabActive) : ImGuiCol_Tab);
		RenderTabBackground(draw_list, bb, col);

		// Render text label (with clipping + alpha gradient) + unsaved marker
		const char* TAB_UNSAVED_MARKER = "*";
		ImRect text_clip_bb(bb.Min + style.FramePadding, bb.Max);
		text_clip_bb.Max.x -= g.Style.ItemInnerSpacing.x;
		float text_gradient_extent = g.FontSize * 1.5f;
		if (flags & ImGuiTabItemFlags_UnsavedDocument)
		{
			text_clip_bb.Max.x -= CalcTextSize(TAB_UNSAVED_MARKER, NULL, false).x;
			ImVec2 unsaved_marker_pos(ImMin(bb.Min.x + style.FramePadding.x + label_size.x + 1, text_clip_bb.Max.x), bb.Min.y + style.FramePadding.y + (float)(int)(-g.FontSize * 0.25f));
			RenderTextClipped(unsaved_marker_pos, bb.Max - style.FramePadding, TAB_UNSAVED_MARKER, NULL, NULL);
		}

		// Close Button
		bool close_button_visible = false;
		if (p_open != NULL)
		{
			// We are relying on a subtle and confusing distinction between 'hovered' and 'g.HoveredId' which happens because we are using ImGuiButtonFlags_AllowOverlapMode + SetItemAllowOverlap()
			//  'hovered' will be true when hovering the Tab but NOT when hovering the close button
			//  'g.HoveredId==id' will be true when hovering the Tab including when hovering the close button
			//  'g.ActiveId==close_button_id' will be true when we are holding on the close button, in which case both hovered booleans are false
			const ImGuiID close_button_id = window->GetID((void*)(intptr_t)(id + 1));
			const bool hovered_unblocked = IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			const bool hovered_regular = g.HoveredId == id || g.HoveredId == close_button_id;
			if (hovered_regular || g.ActiveId == close_button_id)
			{
				close_button_visible = true;
				ImGuiItemHoveredDataBackup last_item_backup;
				last_item_backup.Backup();
				if (CloseButton(close_button_id, ImVec2(bb.Max.x - style.FramePadding.x - close_button_sz, bb.Min.y + style.FramePadding.y + close_button_sz), close_button_sz))
					just_closed = true;
				last_item_backup.Restore();

				// Close with middle mouse button
				if (!(tab_bar->Flags & ImGuiTabBarFlags_NoCloseWithMiddleMouseButton) && IsMouseClicked(2))
					just_closed = true;
			}

			// Select with right mouse button. This is so the common idiom for context menu automatically highlight the current widget.
			if (IsMouseClicked(1) && hovered_unblocked)
				tab_bar->NextSelectedTabId = id;
		}
		if (close_button_visible)
		{
			float dx = close_button_sz + style.FramePadding.x * 1.0f;    // Because we fade clipped label we don't need FramePadding * 2;
			text_clip_bb.Max.x -= dx;
			text_gradient_extent = ImMax(0.0f, text_gradient_extent - dx);
		}

		// Text with alpha fade if it doesn't fit
		// FIXME: Move into fancy RenderText* helpers.
//		int vert_start_idx = draw_list->_VtxCurrentIdx;
		RenderTextClipped(text_clip_bb.Min, text_clip_bb.Max, label, NULL, &label_size, ImVec2(0.0f, 0.0f));
//		if (text_clip_bb.GetWidth() < label_size.x)
//			ShadeVertsLinearAlphaGradientForLeftToRightText(draw_list->_VtxWritePtr - (draw_list->_VtxCurrentIdx - vert_start_idx), draw_list->_VtxWritePtr, text_clip_bb.Max.x - text_gradient_extent, text_clip_bb.Max.x);
	}

	// Process close
	if (just_closed)
	{
		*p_open = false;
		if (tab_selected && !(flags & ImGuiTabItemFlags_UnsavedDocument))
		{
			// This will remove a frame of lag for selecting another tab on closure.
			// However we don't run it in the case where the 'Unsaved' flag is set, so user gets a chance to fully undo the closure
			tab->LastFrameVisible = -1;
			tab_bar->NextSelectedTabId = 0;
			tab_bar->CurrSelectedTabIdIsAlive = false;
		}
		else if (!tab_selected && (flags & ImGuiTabItemFlags_UnsavedDocument))
		{
			// Actually select before expecting closure
			tab_bar->NextSelectedTabId = id;
		}
	}

	// Restore main window position so user can draw there
	if (want_clip_rect)
		PopClipRect();
	window->DC.CursorPos = backup_main_cursor_pos;

	return tab_selected && !tab_appearing;
}
