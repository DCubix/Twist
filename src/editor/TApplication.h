#ifndef T_APPLICATION_H
#define T_APPLICATION_H

#if __has_include("SDL.h")
#include "SDL.h"
#include "SDL_audio.h"
#else
#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"
#endif

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl.h"

#include <functional>
#include <iostream>
#include <optional>

#define _USE_MATH_DEFINES
#include <cmath>

#define TWIST_DEFAULT_SAMPLERATE 44100
#define TWIST_DEFAULT_SAMPLES 1024
#define TWIST_DEFAULT_CHANNELS 1

class TApplication {
public:
	TApplication() {}

	void init(
		SDL_AudioCallback callback,
		void* udata = nullptr,
		int sampleRate = TWIST_DEFAULT_SAMPLERATE,
		int samples = TWIST_DEFAULT_SAMPLES,
		int channels = TWIST_DEFAULT_CHANNELS
	);
	void destroy();

	void exit() { m_shouldClose = true; }
	bool shouldClose() const { return m_shouldClose; }
	void sync();

	virtual void gui(int width, int height) {}
	virtual void input(SDL_Event evt) {}

	SDL_AudioSpec spec() const { return m_spec; }
	SDL_Window* window() { return m_window; }

private:
	void setupIcon();

	SDL_GLContext m_context;
	SDL_Window* m_window;

	SDL_AudioDeviceID m_device;
	SDL_AudioSpec m_spec;

	bool m_shouldClose;
};


#endif // T_APPLICATION_H
