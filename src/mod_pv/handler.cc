#include "handler.hh"
#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <cstring>
#include <pv/Exception.hh>
#include <pv/Errors.hh>
#include <pv/intf.hh>
#include <new>
#include <sstream>
#include "url_codec.hh"
#include <chrono>

extern module AP_MODULE_DECLARE_DATA pv_module;

using clock_type = std::chrono::system_clock;
using time_point = clock_type::time_point;
using std::chrono::seconds;

namespace pv {

static int scan(const Config *cnf, request_rec& r) noexcept;
static int scan(const Config& cnf, request_rec& r);
static int update(const Config *cnf, request_rec& r) noexcept;
static int update(const Config& cnf, request_rec& r);
static int handle_error(const Exception&, request_rec& r) noexcept;

int
handler(const Config *cnf, request_rec *r) noexcept {
	if (nullptr == r || nullptr == r->handler)
		return DECLINED;
	if (0 == strcmp(r->handler, "pv_scan"))
		return scan(cnf, *r);	
	if (0 == strcmp(r->handler, "pv_update"))
		return update(cnf, *r);
	return DECLINED;
}

static int
scan(const Config *cnf, request_rec& r) noexcept {
	if (M_GET != r.method_number)
		return HTTP_METHOD_NOT_ALLOWED;
	if (r.header_only)
		return OK;
	try {
		if (nullptr == cnf)
			throw Exception(__FILE__, __LINE__, error::OOM);
		return scan(*cnf, r);
	} catch (const Exception& err) {
		return handle_error(err, r);
	} catch (const std::bad_alloc& err) {
		return handle_error(Exception(__FILE__, __LINE__, error::OOM,
					      "%s", err.what()), r);
	} catch (const std::exception& err) {
		return handle_error(Exception(__FILE__, __LINE__,
					      error::INTERNAL, "%s",
					      err.what()), r);
	} catch (...) {
		return handle_error(Exception(__FILE__, __LINE__,
					      error::INTERNAL), r);
	}
}

static int
scan(const Config& cnf, request_rec& r) {
	if (nullptr == r.args)
		return HTTP_BAD_REQUEST;
	App app(cnf);
	std::string args = urldec::dec(r.args, r.args + strlen(r.args));
	Result rlt = app.check(args);

	ap_set_content_type(&r, "application/json; charset=utf-8");

	std::ostringstream ss;
	ss << "{\"claim\":" << std::get<0>(rlt) << ",\"signed_by\":";
	if (Status::UnknownKey != std::get<2>(rlt))
		ss << std::get<1>(rlt);
	else
		ss << "null";
	ss << ",\"validation\":";
	switch (std::get<2>(rlt)) {
	case Status::Ok:
		ss << "\"ok\"";
		break;
	case Status::InvalidSignature:
		ss << "\"invalid signature\"";
		break;
	case Status::ExpiredClaim:
		ss << "\"expired claim\"";
		break;
	case Status::UnknownKey:
		ss << "\"unknown key\"";
		break;
	default:
		ss << "\"internal error\"";
		break;
	}
	ss << '}';

	time_point now = clock_type::now();
	time_point exp = std::get<0>(rlt).expires();
	time_point ki = std::get<1>(rlt).cert_to();
	if (ki < exp)
		exp = ki;
	if (time_point::min() != exp && exp > now) {
		seconds age = std::chrono::duration_cast<seconds>(exp - now);
		std::ostringstream ss;
		ss << age.count();
		apr_table_set(r.subprocess_env, "max-age", ss.str().c_str());
	}

	const std::string s = ss.str();
	ap_rwrite(s.data(), s.length(), &r);

	return OK;
}

static int
update(const Config *cnf, request_rec& r) noexcept {
	if (M_POST != r.method_number)
		return HTTP_METHOD_NOT_ALLOWED;
	try {
		if (nullptr == cnf)
			throw Exception(__FILE__, __LINE__, error::OOM);
		return update(*cnf, r);
	} catch (const Exception& err) {
		return handle_error(err, r);
	} catch (const std::bad_alloc& err) {
		return handle_error(Exception(__FILE__, __LINE__, error::OOM,
					      "%s", err.what()), r);
	} catch (const std::exception& err) {
		return handle_error(Exception(__FILE__, __LINE__,
					      error::INTERNAL, "%s",
					      err.what()), r);
	} catch (...) {
		return handle_error(Exception(__FILE__, __LINE__,
					      error::INTERNAL), r);
	}
}

static int
update(const Config& cnf, request_rec& r) {
	App app(cnf);
	return app.update() ? OK : HTTP_CONFLICT;
}

static int
handle_error(const Exception& e, request_rec& r) noexcept {
	const Exception *copy = new(std::nothrow_t{}) Exception(e);
	if (copy)
		ap_set_module_config(r.request_config, &pv_module,
				     const_cast<Exception *>(copy));
	else
		ap_set_module_config(r.request_config, &pv_module,
				     reinterpret_cast<void *>(1));
	
	switch (e.code()) {
	case error::INVALID_ARGS:
		return HTTP_BAD_REQUEST;
	case error::OOM:
	case error::INTERNAL:
	case error::ZLIB:
	case error::CRYPTO:
	case error::CURL:
	default:
		return HTTP_INTERNAL_SERVER_ERROR;
	}
}

}
