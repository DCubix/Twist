#ifndef T_MODULE_NODE
#define T_MODULE_NODE

#include "../TNodeGraph.h"
#include "../tinyfiledialogs.h"

class TModuleNode : public TNode {
public:
	TModuleNode() : TNode("Module", 0, 0) {

	}

	void gui() {
		if (ImGui::Button(!nodeGraph ? "Load" : "Reload")) {
			if (nodeGraph) {
				load(filePath);
			} else {
				const static char* FILTERS[] = { "*.tng\0" };
				const char* filePath = tinyfd_openFileDialog(
					"Open",
					"",
					1,
					FILTERS,
					"Twist Node-Graph",
					0
				);
				if (filePath) {
					this->filePath = std::string(filePath);
					load(this->filePath);
				}
			}
		}
	}

	void solve() {
		if (nodeGraph && !reloading) {
			for (int i = 0; i < m_inputs.size(); i++) {
				inputs->setOutput(i, getInputOr(i, 0.0f));
			}
			nodeGraph->solve();
			for (int i = 0; i < m_outputs.size(); i++) {
				setOutput(i, outputs->getInput(i));
			}
		}
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		json["filePath"] = filePath;
	}

	void load(const std::string& fname) {
		if (fname.empty()) return;
		reloading = true;
		
		if (!nodeGraph)
			nodeGraph = std::unique_ptr<TNodeGraph>(new TNodeGraph());
		nodeGraph->load(fname);

		m_inputs.clear();
		m_outputs.clear();

		if (nodeGraph->type() != TNodeGraph::Module) {
			tinyfd_messageBox("Error", "The selected node graph is not a Module.", "ok", "error", 1);
			nodeGraph.reset();
		} else {
			m_title = nodeGraph->name();
			inputs = nodeGraph->node(nodeGraph->inputsNode());
			outputs = nodeGraph->node(nodeGraph->outputsNode());
			if (inputs != nullptr) {
				for (auto& in : inputs->outputs()) {
					addInput(in.label);
				}
			}
			if (outputs != nullptr) {
				for (auto& out : outputs->inputs()) {
					addOutput(out.label);
				}
			}
		}

		reloading = false;
	}

	static std::string type() { return "Module"; }

	std::string filePath;

	std::unique_ptr<TNodeGraph> nodeGraph;
	TNode *inputs, *outputs;
	bool reloading = false;
};

#endif // T_MODULE_NODE