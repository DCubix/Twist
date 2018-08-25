#ifndef T_NODE_GRAPH_H
#define T_NODE_GRAPH_H

#include <iostream>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>

#include "nodes/TNode.h"
#include "../TGen.h"

using TNodeCtor = TNode*(JSON&);

using TNodePtr = std::unique_ptr<TNode>;
using TNodeFactories = std::map<std::string, TNodeCtor*>;
using TNodeList = std::map<int, TNodePtr>;
using TLinkList = std::vector<std::unique_ptr<TLink>>;
using TIntList = std::vector<int>;

#define GLOBAL_STORAGE_SIZE 32

#define GET(type, v, d) (json[v].is_null() ? d : json[v].get<type>())

class TInputsNode : public TNode {
public:
	TInputsNode() : TNode("Inputs", 0, 0) {
		
	}

	void gui() {
		if (ImGui::Button("Add", ImVec2(130, 18))) {
			addOutput("Output");
		}

		int toRemove = -1;
		for (int i = 0; i < m_outputs.size(); i++) {
			TValue& v = m_outputs[i];
			char id[8] = "##val_";
			id[6] = i + '0';
			id[7] = 0;

			ImGui::PushItemWidth(130 - 18);
			ImGui::InputText(id, v.label.data(), v.label.size());
			ImGui::PopItemWidth();
			ImGui::SameLine();

			char lbl[8] = "x##_bt";
			lbl[6] = i + '0';
			lbl[7] = 0;
			if (ImGui::Button(lbl, ImVec2(18, 18))) {
				toRemove = i;
			}
			if (v.connected && ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Please disconnect this first!");
				toRemove = -1;
			}
		}

		if (toRemove != -1) {
			removeOutput(toRemove);
		}
	}

	void solve() {
	}

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		for (int i = 0; i < m_outputs.size(); i++) {
			json["inputs"][i] = m_outputs[i].label;
		}
	}

	static std::string type() { return "Inputs"; }
};

class TOutputsNode : public TNode {
public:
	TOutputsNode() : TNode("Outputs", 0, 0) {
		
	}

	void gui() {
		if (ImGui::Button("Add", ImVec2(130, 18))) {
			addInput("Input");
		}

		int toRemove = -1;
		for (int i = 0; i < m_inputs.size(); i++) {
			TValue& v = m_inputs[i];
			char id[8] = "##val_";
			id[6] = i + '0';
			id[7] = 0;

			ImGui::PushItemWidth(130 - 18);
			ImGui::InputText(id, v.label.data(), v.label.size());
			ImGui::PopItemWidth();
			ImGui::SameLine();

			char lbl[8] = "x##_bt";
			lbl[6] = i + '0';
			lbl[7] = 0;
			if (ImGui::Button(lbl, ImVec2(18, 18))) {
				toRemove = i;
			}
			if (v.connected && ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Please disconnect this first!");
				toRemove = -1;
			}
		}

		if (toRemove != -1) {
			removeInput(toRemove);
		}
	}

	void solve() { }

	void save(JSON& json) {
		TNode::save(json);
		json["type"] = type();
		for (int i = 0; i < m_inputs.size(); i++) {
			json["outputs"][i] = m_inputs[i].label;
		}
	}

	static std::string type() { return "Outputs"; }
};

class TNodeFactory {
public:
	template <class T>
	static void registerNode(TNodeCtor* ctor) {
		if (factories.find(T::type()) != factories.end()) {
			return;
		}
		factories[T::type()] = ctor;
		
	}

	template <class T>
	static T* create(const std::string& name, JSON& params) {
		return (T*) factories[name](params);
	}

	template <class T>
	static T* createNew(const std::string& name) {
		JSON js;
		return (T*) factories[name](js);
	}

	static TNodeFactories factories;
};

struct TSampleLibEntry {
	std::vector<float> sampleData;
	float sampleRate, duration;
	std::string name;
};

class TNodeEditor;
class TNodeGraph {
	friend class TNodeEditor;
public:
	TNodeGraph();

	enum TGraphType {
		Normal = 0,
		Module
	};

	TNode* addNode(int x, int y, const std::string& type);
	TNode* addNode(int x, int y, const std::string& type, JSON& params, int id = -1);
	void deleteNode(int id);
	void link(int inID, int inSlot, int outID, int outSlot);

	TNodeList& nodes() { return m_nodes; }
	TLinkList& links() { return m_links; }
	void removeLink(int id);

	TNode* node(int id);
	float solve();

	int outputNode() const { return m_outputNode; }
	void outputNode(int id) { m_outputNode = id; }

	// Module type variables
	int inputsNode() const { return m_inputsNode; }
	int outputsNode() const { return m_outputsNode; }
	void inputsNode(int v) { m_inputsNode = v; }
	void outputsNode(int v) { m_outputsNode = v; }

	TGraphType type() const { return m_type; }

	void store(int loc, float value) { m_globalStorage[loc] = value; }
	float load(int loc) const { return m_globalStorage[loc]; }

	void load(const std::string& fileName);
	void save(const std::string& fileName);

	bool addSample(const std::string& fileName);
	void removeSample(int id);
	int getSampleID(const std::string& name);
	TSampleLibEntry* getSample(int id);
	std::vector<const char*> getSampleNames();

	TNodeEditor* editor() { return m_editor; }
	void editor(TNodeEditor* ed) { m_editor = ed; }

	std::string name() const { return m_name; }

	void solveNodes();

protected:
	void addSample(const std::string& fname, const std::vector<float>& data, float sr, float dur);
	int getID();
	TIntList getAllLinksRelatedToNode(int id);
	TIntList getNodeInputs(int id);
	TIntList buildNodes(int id);
	TIntList buildNodes(const TIntList& ids);
	void solveNodes(const TIntList& solved);

	TNodeEditor* m_editor;

	int m_outputNode = 0, m_inputsNode = 0, m_outputsNode = 0;

	TNodeList m_nodes;
	TLinkList m_links;
	std::mutex m_lock;

	TIntList m_solvedNodes;

	ImVec2 m_scrolling;

	TGraphType m_type;
	std::string m_name, m_fileName;

	std::array<float, GLOBAL_STORAGE_SIZE> m_globalStorage;
	std::map<int, std::unique_ptr<TSampleLibEntry>> m_sampleLibrary;

	bool m_open = true, m_saved = false;
};

#endif // T_NODE_GRAPH_H