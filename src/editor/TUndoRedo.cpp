#include "TUndoRedo.h"

void TUndoRedo::undo(int levels) {
	for (int i = 1; i <= levels; i++) {
		if (!m_undo.empty()) {
			m_undo.back()->revert();

			m_redo.push_back(std::move(m_undo.back()));
			m_undo.pop_back();
		}
	}
}

void TUndoRedo::redo(int levels) {
	for (int i = 1; i <= levels; i++) {
		if (!m_redo.empty()) {
			m_redo.back()->execute();

			m_undo.push_back(std::move(m_redo.back()));
			m_redo.pop_back();
		}
	}
}