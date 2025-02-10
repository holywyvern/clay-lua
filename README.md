# clay-lua
Lua port of Clay (C-layout) targeting no specific render

### Building

Windows binaries are available for x64 here.

To build the library yourself, grab the code with:

Next, you need to compile the code to a native Lua module under `love_clay` using CMake.

The process is really close to https://github.com/keharriso/love-nuklear, so you may follow a guide of how to build from there.

I build it myself with MSVC so I run the following commands:

```
set "LUA_DIR=path to luajit"
cmake -Bbuild -H. -A x64 -DLUA_INCLUDE_DIR=%LUA_DIR%\src
cmake --build build --config Release
```
