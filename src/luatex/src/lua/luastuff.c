/*

luastuff.w

Copyright 2006-2013 Taco Hoekwater <taco@@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"
#include "lua/luatex-api.h"

#include <string.h>

lua_State *Luas = NULL;

int luastate_bytes = 0;
int lua_active = 0;

#define Luas_load(Luas,getS,ls,lua_id) \
    lua_load(Luas,getS,ls,lua_id,NULL);
#define Luas_open(name,luaopen_lib) \
    luaL_requiref(L, name, luaopen_lib, 1); \
    lua_pop(L, 1);

void make_table(lua_State * L, const char *tab, const char *mttab, const char *getfunc, const char *setfunc)
{
    /*tex make the table *//* |[{<tex>}]| */
    /*tex |[{<tex>},"dimen"]| */
    lua_pushstring(L, tab);
    /*tex |[{<tex>},"dimen",{}]| */
    lua_newtable(L);
    /*tex |[{<tex>}]| */
    lua_settable(L, -3);
    /*tex fetch it back */
    /*tex |[{<tex>},"dimen"]| */
    lua_pushstring(L, tab);
    /*tex |[{<tex>},{<dimen>}]| */
    lua_gettable(L, -2);
    /*tex make the meta entries */
    /*tex |[{<tex>},{<dimen>},{<dimen_m>}]| */
    luaL_newmetatable(L, mttab);
    /*tex |[{<tex>},{<dimen>},{<dimen_m>},"__index"]| */
    lua_pushstring(L, "__index");
    /*tex |[{<tex>},{<dimen>},{<dimen_m>},"__index","getdimen"]| */
    lua_pushstring(L, getfunc);
    /*tex |[{<tex>},{<dimen>},{<dimen_m>},"__index",<tex.getdimen>]| */
    lua_gettable(L, -5);
    /*tex |[{<tex>},{<dimen>},{<dimen_m>}]|  */
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");  /*tex |[{<tex>},{<dimen>},{<dimen_m>},"__newindex"]| */
    /*tex |[{<tex>},{<dimen>},{<dimen_m>},"__newindex","setdimen"]| */
    lua_pushstring(L, setfunc);
    /*tex |[{<tex>},{<dimen>},{<dimen_m>},"__newindex",<tex.setdimen>]| */
    lua_gettable(L, -5);
    /*tex |[{<tex>},{<dimen>},{<dimen_m>}]| */
    lua_settable(L, -3);
    /*tex |[{<tex>},{<dimen>}]| : assign the metatable */
    lua_setmetatable(L, -2);
    /*tex |[{<tex>}]| : clean the stack */
    lua_pop(L, 1);
}

static const char *getS(lua_State * L, void *ud, size_t * size)
{
    LoadS *ls = (LoadS *) ud;
    (void) L;
    if (ls->size == 0)
        return NULL;
    *size = ls->size;
    ls->size = 0;
    return ls->s;
}

static void *my_luaalloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    void *ret = NULL;
    /*tex define |ud| for -Wunused */
    (void) ud;
    if (nsize == 0)
        free(ptr);
    else
        ret = realloc(ptr, nsize);
    luastate_bytes += (int) (nsize - osize);
    return ret;
}

static int my_luapanic(lua_State * L)
{
    /*tex define |L| to avoid warnings */
    (void) L;
    fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(L, -1));
    return 0;
}

void luafunctioncall(int slot)
{
    int i ;
    int stacktop = lua_gettop(Luas);
    lua_active++;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, lua_key_index(lua_functions));
    lua_gettable(Luas, LUA_REGISTRYINDEX);
    lua_rawgeti(Luas, -1,slot);
    if (lua_isfunction(Luas,-1)) {
        /*tex function index */
        int base = lua_gettop(Luas);
        lua_pushinteger(Luas, slot);
        /* push traceback function */
        lua_pushcfunction(Luas, lua_traceback);
        /*tex put it under chunk  */
        lua_insert(Luas, base);
        ++function_callback_count;
        i = lua_pcall(Luas, 1, 0, base);
        /*tex remove traceback function */
        lua_remove(Luas, base);
        if (i != 0) {
            lua_gc(Luas, LUA_GCCOLLECT, 0);
            Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        }
    }
    lua_settop(Luas,stacktop);
    lua_active--;
}

static const luaL_Reg lualibs[] = {
    /*tex standard \LUA\ libraries */
    { "_G",        luaopen_base },
    { "package",   luaopen_package },
    { "table",     luaopen_table },
    { "io",        luaopen_io },
    { "os",        luaopen_os },
    { "string",    luaopen_string },
    { "math",      luaopen_math },
    { "debug",     luaopen_debug },
    { "utf8",      luaopen_utf8 },
    { "coroutine", luaopen_coroutine },
    { NULL,        NULL }
};

