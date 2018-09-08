#ifndef TWEN_NODE_REGISTRY_H
#define TWEN_NODE_REGISTRY_H

#include "Node.h"
#include "intern/Utils.h"
#include "intern/Log.h"

template <class Nt, typename = void>
struct IsNode { static const bool value = false; };

template <class Nt>
struct IsNode<Nt, decltype(Nt::typeID())> { static const bool value = true; };

using NodeCtor = Node*(JSON);
#define TWEN_NODE_FAC [](JSON json) -> Node*

struct NodeFactory {
	NodeCtor* ctor;
	Str category, title, type;
};

class NodeBuilder {
public:
	template <typename Nt>
	static void registerType(const Str& category, NodeCtor* factory) {
		static_assert(
			std::is_base_of<Node, Nt>::value,
			"The node must be derived from 'Node'."
		);
		if (factories.find(Nt::type()) != factories.end()) {
			LogE("This node is already defined.");
			return;
		}
		factories[Nt::type()].ctor = factory;
		factories[Nt::type()].category = category;
		factories[Nt::type()].title = Nt::prettyName();
		factories[Nt::type()].type = Nt::type();
	}

	static Node* createNode(const Str& typeName, JSON params) {
		if (factories.find(typeName) == factories.end()) {
			LogE("Invalid node type.");
			return nullptr;
		}
		Node* node = factories[typeName].ctor(params);
		node->m_title = factories[typeName].title;
		return node;
	}

	static Map<Str, NodeFactory> factories;
};

#endif // TWEN_NODE_REGISTRY_H