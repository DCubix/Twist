#include "TAudio.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#include "glad/glad.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

TAudio::TAudio(SDL_AudioCallback callback, int sampleRate, int samples, int channels) {
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
	spec.userdata = this;
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
		"TAudio",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 640, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
	);

	m_context = SDL_GL_CreateContext(m_window);
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		std::cout << SDL_GetError() << std::endl;
		SDL_Quit();
		return;
	}

	m_ctx = nk_sdl_init(m_window);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();

	m_shouldClose = false;

	SDL_PauseAudioDevice(m_device, 0);
}

void TAudio::destroy() {
	nk_sdl_shutdown();
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
	SDL_CloseAudioDevice(m_device);
	SDL_Quit();
}

void TAudio::sync() {
	SDL_Event e;

	nk_input_begin(m_ctx);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			m_shouldClose = true;
		}

		nk_sdl_handle_event(&e);
	}
	nk_input_end(m_ctx);

	// GUI
	if (m_guiCallback) {
		m_guiCallback(this);
	}

	// Draw
	int ww, wh;
	SDL_GetWindowSize(m_window, &ww, &wh);
	glViewport(0, 0, ww, wh);
	glClear(GL_COLOR_BUFFER_BIT);
	nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
	SDL_GL_SwapWindow(m_window);
}