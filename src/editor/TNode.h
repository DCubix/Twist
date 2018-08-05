#ifndef T_NODE_H
#define T_NODE_H

#include <iostream>
#include <cstdarg>
#include "../nuk.h"
#include "nk_ext.h"

#include "../TGen.h"

#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

using NkRect = struct nk_rect;
using NkContext = struct nk_context;
using NkCanvas = struct nk_command_buffer;
using NkColor = struct nk_color;
using NkVec2 = struct nk_vec2;

struct TValue {
	float value;
	std::string label;

	TValue() {}
	TValue(const std::string& label) : label(label), value(0) {}
};

class TNodeEditor;
class TNode {
	friend class TNodeEditor;
public:
	TNode() : m_bounds(nk_rect(0, 0, 100, 150)), m_title("TNode") {}
	TNode(const std::string& title, int width, int height)
		 : m_bounds(nk_rect(0, 0, width, height)), m_title(title)
	{}
	virtual ~TNode();

	virtual void _gui(NkContext* ctx, NkCanvas* canvas) = 0;

	void draw(NkContext* ctx, NkCanvas* canvas);

	const NkRect& bounds() const { return m_bounds; }
	NkRect& bounds() { return m_bounds; }

	std::vector<TValue> inputs() const { return m_inputs; }
	std::vector<TValue> outputs() const { return m_outputs; }

	void setOutput(int id, float value) { m_outputs[id].value = value; }
	void setInput(int id, float value) { m_inputs[id].value = value; }

	float getOutput(int id) { return m_outputs[id].value; }
	float getInput(int id) { return m_inputs[id].value; }

	virtual void solve() = 0;

	int id() const { return m_id; }

protected:
	void addInput(const std::string& label);
	void addOutput(const std::string& label);

	int m_id;
	std::string m_title;
	NkRect m_bounds;

	std::vector<TValue> m_inputs;
	std::vector<TValue> m_outputs;
};

struct TLink {
	int inputID, inputSlot;
	int outputID, outputSlot;
};

struct TLinking {
	int inputID, inputSlot;
	bool active;
	TNode* node;
};

static float lerp(float a, float b, float t);
static float remap(float value, float from1, float to1, float from2, float to2);

class TValueNode : public TNode {
public:
	TValueNode() : TNode("Value", 140, 90), value(0) {
		addOutput("Out");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 22, 1);
		value = nk_propertyf(ctx, "#Value", -9999.0f, value, 9999.0f, 0.01f, 0.01f);
	}

	void solve() {
		setOutput(0, value);
	}

	float value;
};

class TVec4Node : public TNode {
public:
	TVec4Node() : TNode("Vector4", 140, 140), x(0), y(0), z(0), w(0) {
		addOutput("X");
		addOutput("Y");
		addOutput("Z");
		addOutput("W");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 18, 1);
		x = nk_propertyf(ctx, "#X", -9999.0f, x, 9999.0f, 0.01f, 0.01f);
		y = nk_propertyf(ctx, "#Y", -9999.0f, y, 9999.0f, 0.01f, 0.01f);
		z = nk_propertyf(ctx, "#Z", -9999.0f, z, 9999.0f, 0.01f, 0.01f);
		w = nk_propertyf(ctx, "#W", -9999.0f, w, 9999.0f, 0.01f, 0.01f);
	}

	void solve() {
		setOutput(0, x);
		setOutput(1, y);
		setOutput(2, z);
		setOutput(3, w);
	}

	float x, y, z, w;
};

class TMixNode : public TNode {
public:
	TMixNode() : TNode("Mix", 140, 90), factor(0.5f) {
		addInput("A");
		addInput("B");
		addOutput("Out");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 22, 1);
		factor = nk_propertyf(ctx, "#Factor", 0.0f, factor, 1.0f, 0.1f, 0.1f);
	}

	void solve() {
		float a = getInput(0);
		float b = getInput(1);
		setOutput(0, (a + b) * factor);
	}

	float factor;
};

