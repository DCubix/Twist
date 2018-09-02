#include "TAudio.h"

#include "glad/glad.h"
#include "editor/TMidi.h"

#include "icon.h"
#include "stb/stb_image.h"

TAudio::TAudio(SDL_AudioCallback callback, void* udata, int sampleRate, int samples, int channels) {
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << SDL_GetError() << std::endl;
		return;
	}

	SDL_AudioSpec spec;
	spec.freq = sampleRate;
	spec.samples = samples;
	spec.channels = channels;
	spec.callback = callback;
	spec.userdata = udata;
	spec.format = AUDIO_F32;

	if ((m_device = SDL_OpenAudioDevice(NULL, 0, &spec, &m_spec, 0)) < 0) {
		std::cout << SDL_GetError() << std::endl;
		return;
	}

	std::cout << "SAMPLE RATE: " << m_spec.freq << std::endl;
	std::cout << "SAMPLES: " << m_spec.samples << std::endl;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	m_window = SDL_CreateWindow(
		TWIST_NAME,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 640, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	setupIcon();

	m_context = SDL_GL_CreateContext(m_window);
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		std::cout << SDL_GetError() << std::endl;
		SDL_Quit();
		return;
	}

	m_shouldClose = false;

	SDL_PauseAudioDevice(m_device, 0);

	ImGuiSystem::Init(m_window);
}

void TAudio::setupIcon() {
	int w, h, comp;
	unsigned char* data = stbi_load_from_memory(twist_png, twist_png_len, &w, &h, &comp, STBI_rgb_alpha);
	if (data == nullptr) {
		return;
	}

	int depth, pitch;
	Uint32 pixel_format;
	if (comp == STBI_rgb) {
		depth = 24;
		pitch = 3 * w;
		pixel_format = SDL_PIXELFORMAT_RGB24;
	} else {
		depth = 32;
		pitch = 4 * w;
		pixel_format = SDL_PIXELFORMAT_RGBA32;
	}

	SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
		(void*)data, w, h, depth, pitch, pixel_format
	);

	if (surf != nullptr) {
		SDL_SetWindowIcon(m_window, surf);
	}

	stbi_image_free(data);
	if (surf != nullptr) SDL_FreeSurface(surf);
}

void TAudio::destroy() {
	ImGuiSystem::Shutdown();
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
	SDL_CloseAudioDevice(m_device);
	SDL_Quit();
}

void TAudio::sync(const InputCallback& cb) {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		if (cb) cb(this, e);

		/// Keyboard Input
		static const int KEYS[] = {
			SDLK_q, SDLK_2, SDLK_w, SDLK_3, SDLK_e, SDLK_r, SDLK_5, SDLK_t, SDLK_6, SDLK_y, SDLK_7, SDLK_u, SDLK_i,
			SDLK_z, SDLK_s, SDLK_x, SDLK_d, SDLK_c, SDLK_v, SDLK_g, SDLK_b, SDLK_h, SDLK_n, SDLK_j, SDLK_m, SDLK_COMMA
		};
		static const int NOTES[] = {
			36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
			24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36
		};
		switch (e.type) {
			case SDL_KEYDOWN: {
				for (int i = 0; i < IM_ARRAYSIZE(KEYS); i++) {
					if (e.key.keysym.sym == KEYS[i] && e.key.repeat == 0) {
						TMessageBus::broadcast(0xF, TMidiCommand::NoteOn, NOTES[i], 127);
					}
				}
			} break;
			case SDL_KEYUP: {
				for (int i = 0; i < IM_ARRAYSIZE(KEYS); i++) {
					if (e.key.keysym.sym == KEYS[i] && e.key.repeat == 0) {
						TMessageBus::broadcast(0xF, TMidiCommand::NoteOff, NOTES[i], 0);
					}
				}
			} break;
		}

		ImGuiSystem::ProcessEvent(&e);
	}

	int ww, wh;
	SDL_GetWindowSize(m_window, &ww, &wh);

	ImGuiSystem::NewFrame();
	// GUI
	if (m_guiCallback) {
		m_guiCallback(this, ww, wh);
	}

	// Draw
	glViewport(0, 0, ww, wh);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGuiSystem::Render();
	SDL_GL_SwapWindow(m_window);
}