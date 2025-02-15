
/**
 * Copyright (C) Hanada
 * Copyright 2019-2022 Kong Inc.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *    http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef _NGX_HTTP_LUA_LOAD_VAR_INDEX_MODULE_H_INCLUDED_
#define _NGX_HTTP_LUA_LOAD_VAR_INDEX_MODULE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#ifdef NGX_LUA_USE_ASSERT
#include <assert.h>
#   define ngx_http_lua_load_var_index_assert(a)  assert(a)
#else
#   define ngx_http_lua_load_var_index_assert(a)
#endif



#endif /* _NGX_HTTP_LUA_LOAD_VAR_INDEX_MODULE_H_INCLUDED_ */