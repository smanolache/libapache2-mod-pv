#include "config.hh"
#include <http_config.h>
#include <pv/Config.hh>
#include <pv/Exception.hh>
#include <pv/Errors.hh>
#include <apr_strings.h>

namespace pv {

static const char *url_conf(cmd_parms *, Config *, const char *) noexcept;
static const char *key_conf(cmd_parms *, Config *, const char *) noexcept;
static const char *cert_conf(cmd_parms *, Config *, const char *) noexcept;

const command_rec cmds[] {
	AP_INIT_TAKE1(
		"pv_url", reinterpret_cast<cmd_func>(&url_conf), nullptr,
		ACCESS_CONF,
		"Specifies the URL of the certificate repository; "
		"Syntax: pv_url string; "
		"Default: https://dgc-verification-prod.incert.lu:9443/api/get-certificates"
	),
	AP_INIT_TAKE1(
		"pv_key", reinterpret_cast<cmd_func>(&key_conf), nullptr,
		ACCESS_CONF,
		"Specifies the client key for authentication at the "
		"certificate repository; "
		"Syntax: pv_key string; "
		"Default: none"
	),
	AP_INIT_TAKE1(
		"pv_cert", reinterpret_cast<cmd_func>(&cert_conf), nullptr,
		ACCESS_CONF,
		"Specifies the client certificate for authentication at the "
		"certificate repository; "
		"Syntax: pv_cert string; "
		"Default: none"
	),
	{nullptr},
};

static const char *
url_conf(cmd_parms *p, Config *cnf, const char *arg) noexcept {
	try {
		if (!cnf)
			throw Exception(__FILE__, __LINE__, error::OOM);
		if (!arg || '\0' == *arg)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Empty argument in pv_url");
		cnf->set_url(arg);
		return nullptr;
	} catch (const std::exception& e) {
		return apr_pstrdup(p->pool, e.what());
	}
}

static const char *
key_conf(cmd_parms *p, Config *cnf, const char *arg) noexcept {
	try {
		if (!cnf)
			throw Exception(__FILE__, __LINE__, error::OOM);
		if (!arg || '\0' == *arg)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Empty argument in pv_key");
		cnf->set_key(arg);
		return nullptr;
	} catch (const std::exception& e) {
		return apr_pstrdup(p->pool, e.what());
	}
}

static const char *
cert_conf(cmd_parms *p, Config *cnf, const char *arg) noexcept {
	try {
		if (!cnf)
			throw Exception(__FILE__, __LINE__, error::OOM);
		if (!arg || '\0' == *arg)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Empty argument in pv_cert");
		cnf->set_cert(arg);
		return nullptr;
	} catch (const std::exception& e) {
		return apr_pstrdup(p->pool, e.what());
	}
}

}