static void do_openlibs(lua_State * L)
{
    const luaL_Reg *lib = lualibs;
    for (; lib->func; lib++) {
        Luas_open(lib->name,lib->func);
    }
}

static int load_aux (lua_State *L, int status) {
    if (status == 0)
        /*tex okay */
        return 1;
    else {
        /*tex return nil plus error message */
        lua_pushnil(L);
        /*tex put before error message */
        lua_insert(L, -2);
        return 2;
    }
}

static int luatex_loadfile (lua_State *L) {
    int status = 0;
    const char *fname = luaL_optstring(L, 1, NULL);
    const char *mode = luaL_optstring(L, 2, NULL);
    /*tex the |env| parameter */
    int env = !lua_isnone(L, 3);
    if (!lua_only && !fname && interaction == batch_mode) {
        /*tex return |nil| plus error message */
        lua_pushnil(L);
        lua_pushstring(L, "reading from stdin is disabled in batch mode");
        return 2;
    }
    status = luaL_loadfilex(L, fname, mode);
    if (status == LUA_OK) {
        recorder_record_input(fname);
        if (env) {
            /*tex the |env| parameter */
            lua_pushvalue(L, 3);
            /*tex set it as first upvalue of loaded chunk */
            lua_setupvalue(L, -2, 1);
        }
    }
    return load_aux(L, status);
}

static int luatex_dofile (lua_State *L) {
    const char *fname = luaL_optstring(L, 1, NULL);
    int n = lua_gettop(L);
    if (!lua_only && !fname) {
        if (interaction == batch_mode) {
            /*tex return |nil| plus error message */
            lua_pushnil(L);
            lua_pushstring(L, "reading from stdin is disabled in batch mode");
            return 2;
        } else {
            tprint_nl("lua> ");
        }
    }
    if (luaL_loadfile(L, fname) != 0)
        lua_error(L);
    recorder_record_input(fname);
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

void luainterpreter(void)
{
    lua_State *L;
    L = lua_newstate(my_luaalloc, NULL);
    if (L == NULL) {
        fprintf(stderr, "Can't create the Lua state.\n");
        return;
    }
    lua_atpanic(L, &my_luapanic);
    /*tex This initializes all the `simple' libraries: */
    do_openlibs(L);
    lua_pushcfunction(L,luatex_dofile);
    lua_setglobal(L, "dofile");
    lua_pushcfunction(L,luatex_loadfile);
    lua_setglobal(L, "loadfile");
    open_strlibext(L);
    /*tex our own libraries register themselves */
    luaopen_fio(L);
    luaopen_tex(L);
    luaopen_token(L);
    luaopen_node(L);
    luaopen_texio(L);
    luaopen_callback(L);
    /*tex now we plug in extra \LUA\ startup code */
    luaopen_lua(L, startup_filename);
    /*tex and open some \TEX\ ones */
    luaopen_stats(L);
    luaopen_font(L);
    luaopen_lang(L);
    lua_createtable(L, 0, 0);
    lua_setglobal(L, "texconfig");
    Luas = L;
}

int hide_lua_table(lua_State * L, const char *name)
{
    int r = 0;
    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        r = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
        lua_setglobal(L, name);
    }
    return r;
}

void unhide_lua_table(lua_State * L, const char *name, int r)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, r);
    lua_setglobal(L, name);
    luaL_unref(L, LUA_REGISTRYINDEX, r);
}

int hide_lua_value(lua_State * L, const char *name, const char *item)
{
    int r = 0;
    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, item);
        r = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
        lua_setfield(L, -2, item);
    }
    return r;
}

void unhide_lua_value(lua_State * L, const char *name, const char *item, int r)
{
    lua_getglobal(L, name);
    if (lua_istable(L, -1)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, r);
        lua_setfield(L, -2, item);
        luaL_unref(L, LUA_REGISTRYINDEX, r);
    }
}

