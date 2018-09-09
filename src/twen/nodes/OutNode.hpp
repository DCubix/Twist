#ifndef TWEN_OUT_NODE_H
#define TWEN_OUT_NODE_H

#include "../Node.h"

class OutNode : public Node {
	TWEN_NODE(OutNode, "Output")
public:
	OutNode() : Node() {
		addInput("In");
		addInput("Vol.");
		ins(1).set(1.0f);
	}

	void solve() {
		nextLvl = std::max(nextLvl, in(0) * in(1));
		in(0) = in(0) * in(1);

		level = Utils::lerp(level, nextLvl, 0.0007f);
		nextLvl -= 1.0f / 22050;
		nextLvl = std::min(std::max(nextLvl, 0.0f), 1.0f);
	}

	float level;

private:
	float nextLvl;
};

#endif // TWEN_OUT_NODE_H
