#pragma once

#include <httpd.h>
#include "visibility.h"
#include <pv/Config.hh>

namespace pv {

int handler(const Config *cnf, request_rec *r) noexcept DSO_LOCAL;

}
