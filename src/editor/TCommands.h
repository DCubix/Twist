#ifndef T_COMMANDS_H
#define T_COMMANDS_H

#include "twen/Node.h"
#include "TUndoRedo.h"
#include "TNodeGraph.h"
#include "twen/NodeGraph.h"

class TAddNodeCommand : public TCommand {
public:
	TAddNodeCommand(TNode *node, int x, int y, const Str& type, JSON params)
		: m_params(params), m_type(type), m_x(x), m_y(y), m_node(node)
	{}

	void execute() {
		m_node = m_nodeGraph->addNode(m_x, m_y, m_type, m_params, false);
	}

	void revert() {
//		m_nodeGraph->deleteNode(m_node, false);
	}

private:
	JSON m_params;
	Str m_type;
	int m_x, m_y;
	TNode *m_node;
};

class TDeleteNodeCommand : public TCommand {
public:
	TDeleteNodeCommand(TNode *node, int x, int y, const std::string& type, JSON params)
		: m_params(params), m_type(type), m_x(x), m_y(y), m_node(node)
	{}

	void execute() {
//		m_nodeGraph->deleteNode(m_node, false);
	}

	void revert() {
		m_node = m_nodeGraph->addNode(m_x, m_y, m_type, m_params, false);
	}

private:
	JSON m_params;
	std::string m_type;
	int m_x, m_y;
	TNode *m_node;
};

class TLinkCommand : public TCommand {
public:
	TLinkCommand(Connection *conn, Node *from, Node *to, u32 slot)
		: m_connection(conn), m_from(from), m_to(to), m_slot(slot)
	{}

	void execute() {
		m_connection = m_nodeGraph->actualNodeGraph()->connect(m_from, m_to, m_slot);
	}

	void revert() {
		m_nodeGraph->disconnect(m_connection, false);
	}

private:
	Connection *m_connection;
	Node *m_from, *m_to;
	u32 m_slot;
};

class TUnLinkCommand : public TCommand {
public:
	TUnLinkCommand(Connection *conn, Node *from, Node *to, u32 slot)
		: m_connection(conn), m_from(from), m_to(to), m_slot(slot)
	{}

	void execute() {
		m_nodeGraph->disconnect(m_connection, false);
	}

	void revert() {
		m_connection = m_nodeGraph->actualNodeGraph()->connect(m_from, m_to, m_slot);
	}

private:
	Connection *m_connection;
	Node *m_from, *m_to;
	u32 m_slot;
};

class TMoveCommand : public TCommand {
public:
	struct Point { Point(){} Point(int x, int y) : x(x), y(y) {} int x, y; };

	TMoveCommand(const Vec<TNode*>& nodes, const Map<TNode*, Point>& deltas)
		: nodes(nodes), deltas(deltas)
	{}

	void execute();
	void revert();

private:
	Vec<TNode*> nodes;
	Map<TNode*, Point> deltas;
};

//class TParamChangeCommand : public TCommand {
//public:
//	TParamChangeCommand(
//			u64 nodeID, u32 index,
//			float floatValue, u32 optionValue,
//			float oldValue, u32 oldOption
//	)
//		: m_nodeID(nodeID), m_index(index),
//		  m_value(floatValue), m_option(optionValue),
//		  m_oldValue(oldValue), m_oldOption(oldOption)
//	{}

//	void execute() {
//		Node* node = m_nodeGraph->actualNodeGraph()->get<Node>(m_nodeID);
//		NodeParam& param = node->params()[m_index];
//		param.value = m_value;
//		param.option = m_option;
//	}

//	void revert() {
//		Node* node = m_nodeGraph->actualNodeGraph()->get<Node>(m_nodeID);
//		NodeParam& param = node->params()[m_index];
//		param.value = m_oldValue;
//		param.option = m_oldOption;
//	}

//private:
//	u64 m_nodeID;
//	u32 m_index;
//	float m_value, m_oldValue;
//	u32 m_option, m_oldOption;
//};

#endif // T_COMMANDS_H
