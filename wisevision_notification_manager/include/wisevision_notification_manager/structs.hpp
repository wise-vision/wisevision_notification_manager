/*
 * Copyright (C) 2025 wisevision
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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