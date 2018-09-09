#ifndef T_BUTTON_NODE_H
#define T_BUTTON_NODE_H

#include "../twen/Node.h"

class ButtonNode : public Node {
	TWEN_NODE(ButtonNode, "Button")
public:
	ButtonNode() : Node() {
		addOutput("Out");
	}

	void solve() {
		outs(0).set(0.0f);
		if (enabled)
			outs(0).set(1.0f);
	}

	bool enabled;
};

#endif // T_BUTTON_NODE_H
