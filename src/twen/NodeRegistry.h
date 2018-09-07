#ifndef TWEN_NODE_REGISTRY_H
#define TWEN_NODE_REGISTRY_H

#include "Node.h"
#include "intern/Utils.h"
#include "intern/Log.h"

template <class Nt, typename = void>
struct IsNode { static const bool value = false; };

template <class Nt>
struct IsNode<Nt, decltype(Nt::typeID())> { static const bool value = true; };

using NodeFactory = Node*(JSON);
#define TWEN_NODE_FAC [](JSON json) -> Node*

class NodeBuilder {
public:
	template <typename Nt>
	static void registerType(NodeFactory* factory) {
		static_assert(
			std::is_base_of<Node, Nt>::value,
			"The node must be derived from 'Node'."
		);
		if (_factories.find(Nt::type()) != _factories.end()) {
			LogE("This node is already defined.");
			return;
		}
		_factories[Nt::type()] = factory;
	}

	template <typename Nt>
	static Nt* createNode(const Str& typeName, JSON params) {
		static_assert(
			std::is_base_of<Node, Nt>::value,
			"The node must be derived from 'Node'."
		);
		if (_factories.find(Nt::type()) != _factories.end()) {
			LogE("Invalid node type.");
			return nullptr;
		}
		return _factories[Nt::type()](params);
	}

	static Map<Str, NodeFactory*> factories;
};

#endif // TWEN_NODE_REGISTRY_H