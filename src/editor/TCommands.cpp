#include "TCommands.h"

#include "TNodeEditor.h"
#include "TNodeGraph.h"

void TMoveCommand::execute() {
	for (u64 id : ids) {
		TNodeUI* node = m_nodeGraph->node(id);
		node->bounds.x += deltas[id].x;
		node->bounds.y += deltas[id].y;
		if (m_nodeGraph->editor()->snapToGrid()) {
			node->gridPos.x = (int(node->bounds.x) / 8) * 8;
			node->gridPos.y = (int(node->bounds.y) / 8) * 8;
		}
	}
}

void TMoveCommand::revert() {
	for (u64 id : ids) {
		TNodeUI* node = m_nodeGraph->node(id);
		node->bounds.x -= deltas[id].x;
		node->bounds.y -= deltas[id].y;
		if (m_nodeGraph->editor()->snapToGrid()) {
			node->gridPos.x = (int(node->bounds.x) / 8) * 8;
			node->gridPos.y = (int(node->bounds.y) / 8) * 8;
		}
	}
}