#pragma once

#include <pv/Exception.hh>
#include <cstdarg>
#include "visibility.h"

namespace pv {
	
class DSO_LOCAL Logger;
class Logger {
public:
	virtual ~Logger() = default;

	virtual void error(const Exception&) const noexcept = 0;
	virtual void error(const char *, int, unsigned int,
			   const char * = "") const noexcept;
	virtual void debug(const char *, int, const char *, ...) const noexcept
		__attribute__((format(printf, 4, 5)));
	virtual void debug(const char *, int, const char *,
			   va_list) const noexcept = 0;
};

inline void
Logger::error(const char *file, int line, unsigned int code,
	      const char *args) const noexcept {
	error(Exception(file, line, code, "%s", args));
}

inline void
Logger::debug(const char *file, int line, const char *format,
	      ...) const noexcept {
	va_list va;
	va_start(va, format);
	debug(file, line, format, va);
	va_end(va);
}

}
