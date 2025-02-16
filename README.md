Name
====
ngx_http_lua_load_var_index_module


Table of Contents
=================

* [Name](#name)
* [Description](#description)
* [Install](#install)
* [Directives](#directives)
    * [lua\_load\_var\_index](#lua_load_var_index)
* [Methods](#methods)
    * [resty.var.patch\_metatable](#restyvarpatch_metatable)
* [License](#license)


Description
===========
Separate from [lua-kong-nginx-module](https://github.com/Kong/lua-kong-nginx-module). it can enable indexed variable access for `ngx.var`. Indexed variable access is a faster way of accessing Nginx variables for OpenResty.


Install
=======
This module can be installed just like any ordinary Nginx C module, using the
`--add-module` configuration option:

```shell
./configure --add-module=/path/to/ngx_http_lua_load_var_index_module \
            ...

```

Directives
=======

lua\_load\_var\_index
-------------------------------------------
**syntax:** *lua_load_var_index $variable | default;*

**context:** *http, server, location*

Ensure *variable* is indexed. Note that variables defined by `set` directive
are always indexed by default and does not need to be defined here again.

Common variables defined by other modules that are already indexed:

- `$proxy_host`
- `$proxy_internal_body_length`
- `$proxy_internal_chunked`
- `$remote_addr`
- `$remote_user`
- `$request`
- `$http_referer`
- `$http_user_agent`
- `$host`

Tips: Allowing directives to be used at the server and location level is only for configuration management convenience. All configurations will take effect at the http level.

See [resty.var.patch\_metatable](#restyvarpatch_metatable) on how to enable
indexed variable access.

[Back to TOC](#table-of-contents)

Methods
=======

resty.var.patch\_metatable
----------------------------------
**syntax:** *resty.var.patch_metatable()*

**context:** *init_by_lua*

**subsystems:** *http*

Indexed variable access is a faster way of accessing Nginx variables for OpenResty.
This method patches the metatable of `ngx.var` to enable index access to variables
that supports it. It should be called once in the `init` phase which will be effective for
all subsequent `ngx.var` uses.

For variables that does not have indexed access, the slower hash based lookup will
be used instead (this is the OpenResty default behavior).

To ensure a variable can be accessed using index, you can use the [lua_load_var_index](#lua_load_var_index)
directive.

[Back to TOC](#table-of-contents)


License
=======

```
Copyright (C) Hanada
Copyright 2020-2023 Kong Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

[Back to TOC](#table-of-contents)