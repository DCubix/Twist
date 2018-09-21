#include <iostream>
#include <cstring>
#include <memory>

#include "twen/intern/Log.h"
#include "twen/Twen.h"
#include "editor/TApplication.h"
#include "editor/TNodeEditor.h"

#include "sndfile.hh"

static void audioCallback(void* app, Uint8* stream, int length);

class App : public TApplication {
public:
	App()
	{
		m_editor = new TNodeEditor();
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
			if (!editor->rendering()) fstream[i] = editor->output();
		}
	}
}

int main() {
	srand(u32(time(nullptr)));
	
	Twen::init();

#if 0
	Arr<float, 44100> sine;

	Ptr<NodeGraph> graph = Ptr<NodeGraph>(new NodeGraph());
	
	u64 out = graph->create<OutNode>();
	u64 osc = graph->create<OscillatorNode>(44100.0f, Oscillator::Sine, 220.0f, 1.0f);
	u64 osc2 = graph->create<OscillatorNode>(44100.0f, Oscillator::Sine, 2.0f, 40.0f);
	u64 mat = graph->create<MathNode>(MathNode::Add);
	u64 vol = graph->create<ValueNode>(0.5f);
	u64 freq = graph->create<ValueNode>(220.0f);

	graph->link(osc2, "Out", mat, "A");
	graph->link(freq, "Out", mat, "B");

	graph->link(mat, "Out", osc, "Freq");
	graph->link(osc, "Out", out, "In");
	graph->link(vol, "Out", out, "Vol.");

	for (int i = 0; i < sine.size(); i++) {
		sine[i] = graph->solve();
	}

	int fmt = SF_FORMAT_WAV | SF_FORMAT_PCM_32;
	SndfileHandle snd = SndfileHandle("test.wav", SFM_WRITE, fmt, 1, 44100);
	snd.writef(sine.data(), sine.size());
#endif

	App* app = new App();
	app->start();
	delete app;

	return 0;
}
