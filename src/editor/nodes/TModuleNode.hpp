#ifndef T_MODULE_NODE
#define T_MODULE_NODE

#include "../TNodeGraph.h"
#include "OsDialog.hpp"

class TModuleNode : public TNode {
public:
	TModuleNode() : TNode("Module", 0, 0) {

	}

	void gui() {
		if (ImGui::Button(!nodeGraph ? "Load" : "Reload")) {
			if (nodeGraph) {
				load(filePath);
			} else {
				auto filePath = osd::Dialog::file(
					osd::DialogAction::OpenFile,
					".",
					osd::Filters("Twist Node-Graph:tng")
				);

				if (filePath.has_value()) {
					this->filePath = filePath.value();
					load(this->filePath);
				}
			}
		}
	}

	void solve() {
		if (nodeGraph && !reloading) {
			for (int i = 0; i < m_inputs.size(); i++) {
				inputs->setOutput(i, getInput(i));
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
		
		if (nodeGraph) {
			nodeGraph.reset();
		}
		nodeGraph = std::unique_ptr<TNodeGraph>(new TNodeGraph());
		nodeGraph->load(fname);
		nodeGraph->editor(parent()->editor());

		m_inputs.clear();
		m_outputs.clear();

		if (nodeGraph->type() != TNodeGraph::Module) {
			osd::Dialog::message(
				osd::MessageLevel::Error,
				osd::MessageButtons::Ok,
				"The selected node graph is not a Module."
			);
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