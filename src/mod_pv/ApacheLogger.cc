#include <cstdarg>
#include <httpd.h>
#include <http_log.h>
#include <ap_release.h>
#include <pv/Exception.hh>
#include <cstdio>
#include <cstring>
#include <new>

#ifdef PV_CHECK
#include <iostream>
#endif

#include "ApacheLogger.hh"

#ifndef PV_CHECK
#if (AP_SERVER_MAJORVERSION_NUMBER == 2 && AP_SERVER_MINORVERSION_NUMBER > 2) || AP_SERVER_MAJORVERSION_NUMBER > 2
APLOG_USE_MODULE(pv);
#endif
#endif

namespace pv {

ApacheLogger::ApacheLogger(server_rec *s) noexcept
	: srv(s)
{
}

void
ApacheLogger::error(const Exception& err) const noexcept {
#ifndef PV_CHECK
#if (AP_SERVER_MAJORVERSION_NUMBER == 2 && AP_SERVER_MINORVERSION_NUMBER > 2) \
	|| AP_SERVER_MAJORVERSION_NUMBER > 2

	ap_log_error(err.file(), err.line(), APLOG_MODULE_INDEX, err.level(),
		     0, srv, "%s", err.what());
#else
	ap_log_error(err.file(), err.line(), err.level(), 0, srv, "%s",
		     err.what());
#endif
#else
	try {
		std::cerr << err.what() << std::endl;
	} catch (...) {
	}
#endif
}

void
ApacheLogger::debug(const char *file, int line, const char *fmt,
		    va_list va) const noexcept {
	static const std::size_t BUF_SIZE = 256;

	char storage[BUF_SIZE];

	va_list cpy;
	va_copy(cpy, va);
	int n = vsnprintf(storage, sizeof(storage), fmt, va);
	if (n < static_cast<int>(sizeof(storage))) {
		va_end(cpy);
		// log what's in storage
		log_debug(file, line, storage);
		return;
	}
	char *buf = reinterpret_cast<char *>(
		::operator new[](n + 1, std::nothrow_t()));
	if (nullptr == buf) {
		va_end(cpy);
		// log what's in storage
		log_debug(file, line, storage);
		return;
	}
	vsnprintf(buf, n + 1, fmt, cpy);
	va_end(cpy);
	// log what's in buf
	log_debug(file, line, buf);
	delete [] buf;
}

void
ApacheLogger::log_debug(const char *file, int line,
			const char *data) const noexcept {
#ifndef PV_CHECK
#if (AP_SERVER_MAJORVERSION_NUMBER == 2 && AP_SERVER_MINORVERSION_NUMBER > 2) \
	|| AP_SERVER_MAJORVERSION_NUMBER > 2
	ap_log_error(basename(file), line, APLOG_MODULE_INDEX, APLOG_DEBUG,
		     0, srv, "%s", data);
#else
	ap_log_error(basename(file), line, APLOG_DEBUG, 0, srv, "%s", data);
#endif
#else
	try {
		std::cerr << basename(file) << '(' << line << "): " << data
			  << std::endl;
	} catch (...) {
	}
#endif
}

const char *
ApacheLogger::basename(const char *file) noexcept {
	const char *p = strrchr(file, '/');
	if (nullptr == p)
		return file;
	return p + 1;
}

}
