#ifndef IMGUI_IMPL_H
#define IMGUI_IMPL_H

struct SDL_Window;
typedef union SDL_Event SDL_Event;

namespace ImGuiSystem {
	IMGUI_API bool Init(SDL_Window* window);
	IMGUI_API void Shutdown();
	IMGUI_API void NewFrame(int ww = 0, int wh = 0);
	IMGUI_API void Render();
	IMGUI_API bool ProcessEvent(SDL_Event* event);

	// Use if you want to reset your rendering device without losing ImGui state.
	IMGUI_API void InvalidateDeviceObjects();
	IMGUI_API bool CreateDeviceObjects();
}

#endif // IMGUI_IMPL_H