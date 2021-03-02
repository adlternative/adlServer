#include "./httpParser.h"
namespace adl {
namespace http {
/* post , get, head,delete, put, connect, options, patch,
 * propfind, propatch,
 * mkcol, copy, move, lock, unlock, trace, head */
const char *httpRequestMethod::methodString[DONT_UNDERSTAND] = {
    "GET", "POST" /* , "HEAD", "PUT", "PATCH", "DELETE", "TRACE", "CONNECT", */
};

void httpRequestLine::debug() {
  LOG(DEBUG) << "version_: " << version_ << " url_: " << url_
             << " method: " << httpRequestMethod::methodString[method_.method_]
             << adl::endl;
}

} // namespace http
} // namespace adl