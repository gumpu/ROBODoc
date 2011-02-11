#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <signal.h>

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;

static void
print_usage (const char *badoption)
{
  if (badoption[1] == 'e' || badoption[1] == 'l')
    {
      luai_writestringerror ("%s: ", progname);
      luai_writestringerror ("'%s' needs argument\n", badoption);
    }
  else
    {
      luai_writestringerror ("%s: ", progname);
      luai_writestringerror ("unrecognized option '%s'\n", badoption);
    }
  luai_writestringerror ("usage: %s [options] [script [args]]\n"
                         "Available options are:\n"
                         "  -e stat  execute string " LUA_QL ("stat") "\n"
                         "  -i       enter interactive mode after executing "
                         LUA_QL ("script") "\n"
                         "  -l name  require library " LUA_QL ("name") "\n"
                         "  -v       show version information\n"
                         "  --       stop handling options\n"
                         "  -        stop handling options and execute stdin\n",
                         progname);
}

static void
lstop (lua_State *L, lua_Debug *ar)
{
  (void) ar;                    /* unused arg. */
  lua_sethook (L, NULL, 0, 0);
  luaL_error (L, "interrupted!");
}

static void
laction (int i)
{
  signal (i, SIG_DFL);          /* if another SIGINT happens before lstop,
                                   terminate process (default action) */
  lua_sethook (globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int
traceback (lua_State *L)
{
  const char *msg = lua_tostring (L, 1);

  if (msg)
    luaL_traceback (L, L, msg, 1);
  else if (!lua_isnoneornil (L, 1))
    {                           /* is there an error object? */
      if (!luaL_callmeta (L, 1, "__tostring"))  /* try its 'tostring' metamethod */
        lua_pushliteral (L, "(no error message)");
    }
  return 1;
}

static void
l_message (const char *pname, const char *msg)
{
  if (pname)
    fprintf (stderr, "%s: ", pname);
  fprintf (stderr, "%s\n", msg);
  fflush (stderr);
}

static int
docall (lua_State *L, int narg, int nres)
{
  int status;

  int base = lua_gettop (L) - narg;     /* function index */

  lua_pushcfunction (L, traceback);     /* push traceback function */
  lua_insert (L, base);         /* put it under chunk and args */
  globalL = L;                  /* to be available to 'laction' */
  signal (SIGINT, laction);
  status = lua_pcall (L, narg, nres, base);
  signal (SIGINT, SIG_DFL);
  lua_remove (L, base);         /* remove traceback function */
  return status;
}

static int
runargs (lua_State *L, char **argv, int n)
{
  int i;

  for (i = 1; i < n; i++)
    {
      lua_assert (argv[i][0] == '-');
      switch (argv[i][1])
        {                       /* option */
#if 0
        case 'e':
          {
            const char *chunk = argv[i] + 2;

            if (*chunk == '\0')
              chunk = argv[++i];
            lua_assert (chunk != NULL);
            if (dostring (L, chunk, "=(command line)") != LUA_OK)
              return 0;
            break;
          }
#endif

#if 0
        case 'l':
          {
            const char *filename = argv[i] + 2;

            if (*filename == '\0')
              filename = argv[++i];
            lua_assert (filename != NULL);
            if (dolibrary (L, filename) != LUA_OK)
              return 0;         /* stop if file fails */
            break;
          }
#endif
        default:
          break;
        }
    }
  return 1;
}

/* check that argument has no extra characters at the end */
#define noextrachars(x)		{if ((x)[2] != '\0') return -1;}

static int
collectargs (char **argv, int *pi, int *pv, int *pe)
{
  int i;

  for (i = 1; argv[i] != NULL; i++)
    {
      if (argv[i][0] != '-')    /* not an option? */
        return i;
      switch (argv[i][1])
        {                       /* option */
        case '-':
          noextrachars (argv[i]);
          return (argv[i + 1] != NULL ? i + 1 : 0);
        case '\0':
          return i;
        case 'i':
          noextrachars (argv[i]);
          *pi = 1;              /* go through */
        case 'v':
          noextrachars (argv[i]);
          *pv = 1;
          break;
        case 'e':
          *pe = 1;              /* go through */
        case 'l':              /* both options need an argument */
          if (argv[i][2] == '\0')
            {                   /* no concatenated argument? */
              i++;              /* try next 'argv' */
              if (argv[i] == NULL || argv[i][0] == '-')
                return -(i - 1);        /* no next argument or it is another option */
            }
          break;
        default:               /* invalid option; return its index... */
          return -i;            /* ...as a negative value */
        }
    }
  return 0;
}

static int
getargs (lua_State *L, char **argv, int n)
{
  int narg;

  int i;

  int argc = 0;

  while (argv[argc])
    argc++;                     /* count total number of arguments */
  narg = argc - (n + 1);        /* number of arguments to the script */
  luaL_checkstack (L, narg + 3, "too many arguments to script");
  for (i = n + 1; i < argc; i++)
    lua_pushstring (L, argv[i]);
  lua_createtable (L, narg, n + 1);
  for (i = 0; i < argc; i++)
    {
      lua_pushstring (L, argv[i]);
      lua_rawseti (L, -2, i - n);
    }
  return narg;
}

static int
report (lua_State *L, int status)
{
  if (status != LUA_OK && !lua_isnil (L, -1))
    {
      const char *msg = lua_tostring (L, -1);

      if (msg == NULL)
        {
          msg = "(error object is not a string)";
        }
      l_message (progname, msg);
      lua_pop (L, 1);
      /* force a complete garbage collection in case of errors */
      lua_gc (L, LUA_GCCOLLECT, 0);
    }
  return status;
}

static int
handle_script (lua_State *L, char **argv, int n)
{
  int status;

  const char *fname;

  int narg = getargs (L, argv, n);      /* collect arguments */

  lua_setglobal (L, "arg");
  fname = argv[n];
  if (strcmp (fname, "-") == 0 && strcmp (argv[n - 1], "--") != 0)
    fname = NULL;               /* stdin */
  status = luaL_loadfile (L, fname);
  lua_insert (L, -(narg + 1));
  if (status == LUA_OK)
    status = docall (L, narg, LUA_MULTRET);
  else
    lua_pop (L, narg);
  return report (L, status);
}

static int
pmain (lua_State *L)
{
  int argc = (int) lua_tointeger (L, 1);

  char **argv = (char **) lua_touserdata (L, 2);

  int script;

  int has_i = 0, has_v = 0, has_e = 0;

  if (argv[0] && argv[0][0])
    {
      progname = argv[0];
    }
  script = collectargs (argv, &has_i, &has_v, &has_e);
  if (script < 0)
    {                           /* invalid arg? */
      print_usage (argv[-script]);
      return 0;
    }
/*  if (has_v) print_version(); */
  /* open standard libraries */
  luaL_checkversion (L);
  lua_gc (L, LUA_GCSTOP, 0);    /* stop collector during initialization */
  luaL_openlibs (L);            /* open libraries */
  lua_gc (L, LUA_GCRESTART, 0);
  /* run LUA_INIT */
/*  if (handle_luainit(L) != LUA_OK) return 0; */
  /* execute arguments -e and -l */
  if (!runargs (L, argv, (script > 0) ? script : argc))
    return 0;
  /* execute main script (if there is one) */
  if (script && handle_script (L, argv, script) != LUA_OK)
    return 0;
  lua_pushboolean (L, 1);       /* signal no errors */
  return 1;
}

/* the next function is called unprotected, so it must avoid errors */
static void
finalreport (lua_State *L, int status)
{
  if (status != LUA_OK)
    {
      const char *msg = (lua_type (L, -1) == LUA_TSTRING) ? lua_tostring (L,
                                                                          -1)
        : NULL;

      if (msg == NULL)
        {
          msg = "(error object is not a string)";
        }
      l_message (progname, msg);
      lua_pop (L, 1);
    }
}

int
main (int argc, char **argv)
{
  int status, result;

  lua_State *L = luaL_newstate ();      /* create state */

  if (L == NULL)
    {
      l_message (argv[0], "cannot create state: not enough memory");
      return EXIT_FAILURE;
    }
  /* call 'pmain' in protected mode */
  lua_pushcfunction (L, &pmain);
  lua_pushinteger (L, argc);    /* 1st argument */
  lua_pushlightuserdata (L, argv);      /* 2nd argument */
  status = lua_pcall (L, 2, 1, 0);
  result = lua_toboolean (L, -1);       /* get result */
  finalreport (L, status);
  lua_close (L);
  return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}
