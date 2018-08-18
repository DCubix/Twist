#include <iostream>
#include <cstring>
#include <memory>

#include "TAudio.h"
#include "TGen.h"
#include "editor/TNodeEditor.h"

static void audioCallback(void* app, Uint8* stream, int length);

class App {
public:

	App() {
		m_sys = new TAudio(audioCallback, this);
		m_editor = new TNodeEditor();
		m_editor->sampleRate = m_sys->spec().freq;

		m_sys->guiCallback([this](TAudio* au, int w, int h) {
			m_editor->draw(w, h);
		});
	}

	void start() {
		while (!m_sys->shouldClose()) {
			m_sys->sync();
		}
		m_sys->destroy();
		delete m_editor;
	}

	TNodeEditor* editor() { return m_editor; }

private:
	TAudio* m_sys;
	TNodeEditor* m_editor;
};

static void audioCallback(void* ud, Uint8* stream, int length) {
	App* app = static_cast<App*>(ud);
	int flen = length / sizeof(float);
	float* fstream = FBUFFER(stream);

	if (app->editor() != nullptr) {
		for (int i = 0; i < flen; i++) {
			if (!app->editor()->rendering()) fstream[i] = app->editor()->output();
		}

		std::memcpy(app->editor()->outNode()->waveForm, fstream, WAVEFORM_LENGTH * sizeof(float));
	}
}

int main() {
	srand(time(0));	
	
	App* app = new App();
	app->start();
	delete app;

	return 0;
}