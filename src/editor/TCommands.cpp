#include "TCommands.h"
#include "TNodeEditor.h"

void TMoveCommand::execute() {
	for (int id : ids) {
		TNode* node = m_nodeGraph->node(id);
		node->bounds().x += deltas[id].x;
		node->bounds().y += deltas[id].y;
		if (m_nodeGraph->editor()->snapToGrid()) {
			node->gridPosition().x = (int(node->bounds().x) / 8) * 8;
			node->gridPosition().y = (int(node->bounds().y) / 8) * 8;
		}
	}
}

void TMoveCommand::revert() {
	for (int id : ids) {
		TNode* node = m_nodeGraph->node(id);
		node->bounds().x -= deltas[id].x;
		node->bounds().y -= deltas[id].y;
		if (m_nodeGraph->editor()->snapToGrid()) {
			node->gridPosition().x = (int(node->bounds().x) / 8) * 8;
			node->gridPosition().y = (int(node->bounds().y) / 8) * 8;
		}
	}
}