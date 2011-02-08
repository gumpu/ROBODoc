#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>

int main( int argc, char** argv )
{
    lua_State *L = luaL_newstate();

    lua_close( L );
    return EXIT_SUCCESS;
}
