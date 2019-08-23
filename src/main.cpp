#include <iostream>
#include <cstring>
#include <memory>

#include "twen/intern/Log.h"
#include "twen/Twen.h"
#include "editor/TApplication.h"
#include "editor/TNodeEditor.h"

#ifdef WINDOWS
#include <windows.h>
#endif

static void audioCallback(void* app, Uint8* stream, int length);

class App : public TApplication {
public:
	App(const std::string& fileName="")
	{
		m_editor = new TNodeEditor(fileName);
		init(audioCallback, m_editor);
		m_editor->sampleRate = spec().freq;
	}

	virtual ~App() {}

	void gui(int w, int h) {
		m_editor->draw(w, h);
	}

	void input(SDL_Event e) {
		if (e.type == SDL_QUIT) {
			m_editor->menuActionExit();
		}
	}

	void start() {
		while (!shouldClose()) {
			sync();
			if (m_editor->exit()) {
				this->exit();
			}
		}
		destroy();
		delete m_editor;
	}

	TNodeEditor* editor() { return m_editor; }

private:
	TNodeEditor* m_editor;
};

static void audioCallback(void* ud, Uint8* stream, int length) {
	TNodeEditor* editor = static_cast<TNodeEditor*>(ud);
	int flen = length / int(sizeof(float));
	float* fstream = reinterpret_cast<float*>(stream);

	if (editor != nullptr) {
		for (int i = 0; i < flen; i++) {
			fstream[i] = editor->output();
		}
	}
}

int main(int argc, char** argv) {
	srand(u32(time(nullptr)));

	Twen::init();

	App* app = new App(argc > 1 ? std::string(argv[1]) : "");
#ifdef WINDOWS
	FreeConsole();
#endif
	app->start();
	delete app;

	return 0;
}