#define WAVEFORM_LENGTH 96
class TOutNode : public TNode {
public:
	TOutNode() : TNode("Output", 160, 140), volume(1.0f) {
		addInput("In");
		std::memset(waveForm, 0, WAVEFORM_LENGTH * sizeof(float));
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 70, 1);
		if (nk_chart_begin(ctx, NK_CHART_COLUMN, WAVEFORM_LENGTH, -1.0f, 1.0f)) {
			for (int i = 0; i < WAVEFORM_LENGTH; i++) {
				nk_chart_push(ctx, waveForm[i]);
			}
			nk_chart_end(ctx);
		}
		nk_layout_row_dynamic(ctx, 16, 1);
		volume = nk_slide_float(ctx, 0.0f, volume, 1.0f, 0.01f);
	}

	void solve() {
	}

	float volume;
	float waveForm[WAVEFORM_LENGTH];
};

class TOscillatorNode : public TNode {
public:
	TOscillatorNode(float sampleRate)
		: TNode("Oscillator", 160, 145),
			m_osc(new TOsc(sampleRate)),
			value(0.0f), wf(TOsc::Sine)
	{
		addInput("Freq");
		addInput("Amp");
		addOutput("Out");
	}

	virtual ~TOscillatorNode() override {
		delete m_osc;
		m_osc = nullptr;
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 24, 2);
		if (nk_option_label(ctx, "Sine", wf == TOsc::Sine)) wf = TOsc::Sine;
		if (nk_option_label(ctx, "Saw", wf == TOsc::Saw)) wf = TOsc::Saw;
		nk_layout_row_dynamic(ctx, 24, 2);
		if (nk_option_label(ctx, "Square", wf == TOsc::Square)) wf = TOsc::Square;
		if (nk_option_label(ctx, "Pulse", wf == TOsc::Pulse)) wf = TOsc::Pulse;
		nk_layout_row_dynamic(ctx, 24, 2);
		if (nk_option_label(ctx, "Triangle", wf == TOsc::Triangle)) wf = TOsc::Triangle;
		if (nk_option_label(ctx, "Noise", wf == TOsc::Noise)) wf = TOsc::Noise;
	}

	void solve() {
		float freq = getInput(0);
		float amp = getInput(1);
		
		m_osc->amplitude(amp);
		m_osc->frequency(freq);
		m_osc->waveForm((TOsc::TWave) wf);

		value = m_osc->sample(freq);

		setOutput(0, value);
	}

	float value;
	int wf;

private:
	TOsc* m_osc;
};

class TNoteNode : public TNode {
public:
	TNoteNode()
		: TNode("Note", 160, 140), note(Notes::C), oct(0)
	{
		addInput("Mod");
		addOutput("Out");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 16, 4);
		if (nk_option_label(ctx, "C", note == Notes::C)) note = Notes::C;
		if (nk_option_label(ctx, "C#", note == Notes::Cs)) note = Notes::Cs;
		if (nk_option_label(ctx, "D", note == Notes::D)) note = Notes::D;
		if (nk_option_label(ctx, "D#", note == Notes::Ds)) note = Notes::Ds;
		if (nk_option_label(ctx, "E", note == Notes::E)) note = Notes::E;
		if (nk_option_label(ctx, "F", note == Notes::F)) note = Notes::F;
		if (nk_option_label(ctx, "F#", note == Notes::Fs)) note = Notes::Fs;
		if (nk_option_label(ctx, "G", note == Notes::G)) note = Notes::G;
		if (nk_option_label(ctx, "G#", note == Notes::Gs)) note = Notes::Gs;
		if (nk_option_label(ctx, "A", note == Notes::A)) note = Notes::A;
		if (nk_option_label(ctx, "A#", note == Notes::As)) note = Notes::As;
		if (nk_option_label(ctx, "B", note == Notes::B)) note = Notes::B;

		nk_layout_row_dynamic(ctx, 22, 1);
		oct = nk_propertyi(ctx, "#Octave", -6, oct, 6, 1, 1);
	}

	void solve() {
		setOutput(0, getInput(0) + NOTE(note) * std::pow(2, oct));
	}

	Notes note;
	int oct;
};

