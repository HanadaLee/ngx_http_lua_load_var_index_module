# vim:set ft= ts=4 sw=4 et:

use Cwd qw(abs_path);
use FindBin;

BEGIN {
    $ENV{TEST_NGINX_MODULE_LUALIB} =
        abs_path("$FindBin::Bin/../lualib");
    $ENV{TEST_NGINX_RESTY_LUALIB} ||= "/usr/local/openresty/lualib";
    $ENV{TEST_NGINX_INIT_BY_LUA} =
        "package.path = '$ENV{TEST_NGINX_RESTY_LUALIB}/?.lua;' "
        . ".. (package.path or ''); require 'resty.core'";
}

use Test::Nginx::Socket::Lua;

repeat_each(1);
plan tests => repeat_each() * blocks() * 3;

no_long_string();
run_tests();

__DATA__

=== TEST 1: indexed variables are available through both APIs
--- http_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
    lua_load_var_index $http_x_test;

    init_by_lua_block {
        require("resty.var").patch_metatable()
    }
--- config
    set $indexed original;
    lua_load_var_index $indexed;

    location = /t {
        content_by_lua_block {
            local var = require "resty.var"

            ngx.say(ngx.var.http_x_test)
            ngx.say(var.get("http_x_test"))
            ngx.say(var.get("indexed"))

            var.set("indexed", "ffi")
            ngx.say(ngx.var.indexed)

            ngx.var.indexed = "metatable"
            ngx.say(var.get("indexed"))

            var.set("indexed", nil)
            ngx.say(var.get("indexed") == nil)
        }
    }
--- request
GET /t
--- more_headers
X-Test: header
--- response_body
header
header
original
ffi
metatable
true
--- no_error_log
[error]



=== TEST 2: request mutations invalidate indexed header values
--- http_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
    lua_load_var_index $http_x_test;

    init_by_lua_block {
        require("resty.var").patch_metatable()
    }
--- config
    location = /t {
        content_by_lua_block {
            ngx.say(ngx.var.http_x_test)
            ngx.req.set_header("X-Test", "changed")
            ngx.say(ngx.var.http_x_test)
        }
    }
--- request
GET /t
--- more_headers
X-Test: original
--- response_body
original
changed
--- no_error_log
[error]



=== TEST 3: phase, name, and mutability errors are reported
--- http_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
    lua_load_var_index $remote_addr;

    init_by_lua_block {
        require("resty.var").patch_metatable()
    }
--- config
    location = /t {
        content_by_lua_block {
            local var = require "resty.var"
            local ok_phase, phase_err = pcall(var.load_indexes)
            local ok_name, name_err = pcall(var.get, "not_indexed")
            local ok_set, set_err = pcall(var.set, "remote_addr", "127.0.0.2")

            ngx.say(ok_phase)
            ngx.say(phase_err:find("init phase", 1, true) ~= nil)
            ngx.say(ok_name)
            ngx.say(name_err:find("not indexed", 1, true) ~= nil)
            ngx.say(ok_set)
            ngx.say(set_err:find("not changeable", 1, true) ~= nil)
        }
    }
--- request
GET /t
--- response_body
false
true
false
true
false
true
--- no_error_log
[error]
