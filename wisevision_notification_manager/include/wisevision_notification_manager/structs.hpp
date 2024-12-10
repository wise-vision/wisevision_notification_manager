#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <curl/curl.h>

struct CurlSlistDeleter {
  void operator()(curl_slist *slist) const {
    if (slist) {
      curl_slist_free_all(slist);
    }
  }
};

#endif // STRUCTS_HPP