class TADSRNode : public TNode {
public:
	TADSRNode(float sampleRate)
		: TNode("ADSR", 100, 140), m_adsr(new TADSR(0, 0, 1, 0)), trigger(false), sr(sampleRate)
	{
		addOutput("Out");
		addInput("A");
		addInput("D");
		addInput("S");
		addInput("R");
		addInput("Gate");
	}

	virtual ~TADSRNode() override {
		delete m_adsr;
		m_adsr = nullptr;
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
	}

	void solve() {
		if (getInput(4) >= 1.0f) {
			if (!trigger) {
				m_adsr->gate(true);
				trigger = true;
			}
		} else {
			if (trigger) {
				m_adsr->gate(false);
				trigger = false;
			}
		}

		m_adsr->attack(getInput(0) * sr);
		m_adsr->decay(getInput(1) * sr);
		m_adsr->sustain(getInput(2) * sr);
		m_adsr->release(getInput(3) * sr);
		value = m_adsr->sample();
		setOutput(0, value);
	}

	float value, sr;
	bool trigger;

private:
	TADSR* m_adsr;
};

class TMathNode : public TNode {
public:
	enum TMathNodeOp {
		Add = 0,
		Sub,
		Mul,
		Pow
	};

	TMathNode(TMathNodeOp op)
		: TNode("Math", 120, 90), op(op)
	{
		addOutput("Out");
		addInput("In 0");
		addInput("In 1");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 22, 1);
		static const char* OPS[] = {
			"Add\0",
			"Subtract\0",
			"Multiply\0",
			"Power\0"
		};
		op = (TMathNodeOp) nk_combo(ctx, OPS, NK_LEN(OPS), (int) op, 18, nk_vec2(120, 120));
	}

	void solve() {
		float a = getInput(0);
		float b = getInput(1);
		float out = 0.0f;
		switch (op) {
			case Add: out = a + b; break;
			case Sub: out = a - b; break;
			case Mul: out = a * b; break;
			case Pow: out = std::pow(a, b); break;
		}
		setOutput(0, out);
	}

	TMathNodeOp op;
};

class TFilterNode : public TNode {
public:
	enum TFilter {
		LowPass = 0,
		HighPass
	};

	TFilterNode(float sampleRate)
		: TNode("Filter", 120, 90),
		filter(TFilter::LowPass), sampleRate(sampleRate), out(0.0f)
	{
		addInput("In");
		addInput("CutOff");
		addOutput("Out");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 22, 1);
		static const char* OPS[] = {
			"Low-Pass\0",
			"High-Pass\0"
		};
		filter = (TFilter) nk_combo(ctx, OPS, NK_LEN(OPS), (int) filter, 18, nk_vec2(120, 120));
	}

	void solve() {
		float cutOff = std::min(std::max(getInput(1), 20.0f), 20000.0f);
		float in = getInput(0);
		switch (filter) {
			case LowPass: {
				if (cutOff > 0.0f) {
					float dt = 1.0f / sampleRate;
					float a = dt / (dt + 1.0f / (2.0 * M_PI * cutOff));
					out = lerp(out, in, a);
				} else {
					out = 0.0f;
				}
			} break;
			case HighPass: {
				float rc = 1.0f / (2.0f * M_PI * cutOff);
				float a = rc / (rc + 1.0f / sampleRate);

				float result = a * (prev + in);
				prev = result - in;

				out = result;
			} break;
		}
		setOutput(0, out);
	}

	TFilter filter;
	float sampleRate, out, prev;
};

class TButtonNode : public TNode {
public:
	TButtonNode() : TNode("Button", 80, 120), enabled(false) {
		addOutput("Out");
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		nk_layout_row_dynamic(ctx, 60, 1);
		enum nk_symbol_type sym = enabled ? NK_SYMBOL_CIRCLE_SOLID : NK_SYMBOL_CIRCLE_OUTLINE;
		if (nk_button_symbol(ctx, sym)) {
			enabled = !enabled;
		}
	}

