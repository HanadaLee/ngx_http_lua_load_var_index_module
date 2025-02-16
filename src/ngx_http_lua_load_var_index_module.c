
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


#include "ngx_http_lua_load_var_index_module.h"


static char *ngx_http_lua_load_var_index(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static ngx_command_t  ngx_http_lua_load_var_index_commands[] = {

    { ngx_string("lua_load_var_index"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_lua_load_var_index,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_lua_load_var_index_module_ctx = {
    NULL,                                          /* preconfiguration */
    NULL,                                          /* postconfiguration */

    NULL,                                          /* create main conf */
    NULL,                                          /* init main conf */

    NULL,                                          /* create srv conf */
    NULL,                                          /* merge srv conf */

    NULL,                                          /* create loc conf */
    NULL                                           /* merge loc conf */
};


ngx_module_t  ngx_http_lua_load_var_index_module = {
    NGX_MODULE_V1,
    &ngx_http_lua_load_var_index_module_ctx,       /* module context */
    ngx_http_lua_load_var_index_commands,          /* module directives */
    NGX_HTTP_MODULE,                               /* module type */
    NULL,                                          /* init master */
    NULL,                                          /* init module */
    NULL,                                          /* init process */
    NULL,                                          /* init thread */
    NULL,                                          /* exit thread */
    NULL,                                          /* exit process */
    NULL,                                          /* exit master */
    NGX_MODULE_V1_PADDING
};


static char *
ngx_http_lua_load_var_index(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                     *value;
    ngx_int_t                      index;

    value = cf->args->elts;

    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                            "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    index = ngx_http_get_variable_index(cf, &value[1]);

    if (index == NGX_ERROR) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "unable to mark variable \"%V\" "
                           "as indexed: no memory",
                           &value[1]);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


ngx_uint_t
ngx_http_lua_ffi_var_load_indexes(ngx_str_t **names)
{
    ngx_uint_t                  i;
    ngx_http_variable_t        *v;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_cycle_get_module_main_conf(ngx_cycle, ngx_http_core_module);

    /* return required size only */
    if (names == NULL) {
        return cmcf->variables.nelts;
    }

    v = cmcf->variables.elts;

    ngx_http_lua_load_var_index_assert(v != NULL);

    for (i = 0; i < cmcf->variables.nelts; i++) {
        ngx_http_lua_load_var_index_assert(v[i].index == i);
        names[i] = &v[i].name;
    }

    return NGX_OK;
}


int
ngx_http_lua_ffi_var_get_by_index(ngx_http_request_t *r, ngx_uint_t index,
    u_char **value, size_t *value_len, char **err)
{
    ngx_http_variable_value_t   *vv;

    if (r == NULL) {
        *err = "no request object found";
        return NGX_ERROR;
    }

    if ((r)->connection->fd == (ngx_socket_t) -1) {
        *err = "API disabled in the current context";
        return NGX_ERROR;
    }

    vv = ngx_http_get_indexed_variable(r, index);
    if (vv == NULL || vv->not_found) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                "variable value is not found by index %d", index);

        return NGX_DECLINED;
    }

    *value = vv->data;
    *value_len = vv->len;

    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "get variable value '%v' by index %d", vv, index);

    return NGX_OK;
}


int
ngx_http_lua_ffi_var_set_by_index(ngx_http_request_t *r, ngx_uint_t index,
    u_char *value, size_t value_len, char **err)
{
    u_char                      *p;
    ngx_http_variable_t         *v;
    ngx_http_variable_value_t   *vv;
    ngx_http_core_main_conf_t   *cmcf;

    if (r == NULL) {
        *err = "no request object found";
        return NGX_ERROR;
    }

    if ((r)->connection->fd == (ngx_socket_t) -1) {
        *err = "API disabled in the current context";
        return NGX_ERROR;
    }

    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);

    ngx_http_lua_load_var_index_assert(index < cmcf->variables.nelts);

    v = ((ngx_http_variable_t *) cmcf->variables.elts) + index;

    /*
     * following is slightly modified from
     * openresty/lua-nginx-module/blob/master/src/ngx_http_lua_variable.c
     */

    if (!(v->flags & NGX_HTTP_VAR_CHANGEABLE)) {
        *err = "variable not changeable";
        return NGX_ERROR;
    }

    if (v->set_handler) {
        if (value != NULL && value_len) {
            vv = ngx_palloc(r->pool, sizeof(ngx_http_variable_value_t)
                            + value_len);
            if (vv == NULL) {
                goto nomem;
            }

            p = (u_char *) vv + sizeof(ngx_http_variable_value_t);
            ngx_memcpy(p, value, value_len);
            value = p;

        } else {
            vv = ngx_palloc(r->pool, sizeof(ngx_http_variable_value_t));
            if (vv == NULL) {
                goto nomem;
            }
        }

        if (value == NULL) {
            vv->valid = 0;
            vv->not_found = 1;
            vv->no_cacheable = 0;
            vv->data = NULL;
            vv->len = 0;

        } else {
            vv->valid = 1;
            vv->not_found = 0;
            vv->no_cacheable = 0;

            vv->data = value;
            vv->len = value_len;
        }

        v->set_handler(r, vv, v->data);

        return NGX_OK;
    }

    ngx_http_lua_load_var_index_assert(v->flags & NGX_HTTP_VAR_INDEXED);

    vv = &r->variables[index];

    if (value == NULL) {
        vv->valid = 0;
        vv->not_found = 1;
        vv->no_cacheable = 0;

        vv->data = NULL;
        vv->len = 0;

    } else {
        p = ngx_palloc(r->pool, value_len);
        if (p == NULL) {
            goto nomem;
        }

        ngx_memcpy(p, value, value_len);
        value = p;

        vv->valid = 1;
        vv->not_found = 0;
        vv->no_cacheable = 0;

        vv->data = value;
        vv->len = value_len;
    }

    return NGX_OK;

nomem:

    *err = "no memory";
    return NGX_ERROR;
}
