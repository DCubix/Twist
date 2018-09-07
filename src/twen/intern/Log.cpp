#include "Log.h"
#include <cstdarg>

Out Log::_out = nullptr;
bool Log::_colorize = true;

void Log::redirect(Out out, bool colorize) {
	_out = out;
	_colorize = colorize;
}

void Log::log(
	LogLevel level,
	const char* file,
	const char* function,
	int line,
	const std::string& msg
) {
	if (_out == nullptr)
		redirect(&std::cout);

#define OUT (*_out)

#define BOLD(o, msg, col) if (_colorize) { \
	o << col << termcolor::bold << msg << termcolor::reset; \
} else { \
	o << msg; \
}

#define NORM(o, msg, col) if (_colorize) { \
	o << col << msg << termcolor::reset; \
} else { \
	o << msg; \
}

	time_t now = time(0);
	struct tm tstruct;
	char timeBuf[80] = {0};
	tstruct = *localtime(&now);
	strftime(timeBuf, sizeof(timeBuf), "%m/%d/%Y %X", &tstruct);

	if (_colorize) {
		OUT << "[";
		NORM(OUT, timeBuf, termcolor::green);
		OUT << "] ";
	} else {
		OUT << "[" << timeBuf << "] ";
	}

	switch (level) {
		case Info: BOLD(OUT, "[INFO]", termcolor::cyan); break;
		case Warning: BOLD(OUT, "[WARN]", termcolor::yellow); break;
		case Error: BOLD(OUT, "[ERROR]", termcolor::red); break;
		case Assert: BOLD(OUT, "[FAIL]", termcolor::magenta << termcolor::blink); break;
	}

	std::string fileS(file);
	fileS = fileS.substr(fileS.find_last_of(SEP)+1);

	OUT << " [" << fileS << "(" << function << ")@" << line << "] " << msg << std::endl;

}