	void solve() {
		setOutput(0, enabled ? 1.0f : 0.0f);
	}

	bool enabled;
};

#define SEQUENCER_SIZE 8
class TSequencerNode : public TNode {
public:
	TSequencerNode(float sampleRate)
		: TNode("Sequencer", 410, 150),
			gate(false), noteIndex(0),
			stime(0.0f), sampleRate(sampleRate),
			bpm(120), out(0.0f)
	{
		addInput("Mod");
		addOutput("Freq");
		addOutput("Gate");
		std::memset(notes, 0, sizeof(Notes) * SEQUENCER_SIZE);
		std::memset(octs, 0, sizeof(float) * SEQUENCER_SIZE);
	}

	void _gui(NkContext* ctx, NkCanvas* canvas) {
		static const char* NOTES[] = {
			"C\0",
			"C#\0",
			"D\0",
			"D#\0",
			"E\0",
			"F\0",
			"F#\0",
			"G\0",
			"G#\0",
			"A\0",
			"A#\0",
			"B\0"
		};
		nk_layout_row_dynamic(ctx, 18, SEQUENCER_SIZE);
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			if (i != (noteIndex % SEQUENCER_SIZE)) {
				if (nk_button_symbol(ctx, NK_SYMBOL_RECT_OUTLINE)) {
					noteIndex = i;
				}
			} else {
				nk_button_symbol(ctx, NK_SYMBOL_RECT_SOLID);
			}
		}
		nk_layout_row_dynamic(ctx, 18, SEQUENCER_SIZE);
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			notes[i] = (Notes) nk_combo(ctx, NOTES, NK_LEN(NOTES), (int) notes[i], 18, nk_vec2(42, 100));
		}
		nk_layout_row_dynamic(ctx, 22, SEQUENCER_SIZE);
		for (int i = 0; i < SEQUENCER_SIZE; i++) {
			octs[i] = nk_propertyi(ctx, "#", -8, octs[i], 8, 1, 1);
		}
		nk_layout_row_dynamic(ctx, 22, 1);
		bpm = nk_propertyf(ctx, "BPM", 40.0f, bpm, 240.0f, 0.5f, 0.5f);
	}

	void solve() {
		const float step = (1.0f / sampleRate) * 4;
		float delay = (60000.0f / bpm) / 1000.0f;
		stime += step;
		if (stime >= delay) {
			int ni = noteIndex;
			out = NOTE(notes[ni % SEQUENCER_SIZE]) * std::pow(2, octs[ni % SEQUENCER_SIZE]);
			noteIndex++;
			gate = false;
			stime = 0.0f;
		} else {
			gate = true;
		}

		setOutput(0, getInput(0) + out);
		setOutput(1, gate ? 1.0f : 0.0f);
	}

	int noteIndex;
	bool gate;
	float bpm, stime, sampleRate, out;
	Notes notes[SEQUENCER_SIZE];
	float octs[SEQUENCER_SIZE];
};

class TNodeEditor {
public:
	TNodeEditor();
	virtual ~TNodeEditor();
	
	void draw(NkContext* ctx);

	void addNode(int x, int y, TNode* node);
	void deleteNode(TNode* node);
	void link(int inID, int inSlot, int outID, int outSlot);

	float output();

	float sampleRate;

	TOutNode* outNode() { return m_outputNode; }
	void solveNodes();
private:
	std::vector<TNode*> m_nodes;
	std::vector<TLink*> m_links;
	TLinking m_linking;
	
	NkVec2 m_scrolling;

	TNode* m_selected;
	TOutNode* m_outputNode;
	NkRect m_bounds;

	float m_signalDC, m_envelope;

	std::vector<TLink*> getNodeLinks(TNode* node);
	std::vector<TNode*> getNodeInputs(TNode* node);
	std::vector<TNode*> buildNodes(TNode* out);

	std::vector<TNode*> m_solvedNodes;

	void solve();
};

#endif // T_NODE_H