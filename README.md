# ioq3
![travis](https://travis-ci.org/ranguli/ioq3.svg?branch=master)

## What is in this fork?
- Documentation of the Quake codebase generated by Doxygen
- Collection of silly hacks in the `hacks/` directory.
- Scripts for easier server/client startup

## Programmer's Guide to the Quake III Engine: 

### Notes on Architecture:
- The Quake 3 server is under `game/`, and the client code is under `cgame/`
  and `ui/`
- `botlib/` is for Bot AI.

### Variable naming conventions:
- `typedef float vec_t`: variables that end in `_t` signify a typdef.
- `struct teamgame_s`: structs that end in `_s` signif a struct.

### Function naming conventions:
- Suffix `_r`: functions that end in `_r` signify that they are recursive.

### File naming conventions:
- `Prefix g_`: File is part of `game`, which is server-side code.
- `Prefix bg_`: Means 'both games', things shared by both the server game and client game. 
- `ai_main.c`:
- Prefix `be_ai`: The source file is part of `botlib` and deals
  with AI
- Prefix `be_aas`: Part of `botlib` and relates to the Area Awareness System ( how bots know to navigate a map)
- Prefix `cg_`: stands for "Client Game", meaning client-side game code.

### Botlib:
- Contains the code for Bot AI.
- Uses `LibVar` (`l_libvar.h` and `l_libvar.c`) which contains the bot library
  variables.

### Notes on cvars:
- A Console Variable (cvar) stores something in a way that can be accessed by
  the console. 
- Your code will check the value the next time it needs it.  
- A cvar in the source can be read and used by any part of that source code
  module.
- Cvars are created in the code as a data structure. They require a default
  initialization value and flags that control access.

The locations of a cvar can be one of these three:
- `game/g_main.c`
- `cgame/cg_main.c`
- `ui/ui_main.c`


### Creating client game cvars: 
```
vmCvar_t    cg_drawFPS;
```

In order to allow the cvar to update automatically it must be added into the
cvar table:

```
cvarTable_t cvarTable[] = {
    ... 
    { &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE }
    ...
}
```

- `&cg_drawFPS`:  Pointer to `cg_drawFPS` that stores the cvar value. 
- `"cg_drawFPS"`: The name the cvar is called in the console.
- `"0"`: The initialization value, which can be anything.
- `CVAR_ARCHIVE`: Flag controlling the behavior of the variable.

There are more flags in `q_shared.h`.

In order to reference a cvar from another source file within the module:

```
extern  vmCvar_t        cg_drawFPS;
```

### Creating server cvar:
```
cvarTable_t cvarTable[] = {
    ... 
    { &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH, 0, qfalse }
    ...
}
```
