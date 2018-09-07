#ifndef TWEN_LOG_H
#define TWEN_LOG_H

#include <sstream>
#include <string>
#include <utility>

#include "termcolor/termcolor.hpp"

using Out = std::ostream*;

namespace _intern {
	template<typename T>
	inline std::string Stringfy(const T& value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	template<typename T, typename ... Args >
	inline std::string Stringfy(const T& value, const Args&... args) {
		return Stringfy(value) + Stringfy(args...);
	}
}

class Log {
public:
	enum LogLevel {
		Info = 0,
		Warning,
		Error,
		Assert
	};

	static void redirect(Out out, bool colorize=true);

	static void log(
		LogLevel level,
		const char* file,
		const char* function,
		int line,
		const std::string& msg
	);

private:
	static Out _out;
	static bool _colorize;
};

#ifdef __PRETTY_FUNCTION__
#define FN __PRETTY_FUNCTION__
#else
#define FN __FUNCTION__
#endif

#ifdef WINDOWS
#define SEP '\\'
#else
#define SEP '/'
#endif

#define LogPrint(lvl, ...) Log::log(lvl, __FILE__, FN, __LINE__, _intern::Stringfy(__VA_ARGS__))
#define Log(...) LogPrint(Log::Info, __VA_ARGS__)
#define LogW(...) LogPrint(Log::Warning, __VA_ARGS__)
#define LogE(...) LogPrint(Log::Error, __VA_ARGS__)
#define LogAssert(cond, ...) if (!(cond)) { \
	LogPrint(Log::Assert, __VA_ARGS__); \
	abort(); \
}

#endif // TWEN_LOG_H