int lua_traceback(lua_State * L)
{
    lua_getglobal(L, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    /*tex pass error message */
    lua_pushvalue(L, 1);
    /*tex skip this function and traceback */
    lua_pushinteger(L, 2);
    /*tex call |debug.traceback| */
    lua_call(L, 2, 1);
    return 1;
}

static void luacall(int p, int nameptr, boolean is_string)
{
    LoadS ls;
    int i;
    size_t ll = 0;
    char *lua_id;
    char *s = NULL;
    int stacktop = lua_gettop(Luas);
    if (Luas == NULL) {
        luainterpreter();
    }
    lua_active++;
    if (is_string) {
        const char *ss = NULL;
        lua_rawgeti(Luas, LUA_REGISTRYINDEX, p);
        if (lua_isfunction(Luas,-1)) {
            /*tex function index */
            int base = lua_gettop(Luas);
            lua_checkstack(Luas, 1);
            /*tex push traceback function */
            lua_pushcfunction(Luas, lua_traceback);
            /*tex put it under chunk  */
            lua_insert(Luas, base);
            ++late_callback_count;
            i = lua_pcall(Luas, 0, 0, base);
            /*tex remove traceback function */
            lua_remove(Luas, base);
            if (i != 0) {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
            lua_settop(Luas,stacktop);
            lua_active--;
            return ;
        }
        ss = lua_tolstring(Luas, -1, &ll);
        s = xmalloc(ll+1);
        memcpy(s,ss,ll+1);
        lua_pop(Luas,1);
    } else {
        int l = 0;
        s = tokenlist_to_cstring(p, 1, &l);
        ll = (size_t)l;
    }
    ls.s = s;
    ls.size = ll;
    if (ls.size > 0) {
        if (nameptr > 0) {
            /*tex |l| is not used */
            int l = 0;
            lua_id = tokenlist_to_cstring(nameptr, 1, &l);
            i = Luas_load(Luas, getS, &ls, lua_id);
            xfree(lua_id);
        } else if (nameptr < 0) {
            lua_id = get_lua_name((nameptr + 65536));
            if (lua_id != NULL) {
                i = Luas_load(Luas, getS, &ls, lua_id);
            } else {
                i = Luas_load(Luas, getS, &ls, "=[\\latelua]");
            }
        } else {
            i = Luas_load(Luas, getS, &ls, "=[\\latelua]");
        }
        if (i != 0) {
            Luas = luatex_error(Luas, (i == LUA_ERRSYNTAX ? 0 : 1));
        } else {
            /*tex function index */
            int base = lua_gettop(Luas);
            lua_checkstack(Luas, 1);
            /*tex push traceback function */
            lua_pushcfunction(Luas, lua_traceback);
            /*tex put it under chunk  */
            lua_insert(Luas, base);
            ++late_callback_count;
            i = lua_pcall(Luas, 0, 0, base);
            /*tex remove traceback function */
            lua_remove(Luas, base);
            if (i != 0) {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
        }
        xfree(ls.s);
    }
    lua_settop(Luas,stacktop);
    lua_active--;
}

void luacall_vf(int p, int f, int c)
{
    int i;
    int stacktop = lua_gettop(Luas);
    if (Luas == NULL) {
        luainterpreter();
    }
    lua_active++;
    lua_rawgeti(Luas, LUA_REGISTRYINDEX, p);
    if (lua_isfunction(Luas,-1)) {
        /*tex function index */
        int base = lua_gettop(Luas);
        lua_checkstack(Luas, 1);
        /*tex push traceback function */
        lua_pushcfunction(Luas, lua_traceback);
        /*tex put it under chunk  */
        lua_insert(Luas, base);
        lua_pushinteger(Luas, f);
        lua_pushinteger(Luas, c);
        ++late_callback_count;
        i = lua_pcall(Luas, 2, 0, base);
        /*tex remove traceback function */
        lua_remove(Luas, base);
        if (i != 0) {
            lua_gc(Luas, LUA_GCCOLLECT, 0);
            Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
        }
    } else {
        LoadS ls;
        size_t ll = 0;
        char *s = NULL;
        const char *ss = NULL;
        ss = lua_tolstring(Luas, -1, &ll);
        s = xmalloc(ll+1);
        memcpy(s,ss,ll+1);
        lua_pop(Luas,1);
        ls.s = s;
        ls.size = ll;
        if (ls.size > 0) {
            i = Luas_load(Luas, getS, &ls, "=[vf command]");
            if (i != 0) {
                Luas = luatex_error(Luas, (i == LUA_ERRSYNTAX ? 0 : 1));
            } else {
                int base = lua_gettop(Luas);        /* function index */
                lua_checkstack(Luas, 1);
                lua_pushcfunction(Luas, lua_traceback);     /* push traceback function */
                lua_insert(Luas, base);     /* put it under chunk  */
                ++late_callback_count;
                i = lua_pcall(Luas, 0, 0, base);
                lua_remove(Luas, base);     /* remove traceback function */
                if (i != 0) {
                    lua_gc(Luas, LUA_GCCOLLECT, 0);
                    Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
                }
            }
            xfree(ls.s);
        }
    }
    lua_settop(Luas,stacktop);
    lua_active--;
}

// TODO(mvlasak): why pdf parameter?
//void late_lua(PDF pdf, halfword p)
//{
//    halfword t;
//    (void) pdf;
//    t = late_lua_type(p);
//    if (t == normal) {
//        /*tex sets |def_ref| */
//        expand_macros_in_tokenlist(p);
//        luacall(def_ref, late_lua_name(p), false);
//        flush_list(def_ref);
//    } else if (t == lua_refid_call) {
//        luafunctioncall(late_lua_data(p));
//    } else if (t == lua_refid_literal) {
//        luacall(late_lua_data(p), late_lua_name(p), true);
//    } else {
//        /*tex Let's just ignore it, could be some user specific thing. */
//    }
//}

void luatokencall(int p, int nameptr)
{
    LoadS ls;
    int i;
    int l = 0;
    char *s = NULL;
    char *lua_id;
    int stacktop = lua_gettop(Luas);
    lua_active++;
    s = tokenlist_to_cstring(p, 1, &l);
    ls.s = s;
    ls.size = (size_t) l;
    if (ls.size > 0) {
        if (nameptr > 0) {
            lua_id = tokenlist_to_cstring(nameptr, 1, &l);
            i = Luas_load(Luas, getS, &ls, lua_id);
            xfree(lua_id);
        } else if (nameptr < 0) {
            lua_id = get_lua_name((nameptr + 65536));
            if (lua_id != NULL) {
                i = Luas_load(Luas, getS, &ls, lua_id);
            } else {
                i = Luas_load(Luas, getS, &ls, "=[\\directlua]");
            }
        } else {
            i = Luas_load(Luas, getS, &ls, "=[\\directlua]");
        }
        xfree(s);
        if (i != 0) {
            Luas = luatex_error(Luas, (i == LUA_ERRSYNTAX ? 0 : 1));
        } else {
            /*tex function index */
            int base = lua_gettop(Luas);
            lua_checkstack(Luas, 1);
            /*tex push traceback function */
            lua_pushcfunction(Luas, lua_traceback);
            /*tex put it under chunk  */
            lua_insert(Luas, base);
            ++direct_callback_count;
            i = lua_pcall(Luas, 0, 0, base);
            /*tex remove traceback function */
            lua_remove(Luas, base);
            if (i != 0) {
                lua_gc(Luas, LUA_GCCOLLECT, 0);
                Luas = luatex_error(Luas, (i == LUA_ERRRUN ? 0 : 1));
            }
        }
    }
    lua_settop(Luas,stacktop);
    lua_active--;
}

lua_State *luatex_error(lua_State * L, int is_fatal)
{
    const_lstring luaerr;
    char *err = NULL;
    if (lua_type(L, -1) == LUA_TSTRING) {
        luaerr.s = lua_tolstring(L, -1, &luaerr.l);
        /*tex
            Free the last one.
        */
        err = (char *) xmalloc((unsigned) (luaerr.l + 1));
        snprintf(err, (luaerr.l + 1), "%s", luaerr.s);
        /*tex
            What if we have several .. not freed?
        */
        last_lua_error = err;
    }
    if (is_fatal > 0) {
        /*
            Normally a memory error from lua. The pool may overflow during the
            |maketexlstring()|, but we are crashing anyway so we may as well
            abort on the pool size
        */
        normal_error("lua",err);
        /*tex
            This is never reached.
        */
        lua_close(L);
        return (lua_State *) NULL;
    } else {
        normal_warning("lua",err);
        return L;
    }
}

void preset_environment(lua_State * L, const parm_struct * p, const char *s)
{
    int i;
    assert(L != NULL);
    /*tex double call with same s gives assert(0) */
    lua_pushstring(L, s);
    /*tex state: s */
    lua_gettable(L, LUA_REGISTRYINDEX);
    /*tex state: t */
    assert(lua_isnil(L, -1));
    lua_pop(L, 1);
    /*tex state: - */
    lua_pushstring(L, s);
    /*tex state: s */
    lua_newtable(L);
    /*tex state: t s */
    for (i = 1, ++p; p->name != NULL; i++, p++) {
        assert(i == p->idx);
        lua_pushstring(L, p->name);
        /*tex state: k t s */
        lua_pushinteger(L, p->idx);
        /*tex state: v k t s */
        lua_settable(L, -3);
        /*tex state: t s */
    }
    lua_settable(L, LUA_REGISTRYINDEX);
    /* tex state: - */
}
