#ifndef IMGUI_USER_H
#define IMGUI_USER_H

#include "../TTex.hpp"

typedef int ImGuiTabBarFlags;
typedef int ImGuiTabItemFlags;

namespace ImGui {
static TTex* KnobTex = nullptr;
static TTex* VUTex = nullptr;
static TTex* SWTex = nullptr;

IMGUI_API bool          Knob(const char* label, float* value_p, float minv, float maxv);
IMGUI_API void          ToggleButton(const char* str_id, bool* v);
IMGUI_API void          BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);
IMGUI_API void          EndTabBar();
IMGUI_API bool          TabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags = 0);

// FIXME-WIP: Obsolete API
IMGUI_API void          SetTabItemClosed(const char* label);
IMGUI_API void          SetTabItemSelected(const char* label);

IMGUI_API float         VUMeter(const char* id, float value);
IMGUI_API void          AudioView(const char* id, float width, float* values, int length, int pos);
IMGUI_API bool          Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
}

enum ImGuiTabBarFlags_
{
    ImGuiTabBarFlags_None                           = 0,
    ImGuiTabBarFlags_NoAnim                         = 1 << 0,   // Disable horizontal and vertical sliding animations
    ImGuiTabBarFlags_NoReorder                      = 1 << 1,
    ImGuiTabBarFlags_NoCloseWithMiddleMouseButton   = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    ImGuiTabBarFlags_NoResetOrderOnAppearing        = 1 << 3,
    ImGuiTabBarFlags_NoSelectionOnAppearing         = 1 << 4,   // Do not become automatically selected when appearing
    ImGuiTabBarFlags_SizingPolicyFit                = 1 << 5,
    ImGuiTabBarFlags_SizingPolicyEqual              = 1 << 6,

    ImGuiTabBarFlags_SizingPolicyMask_              = ImGuiTabBarFlags_SizingPolicyFit | ImGuiTabBarFlags_SizingPolicyEqual,
    ImGuiTabBarFlags_SizingPolicyDefault_           = ImGuiTabBarFlags_SizingPolicyFit
};

enum ImGuiTabItemFlags_
{
    ImGuiTabItemFlags_None                          = 0,
    ImGuiTabItemFlags_UnsavedDocument               = 1 << 0,   // Automatically append an '*' to the label without affecting the ID, as a convenience to avoid using ### operator everywhere.
    ImGuiTabItemFlags_SetSelected                   = 1 << 1    // Trigger flag to programatically make the tab selected when calling TabItem()
};

#endif // IMGUI_USER_H