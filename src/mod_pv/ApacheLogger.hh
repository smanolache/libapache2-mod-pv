#pragma once

#include <httpd.h>
#include <pv/Exception.hh>
#include "Logger.hh"
#include <cstdarg>
#include "visibility.h"

namespace pv {
	
class DSO_LOCAL ApacheLogger;
class ApacheLogger : public Logger {
protected:
	server_rec *srv;

public:
	ApacheLogger(server_rec *) noexcept;

	using Logger::error;
	using Logger::debug;

	virtual void error(const Exception&) const noexcept;
	virtual void debug(const char *, int, const char *,
			   va_list) const noexcept;

protected:
	virtual void log_debug(const char *, int, const char *) const noexcept;
	static const char *basename(const char *) noexcept;
};

}
