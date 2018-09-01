#ifndef T_COMMANDS_H
#define T_COMMANDS_H

#include "TNodeGraph.h"
#include "nodes/TNode.h"

class TAddNodeCommand : public TCommand {
public:
	TAddNodeCommand(int id, int x, int y, const std::string& type, JSON params)
		: m_params(params), m_type(type), m_x(x), m_y(y), m_id(id)
	{}

	void execute() {
		m_id = m_nodeGraph->addNode(m_x, m_y, m_type, m_params, -1, false)->id();
	}

	void revert() {
		m_nodeGraph->deleteNode(m_id, false);
	}

private:
	JSON m_params;
	std::string m_type;
	int m_x, m_y;
	int m_id;
};

class TDeleteNodeCommand : public TCommand {
public:
	TDeleteNodeCommand(int id, int x, int y, const std::string& type, JSON params)
		: m_params(params), m_type(type), m_x(x), m_y(y), m_id(id)
	{}

	void execute() {
		m_nodeGraph->deleteNode(m_id, false);
	}

	void revert() {
		m_id = m_nodeGraph->addNode(m_x, m_y, m_type, m_params, -1, false)->id();
	}

private:
	JSON m_params;
	std::string m_type;
	int m_x, m_y;
	int m_id;
};

class TLinkCommand : public TCommand {
public:
	TLinkCommand(int id, int inID, int inSlot, int outID, int outSlot)
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
	int m_linkID;
	int inputID, inputSlot;
	int outputID, outputSlot;
};

class TUnLinkCommand : public TCommand {
public:
	TUnLinkCommand(int id, int inID, int inSlot, int outID, int outSlot)
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
	int m_linkID;
	int inputID, inputSlot;
	int outputID, outputSlot;
};

class TMoveCommand : public TCommand {
public:
	struct Point { Point(){} Point(int x, int y) : x(x), y(y) {} int x, y; };

	TMoveCommand(const std::vector<int>& ids, const std::map<int, Point>& deltas)
		: ids(ids), deltas(deltas)
	{}

	void execute();
	void revert();

private:
	std::vector<int> ids;
	std::map<int, Point> deltas;
};

#endif // T_COMMANDS_H