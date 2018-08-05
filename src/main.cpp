#include <iostream>
#include <cstring>
#include <memory>

#include "TAudio.h"
#include "TGen.h"
#include "editor/TNode.h"

TNodeEditor* g_Editor;

void audioCallback(void* audio, Uint8* stream, int length) {
	int flen = length / sizeof(float);
	float* fstream = FBUFFER(stream);
	TAudio* aud = (TAudio*) audio;

	g_Editor->solveNodes();
	for (int i = 0; i < flen; i++) {
		fstream[i] = g_Editor->output();
	}

	std::memcpy(g_Editor->outNode()->waveForm, fstream, WAVEFORM_LENGTH * sizeof(float));
}

int main() {
	srand(time(0));
	TAudio aud(audioCallback);

	float sr = aud.spec().freq;
	
	g_Editor = new TNodeEditor();
	g_Editor->sampleRate = sr;
	
	aud.guiCallback([&](TAudio* au) {
		g_Editor->draw(au->ctx());
	});

	while (!aud.shouldClose()) {
		aud.sync();
	}
	aud.destroy();
	delete g_Editor;
	return 0;
}