#ifndef T_UNDO_REDO_H
#define T_UNDO_REDO_H

#include <vector>
#include <memory>
#include <type_traits>

class TNodeGraph;
class TCommand {
	friend class TUndoRedo;
public:
	virtual void execute() = 0;
	virtual void revert() = 0;

protected:
	TNodeGraph* m_nodeGraph;
};

using TCommandPtr = std::unique_ptr<TCommand>;

class TUndoRedo {
public:
	void undo(int levels = 1);
	void redo(int levels = 1);

	template <class T, typename... Args>
	void performedAction(TNodeGraph* graph, Args&&... args) {
		static_assert(
			std::is_base_of<TCommand, T>::value,
			"Invalid command."
		);
		T* cmd = new T(args...);
		cmd->m_nodeGraph = graph;
		m_undo.push_back(std::unique_ptr<T>(cmd));
		m_redo.clear();
	}

	bool canUndo() const { return !m_undo.empty(); }
	bool canRedo() const { return !m_redo.empty(); }

private:
	std::vector<TCommandPtr> m_undo, m_redo;
};

#endif // T_UNDO_REDO_H