#include <apr_pools.h>
#include <ap_release.h>
#include <httpd.h>
#include <http_config.h>
#include <apr_hooks.h>
#include <apr_errno.h>
#include <apr_thread_proc.h>
#include <pv/Exception.hh>
#include <pv/Errors.hh>
#include <stdexcept>
#include <http_protocol.h>
#include <curl/curl.h>
#include "ApacheLogger.hh"
#include "config.hh"
#include "handler.hh"
#include <pv/Config.hh>
#include <openssl/err.h>
#include <openssl/evp.h>

#if (AP_SERVER_MAJORVERSION_NUMBER == 2 && AP_SERVER_MINORVERSION_NUMBER > 2) || AP_SERVER_MAJORVERSION_NUMBER > 2
APLOG_USE_MODULE(pv);
#endif

using pv::Exception;
using pv::error;

static void register_hooks(apr_pool_t *) noexcept;
static void child_init(apr_pool_t *p, server_rec *s) noexcept;
static apr_status_t cleanup_ssl(void *) noexcept;
static int handler(request_rec *r) noexcept;
static int log(request_rec *r) noexcept;
static pv::Config *create_dir_config(apr_pool_t *, const char *dir) noexcept;
static pv::Config *merge_dir_config(apr_pool_t *, pv::Config *parent,
				    pv::Config *child) noexcept;
static apr_status_t delete_cnf(pv::Config *) noexcept;

module AP_MODULE_DECLARE_DATA pv_module = {
	STANDARD20_MODULE_STUFF,
	reinterpret_cast<void *(*)(apr_pool_t *, char *)>(
		&create_dir_config),
	reinterpret_cast<void *(*)(apr_pool_t *, void *, void *)>(
		&merge_dir_config),
	nullptr,
	nullptr,
	pv::cmds,
	register_hooks
};

static pv::Config *
create_dir_config(apr_pool_t *p, const char *dir) noexcept {
	pv::Config *cnf = new (std::nothrow_t{}) pv::Config();
	if (cnf)
		apr_pool_cleanup_register(
			p, cnf, reinterpret_cast<apr_status_t (*)(void *)>(
				&delete_cnf), apr_pool_cleanup_null);
	return cnf;
}

static pv::Config *
merge_dir_config(apr_pool_t *p, pv::Config *parent,
		 pv::Config *child) noexcept {
	if (!parent)
		return child;
	if (!child)
		return parent;
	pv::Config *cnf = new (std::nothrow_t{}) pv::Config(*child);
	if (!cnf)
		return nullptr;
	try {
		cnf->merge(*parent);
		apr_pool_cleanup_register(
			p, cnf, reinterpret_cast<apr_status_t (*)(void *)>(
				&delete_cnf), apr_pool_cleanup_null);
		return cnf;
	} catch (...) {
		delete cnf;
		return nullptr;
	}
}

static apr_status_t
delete_cnf(pv::Config *cnf) noexcept {
	delete cnf;
	return APR_SUCCESS;
}

static void
register_hooks(apr_pool_t *) noexcept {
	ap_hook_child_init(&child_init, nullptr, nullptr, APR_HOOK_MIDDLE);
	ap_hook_handler(&handler, nullptr, nullptr, APR_HOOK_MIDDLE);
	static const char * const succ[] = {"mod_log_config.c", nullptr};
	ap_hook_log_transaction(&log, nullptr, succ, APR_HOOK_MIDDLE);
}

static void
child_init(apr_pool_t *p, server_rec *s) noexcept {
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	if (0 != curl_global_init(CURL_GLOBAL_ALL)) {
		const pv::ApacheLogger logger(s);
		logger.error(__FILE__, __LINE__, error::CURL,
			     ": curl_global_init.");
		return;
	}
	apr_pool_cleanup_register(
		p, nullptr, reinterpret_cast<apr_status_t (*)(void *)>(
			&cleanup_ssl), apr_pool_cleanup_null);
	if (APR_SUCCESS != apr_setup_signal_thread()) {
		const pv::ApacheLogger logger(s);
		logger.error(__FILE__, __LINE__, error::INTERNAL,
			     ": Setting thread signal mask in "
			     "apr_setup_signal_thread.");
		return;
	}
}

static apr_status_t
cleanup_ssl(void *) noexcept {
	curl_global_cleanup();
	return APR_SUCCESS;
}

static int
handler(request_rec *r) noexcept {
	const pv::Config *cnf = reinterpret_cast<const pv::Config *>(
		ap_get_module_config(r->per_dir_config, &pv_module));
	return pv::handler(cnf, r);
}

static int
log(request_rec *r) noexcept {
	if (nullptr == r || nullptr == r->handler ||
	    (0 != strcmp(r->handler, "pv_scan") &&
	     0 != strcmp(r->handler, "pv_update")))
		return DECLINED;
	const void *cnf = ap_get_module_config(r->request_config, &pv_module);
	if (!cnf)
		return DECLINED;
	const pv::ApacheLogger logger(r->server);
	if (reinterpret_cast<const void *>(1) != cnf)
		logger.error(*reinterpret_cast<const Exception *>(cnf));
	else
		logger.error(Exception(__FILE__, __LINE__, error::OOM));
	return OK;
}
