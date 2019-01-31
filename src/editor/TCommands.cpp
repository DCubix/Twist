#include "TCommands.h"

#include "TNodeEditor.h"
#include "TNodeGraph.h"

void TMoveCommand::execute() {
	for (TNode* node : nodes) {
		node->bounds.x += deltas[node].x;
		node->bounds.y += deltas[node].y;
		if (m_nodeGraph->editor()->snapToGrid()) {
			node->gridPos.x = (int(node->bounds.x) / 8) * 8;
			node->gridPos.y = (int(node->bounds.y) / 8) * 8;
		}
	}
}

void TMoveCommand::revert() {
	for (TNode* node : nodes) {
		node->bounds.x -= deltas[node].x;
		node->bounds.y -= deltas[node].y;
		if (m_nodeGraph->editor()->snapToGrid()) {
			node->gridPos.x = (int(node->bounds.x) / 8) * 8;
			node->gridPos.y = (int(node->bounds.y) / 8) * 8;
		}
	}
}
