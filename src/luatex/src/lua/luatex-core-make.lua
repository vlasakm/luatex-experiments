local byte, format, gmatch, gsub = string.byte, string.format, string.gmatch, string.gsub
local concat = table.concat

local data = io.loaddata('luatex-core.lua')

data = gsub(data,'%-%-%s*start%s*omit.-%-%-%s*stop%s*omit%s*','')
data = gsub(data,'\r\n','\n')

local t = { }
local r = { }
local n = 0
local s = utilities.merger.compact(data) -- no comments and less spaces

t[#t+1] = '/* generated from and by luatex-core.lua */'
t[#t+1] = ''
t[#t+1] = '#include "lua.h"'
t[#t+1] = '#include "lauxlib.h"'
t[#t+1] = ''
t[#t+1] = 'int load_luatex_core_lua (lua_State * L);'
t[#t+1] = ''
t[#t+1] = 'int load_luatex_core_lua (lua_State * L)'
t[#t+1] = '{'
t[#t+1] = '  static unsigned char luatex_core_lua[] = {'
for c in gmatch(data,'.') do
    if n == 16 then
        n = 1
        t[#t+1] = '    ' .. concat(r,', ') .. ','
    else
        n = n + 1
    end
    r[n] = format('0x%02x',byte(c))
end
n = n + 1
r[n] = '0x00'
t[#t+1] = '    ' .. concat(r,', ',1,n)
t[#t+1] = '  };'
t[#t+1] = '  return luaL_dostring(L, (const char*) luatex_core_lua);'
t[#t+1] = '}'

io.savedata('luatex-core.c',concat(t,'\n'))
