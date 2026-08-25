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

use Test::Nginx::Socket::Lua::Stream;

repeat_each(1);
plan tests => repeat_each() * blocks() * 3;

no_long_string();
run_tests();

__DATA__

=== TEST 1: stream indexed get and set
--- stream_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
    lua_load_var_index $remote_addr;

    init_by_lua_block {
        require("resty.var").patch_metatable()
    }
--- stream_server_config
    set $indexed original;
    lua_load_var_index $indexed;

    content_by_lua_block {
        local var = require "resty.var"

        ngx.say(var.get("remote_addr"))
        ngx.say(ngx.var.remote_addr)
        ngx.say(var.get("indexed"))
        var.set("indexed", "ffi")
        ngx.say(ngx.var.indexed)
        ngx.var.indexed = "metatable"
        ngx.say(var.get("indexed"))
        var.set("indexed", nil)
        ngx.say(var.get("indexed") == nil)
    }
--- stream_response
127.0.0.1
127.0.0.1
original
ffi
metatable
true
--- no_error_log
[error]



=== TEST 2: stream phase, name, and mutability errors
--- stream_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
    lua_load_var_index $remote_addr;

    init_by_lua_block {
        require("resty.var").patch_metatable()
    }
--- stream_server_config
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
--- stream_response
false
true
false
true
false
true
--- no_error_log
[error]
