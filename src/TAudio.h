#ifndef T_AUDIO_H
#define T_AUDIO_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

#include "editor/imgui/imgui.h"
#include "editor/imgui/imgui_impl.h"

#include <functional>
#include <iostream>
#include <optional>
#include <cmath>

#define TAUDIO_DEFAULT_SAMPLERATE 44100
#define TAUDIO_DEFAULT_SAMPLES 2048
#define TAUDIO_DEFAULT_CHANNELS 1

#define FBUFFER(x) ((float*)x)

class TAudio;
using GUICallback = std::function<void(TAudio*, int, int)>;

class TAudio {
public:
	TAudio(
		SDL_AudioCallback callback,
		void* udata = nullptr,
		int sampleRate = TAUDIO_DEFAULT_SAMPLERATE,
		int samples = TAUDIO_DEFAULT_SAMPLES,
		int channels = TAUDIO_DEFAULT_CHANNELS
	);
	void destroy();

	void guiCallback(const GUICallback& cb) { m_guiCallback = cb; }

	bool shouldClose() const { return m_shouldClose; }
	void sync();

	SDL_AudioSpec spec() const { return m_spec; }
	SDL_Window* window() { return m_window; }

private:
	SDL_GLContext m_context;
	SDL_Window* m_window;

	SDL_AudioDeviceID m_device;
	SDL_AudioSpec m_spec;

	GUICallback m_guiCallback;

	bool m_shouldClose;
};


#endif // T_AUDIO_H
