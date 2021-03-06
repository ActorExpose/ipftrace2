/*
 * SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
 * Copyright (C) 2020 Yutaro Hayakawa
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/bpf.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "ipftrace.h"
#include "bpf_lua.h"
#include "bpf.h"

struct ipft_script {
  lua_State *L;
};

struct ipft_script_opt {
  char *path;
};

/*
 * ipft.offsetof("struct foo", "member")
 */
static int
script_ipft_offsetof(lua_State *L)
{
  int error, n;
  size_t offset;
  const char *type, *member;
  struct ipft_debuginfo **dinfop;

  n = lua_gettop(L);
  if (n != 2) {
    lua_pushliteral(L, "Incorrect number of argument. Expect 2.");
    lua_error(L);
  }

  type = lua_tostring(L, -2);
  member = lua_tostring(L, -1);

  dinfop = (struct ipft_debuginfo **)lua_getextraspace(L);

  error = debuginfo_offsetof(*dinfop, type, member, &offset);
  if (error == -1) {
    lua_pushliteral(L, "Couldn't get offset");
    lua_error(L);
  }

  lua_pushinteger(L, offset);

  return 1;
}

/*
 * ipft.sizeof("type")
 */
static int
script_ipft_sizeof(lua_State *L)
{
  size_t size;
  int error, n;
  const char *type;
  struct ipft_debuginfo **dinfop;

  n = lua_gettop(L);
  if (n != 1) {
    lua_pushliteral(L, "Incorrect number of argument. Expect 1.");
    lua_error(L);
  }

  type = lua_tostring(L, -1);

  dinfop = (struct ipft_debuginfo **)lua_getextraspace(L);

  error = debuginfo_sizeof(*dinfop, type, &size);
  if (error == -1) {
    lua_pushliteral(L, "Couldn't get size");
    lua_error(L);
  }

  lua_pushinteger(L, size);

  return 1;
}

/*
 * ipft.typeof("type", "member")
 */
static int
script_ipft_typeof(lua_State *L)
{
  char *name;
  int error, n;
  const char *type, *member;
  struct ipft_debuginfo **dinfop;

  n = lua_gettop(L);
  if (n != 2) {
    lua_pushliteral(L, "Incorrect number of argument. Expect 2.");
    lua_error(L);
  }

  type = lua_tostring(L, -2);
  member = lua_tostring(L, -1);

  dinfop = (struct ipft_debuginfo **)lua_getextraspace(L);

  error = debuginfo_typeof(*dinfop, type, member, &name);
  if (error == -1) {
    lua_pushliteral(L, "Couldn't get member type");
    lua_error(L);
  }

  lua_pushstring(L, name);
  free(name);

  return 1;
}

static bool
script_is_function(lua_State *L, char *name)
{
  bool ret;
  lua_getglobal(L, name);
  ret = lua_isfunction(L, -1);
  lua_pop(L, 1);
  return ret;
}

static bool
script_has_init(lua_State *L)
{
  return script_is_function(L, "init");
}

static bool
script_has_fini(lua_State *L)
{
  return script_is_function(L, "fini");
}

static bool
script_has_dump(lua_State *L)
{
  return script_is_function(L, "dump");
}

static bool
script_has_emit(lua_State *L)
{
  return script_is_function(L, "emit");
}

static void
script_exec_init(lua_State *L)
{
  if (!script_has_init(L)) {
    return;
  }

  lua_getglobal(L, "init");
  lua_call(L, 0, 0);
}

static void
script_exec_fini(lua_State *L)
{
  if (!script_has_fini(L)) {
    return;
  }

  lua_getglobal(L, "fini");
  lua_call(L, 0, 0);
}

int
script_create(struct ipft_script **scriptp, struct ipft_debuginfo *dinfo,
              const char *path)
{
  int error;
  lua_State *L;
  struct ipft_script *script;
  struct ipft_debuginfo **extraspace;

  if (path == NULL) {
    *scriptp = NULL;
    return 0;
  }

  script = malloc(sizeof(*script));
  if (script == NULL) {
    fprintf(stderr, "Cannot allocate memory\n");
    return -1;
  }

  L = luaL_newstate();

  /*
   * Load required libraries
   */
  luaL_openlibs(L);

  /*
   * Load BPF library
   */
  char bpf_lua_str[sizeof(bpf_lua) + 1] = {};
  memcpy(bpf_lua_str, bpf_lua, bpf_lua_len);
  (void)luaL_dostring(L, bpf_lua_str);

  /*
   * Register debuginfo functions
   */
  lua_newtable(L);
  lua_pushcfunction(L, script_ipft_offsetof);
  lua_setfield(L, -2, "offsetof");
  lua_pushcfunction(L, script_ipft_sizeof);
  lua_setfield(L, -2, "sizeof");
  lua_pushcfunction(L, script_ipft_typeof);
  lua_setfield(L, -2, "typeof");
  lua_setglobal(L, "ipft");

  /*
   * Bind debuginfo to Lua state
   */
  extraspace = (struct ipft_debuginfo **)lua_getextraspace(L);
  *extraspace = dinfo;

  /*
   * Load user script
   */
  error = luaL_dofile(L, path);
  if (error != 0) {
    const char *cause = lua_tostring(L, -1);
    fprintf(stderr, "Lua error: %s\n", cause);
    goto err0;
  }

  /*
   * Call init
   */
  script_exec_init(L);

  script->L = L;
  *scriptp = script;

  return 0;

err0:
  free(script);
  return -1;
}

void
script_destroy(struct ipft_script *script)
{
  if (script == NULL) {
    return;
  }
  script_exec_fini(script->L);
  lua_close(script->L);
  free(script);
}

int
script_exec_emit(struct ipft_script *script, struct bpf_insn **modp,
                 uint32_t *mod_cnt)
{
  size_t len;
  const char *raw;
  struct bpf_insn *mod;

  if (script == NULL) {
    *modp = NULL;
    *mod_cnt = 0;
    return 0;
  }

  if (!script_has_emit(script->L)) {
    return 0;
  }

  lua_getglobal(script->L, "emit");
  lua_call(script->L, 0, 1);

  raw = lua_tolstring(script->L, -1, &len);
  if (raw == NULL) {
    fprintf(stderr, "Failed to get module binary\n");
    return -1;
  }

  mod = malloc(len);
  if (mod == NULL) {
    perror("malloc");
    goto err0;
  }

  memcpy(mod, raw, len);

  *modp = mod;
  *mod_cnt = len / sizeof(*mod);

  lua_pop(script->L, 1);

  return 0;

err0:
  lua_pop(script->L, 1);
  return -1;
}

char *
script_exec_dump(struct ipft_script *script, uint8_t *data, size_t len)
{
  const char *dump;
  size_t dump_len;

  if (script == NULL) {
    return NULL;
  }

  if (!script_has_dump(script->L)) {
    return NULL;
  }

  lua_getglobal(script->L, "dump");
  lua_pushlstring(script->L, (char *)data, len);
  lua_call(script->L, 1, 1);

  dump = lua_tolstring(script->L, -1, &dump_len);
  if (dump == NULL) {
    fprintf(stderr, "lua_tolstring failed\n");
    return NULL;
  }

  lua_pop(script->L, 1);

  return strndup(dump, dump_len);
}
