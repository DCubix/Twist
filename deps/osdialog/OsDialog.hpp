#ifndef OS_DIALOG_HPP
#define OS_DIALOG_HPP

#include <string>
#include <optional>
#include "osdialog.h"

namespace osd {
	template <typename T> using Opt = std::optional<T>;
	using Color = osdialog_color;

	enum MessageLevel {
		Info = OSDIALOG_INFO,
		Warning = OSDIALOG_WARNING,
		Error = OSDIALOG_ERROR
	};

	enum MessageButtons {
		Ok = OSDIALOG_OK,
		OkCancel = OSDIALOG_OK_CANCEL,
		YesNo = OSDIALOG_YES_NO
	};

	enum DialogAction {
		OpenFile = OSDIALOG_OPEN,
		OpenDirectory = OSDIALOG_OPEN_DIR,
		SaveFile = OSDIALOG_SAVE
	};

	class Filters {
		friend class Dialog;
	public:
		Filters() : m_filters(nullptr) {}
		Filters(const std::string& filters) {
			m_filters = osdialog_filters_parse(filters.c_str());
		}

		~Filters() {
			if (m_filters != nullptr) {
				osdialog_filters_free(m_filters);
				m_filters = nullptr;
			}
		}
	protected:
		osdialog_filters* m_filters;
	};

	class Dialog {
	public:
		static Opt<std::string> file(
			DialogAction action,
			const std::string& defaultPath,
			const Filters& filters
		) {

			char* fileName = osdialog_file(
				(osdialog_file_action)action,
				defaultPath.c_str(),
				"",
				filters.m_filters
			);
			if (fileName == nullptr) {
				return {};
			}

			std::string ret(fileName);
			free(fileName);
			return ret;
		}

		static bool message(
			MessageLevel level,
			MessageButtons buttons,
			const std::string& msg
		) {
			return osdialog_message(
				(osdialog_message_level)level,
				(osdialog_message_buttons)buttons,
				msg.c_str()
			) == 1 ? true : false;
		}

		static Opt<Color> color(bool alpha=false) {
			Color ret = { 0, 0, 0, 0 };
			if (osdialog_color_picker(&ret, alpha ? 1 : 0) == 1) {
				return ret;
			}
			return {};
		}

		static void web(const std::string& url) {
			osdialog_web(url.c_str());
		}
	};
};

#endif // OS_DIALOG_HPP