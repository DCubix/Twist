#ifndef T_COMMANDS_H
#define T_COMMANDS_H

#include "twen/Node.h"
#include "TUndoRedo.h"
#include "TNodeGraph.h"
#include "twen/NodeGraph.h"

class TAddNodeCommand : public TCommand {
public:
	TAddNodeCommand(u64 id, int x, int y, const Str& type, JSON params)
		: m_params(params), m_type(type), m_x(x), m_y(y), m_id(id)
	{}

	void execute() {
		m_id = m_nodeGraph->addNode(m_x, m_y, m_type, m_params, false)->node->id();
	}

	void revert() {
		m_nodeGraph->deleteNode(m_id, false);
	}

private:
	JSON m_params;
	Str m_type;
	int m_x, m_y;
	u64 m_id;
};

class TDeleteNodeCommand : public TCommand {
public:
	TDeleteNodeCommand(u64 id, int x, int y, const std::string& type, JSON params)
		: m_params(params), m_type(type), m_x(x), m_y(y), m_id(id)
	{}

	void execute() {
		m_nodeGraph->deleteNode(m_id, false);
	}

	void revert() {
		m_id = m_nodeGraph->addNode(m_x, m_y, m_type, m_params, false)->node->id();
	}

private:
	JSON m_params;
	std::string m_type;
	int m_x, m_y;
	u64 m_id;
};

class TLinkCommand : public TCommand {
public:
	TLinkCommand(u64 id, u64 inID, u32 inSlot, u64 outID, u32 outSlot)
	 : m_linkID(id), inputID(inID), inputSlot(inSlot),
		outputID(outID), outputSlot(outSlot)
	{}

	void execute() {
		m_linkID = m_nodeGraph->link(
			inputID, inputSlot,
			outputID, outputSlot,
			false
		);
	}

	void revert() {
		m_nodeGraph->removeLink(m_linkID, false);
	}

private:
	u64 m_linkID;
	u64 inputID;
	u64 outputID;
	u32 inputSlot;
	u32 outputSlot;
};

class TUnLinkCommand : public TCommand {
public:
	TUnLinkCommand(u64 id, u64 inID, u32 inSlot, u64 outID, u32 outSlot)
	 : m_linkID(id), inputID(inID), inputSlot(inSlot),
		outputID(outID), outputSlot(outSlot)
	{}

	void execute() {
		m_nodeGraph->removeLink(m_linkID, false);
	}

	void revert() {
		m_linkID = m_nodeGraph->link(
			inputID, inputSlot,
			outputID, outputSlot,
			false
		);
	}

private:
	u64 m_linkID;
	u64 inputID;
	u64 outputID;
	u32 inputSlot;
	u32 outputSlot;
};

class TMoveCommand : public TCommand {
public:
	struct Point { Point(){} Point(int x, int y) : x(x), y(y) {} int x, y; };

	TMoveCommand(const Vec<u64>& ids, const Map<u64, Point>& deltas)
		: ids(ids), deltas(deltas)
	{}

	void execute();
	void revert();

private:
	Vec<u64> ids;
	Map<u64, Point> deltas;
};

class TParamChangeCommand : public TCommand {
public:
	TParamChangeCommand(
			u64 nodeID, u32 index,
			float floatValue, u32 optionValue,
			float oldValue, u32 oldOption
	)
		: m_nodeID(nodeID), m_index(index),
		  m_value(floatValue), m_option(optionValue),
		  m_oldValue(oldValue), m_oldOption(oldOption)
	{}

	void execute() {
		Node* node = m_nodeGraph->actualNodeGraph()->get<Node>(m_nodeID);
		NodeParam& param = node->params()[m_index];
		param.value = m_value;
		param.option = m_option;
	}

	void revert() {
		Node* node = m_nodeGraph->actualNodeGraph()->get<Node>(m_nodeID);
		NodeParam& param = node->params()[m_index];
		param.value = m_oldValue;
		param.option = m_oldOption;
	}

private:
	u64 m_nodeID;
	u32 m_index;
	float m_value, m_oldValue;
	u32 m_option, m_oldOption;
};

#endif // T_COMMANDS_H
