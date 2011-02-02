/* vi: spell ff=unix 
 */

/*
Copyright (C) 1994-2007  Frans Slothouber, Jacco van Weert, Petteri Kettunen,
Bernd Koesling, Thomas Aglassinger, Anthon Pang, Stefan Kost, David Druffner,
Sasha Vasko, Kai Hofmann, Thierry Pierron, Friedrich Haase, and Gergely Budai.

This file is part of ROBODoc

ROBODoc is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


/****h* ROBODoc/Configuration
 * FUNCTION
 *   Functions to access the ROBODoc configuration and configuration
 *   file (robodoc.rc) or the file specified with the --rc option.
 *
 *   The robodoc.rc file consists of a number of blocks.  Each
 *   block starts with a line of the form
 *
 *     <block name>:
 *
 *   This is followed by a number of lines of data.  Each line starts
 *   with at least one space followed by the actual data.
 *
 *   This module parses this data and stores it in the global
 *   configuration.
 *
 * NOTES
 *   Is missing a lot of documentation.
 *
 *   You can not use RB_Say() in this module since the --tell flag
 *   won't be parsed until after this module has finished.
 *
 ******
 * $Id: roboconfig.c,v 1.57 2008/06/17 11:49:28 gumpu Exp $
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "headertypes.h"
#include "util.h"
#include "roboconfig.h"
#include "globals.h"
#include "optioncheck.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


/****v* Configuration/default_item_names
 * FUNCTION
 *   Defines the names of items that ROBODoc recognized as
 *   items by default if none are specified in the
 *   robodoc.rc file.
 * SOURCE
 */

static char        *default_item_names[] = {
    "SOURCE",                   /* source code inclusion */
    "NAME",                     /* Item name + short description */
    "COPYRIGHT",                /* who own the copyright : "(c) <year>-<year> by <company/person>" */
    "SYNOPSIS", "USAGE",        /* how to use it */
    "FUNCTION", "DESCRIPTION", "PURPOSE",       /* what does it */
    "AUTHOR",                   /* who wrote it */
    "CREATION DATE",            /* when did the work start */
    "MODIFICATION HISTORY", "HISTORY",  /* who done what changes when */
    "INPUTS", "ARGUMENTS", "OPTIONS", "PARAMETERS", "SWITCHES", /* what can we feed into it */
    "OUTPUT", "SIDE EFFECTS",   /* what output will be made */
    "RESULT", "RETURN VALUE",   /* what do we get returned */
    "EXAMPLE",                  /* a clear example of the items use */
    "NOTES",                    /* any annotations */
    "DIAGNOSTICS",              /* diagnostical output */
    "WARNINGS", "ERRORS",       /* warning & error-messages */
    "BUGS",                     /* known bugs */
    "TODO", "IDEAS",            /* what to implement next & ideas */
    "PORTABILITY",              /* where does it come from, where will it work */
    "SEE ALSO",                 /* references */
    "METHODS", "NEW METHODS",   /* oop methods */
    "ATTRIBUTES", "NEW ATTRIBUTES",     /* oop attributes */
    "TAGS",                     /* tagitem description */
    "COMMANDS",                 /* command description */
    "DERIVED FROM",             /* oop super class */
    "DERIVED BY",               /* oop sub class */
    "USES", "CHILDREN",         /* what modules are used by this one */
    "USED BY", "PARENTS",       /* which modules do use this */
    NULL,                       /* don't delete, so we can count how many there are... */
};

/***********/

static char        *default_remark_begin_markers[] = {
    "/*",
    "(*",
    "<!--",
    "{*",
    NULL
};

static char        *default_remark_end_markers[] = {
    "*/",
    "*)",
    "-->",
    "*}",
    NULL
};

/****v* Configuration/c_keywords
 * FUNCTION
 *   The default C keywords.
 * SOURCE
 */

static char        *c_keywords[] = {

    /* ANSI C Keywords */
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",

    /* Some preprocessor directives */
    "#include",
    "#define",
    "#undef",
    "#if",
    "#else",
    "#elif",
    "#endif",
    "#ifdef",
    "#ifndef",
    "#pragma",

    NULL,                       /* don't delete, so we can count how many there are... */
};

/* C comments */
#define C_LINE_COMMENT        "//"
#define C_BLOCK_COMMENT_START "/*"
#define C_BLOCK_COMMENT_END   "*/"

/***********/


/* Default header separation character */
static char        *default_header_separate_chars[] = {
    ",",
    NULL
};


/* Default header ignore character (to include remarks, version data, etc...) */
static char        *default_header_ignore_chars[] = {
    "[",
    NULL
};


/* Maximum length of a line in the configuration file */
#define BUFFER_LENGTH 2048


/****v* Configuration/configuration
 * FUNCTION
 *   This global stores all the configuration parameters specified on
 *   the command line and in the robodoc.rc file.
 * SOURCE
 */

struct RB_Configuration configuration;

/*****/


static T_Block_Kind BlockKind(
    char *line );
static T_Line_Kind  ConfigLineKind(
    char *line );
static void         SecondScan(
    FILE *f );
static void         Alloc_Parameters(
    struct Parameters *parameters,
    unsigned int size );
static void         AddParameter(
    char *name,
    struct Parameters *parameters );
static void         GetParameters(
    char *line,
    struct Parameters *parameters );
static void         Install_Custom_HeaderTypes(
    void );
static void         Complement_Remark_Markers(
    void );
static void         ComplementItemNames(
    void );
static void         ComplementHeaderMarkers(
    void );
static char        *Get_rc(
    char *rcname );


/****v* Configuration/keywords_hash_mask
 * FUNCTION
 *   Mask for keyword hash function.
 *   This mask reduces the hash value for the actual hash table size.
 *   Also the size of the hash table can be derived from this mask:
 *     hash table size = keywords_hash_mask + 1
 * SOURCE
 */
static unsigned int keywords_hash_mask;

/*****/

/****v* Configuration/keywords_hash
 * FUNCTION
 *   This is the hash table for the keywords.
 *   See keywords_hash_s.
 * SOURCE
 */
static struct keywords_hash_s **keywords_hash;

/*****/

/****f* Configuration/allocate_keywords_hash_table
 * FUNCTION
 *   Allocates space for the keyword hash table.
 *
 *   The size of the table depends on the number of keywords rounded up to the
 *   next power of two.
 * SYNOPSIS
 */
void allocate_keywords_hash_table(
    void )
/*
 * SOURCE
 */
{
    unsigned int        i;

    /* Calculate hash table size (powers of two) */
    for ( keywords_hash_mask = 2;
          keywords_hash_mask < configuration.keywords.number;
          keywords_hash_mask <<= 1 );
    keywords_hash_mask -= 1;

    /* Allocate space for hash table */
    keywords_hash = RB_malloc( ( keywords_hash_mask + 1 ) *
                               sizeof( struct keywords_hash_s * ) );

    /* Zero out all rows */
    for ( i = 0; i <= keywords_hash_mask; i++ )
    {
        keywords_hash[i] = NULL;
    }
}

/*****/

/****f* Hash_Keyword/HASH_MIX
 * FUNCTION
 *   Helper macro for the function Hash_Keyword( )
 * SOURCE
 */
#define HASH_MIX(a,b,c)             \
{                                   \
  a -= b; a -= c; a ^= ( c >> 13 ); \
  b -= c; b -= a; b ^= ( a << 8 );  \
  c -= a; c -= b; c ^= ( b >> 13 ); \
  a -= b; a -= c; a ^= ( c >> 12 ); \
  b -= c; b -= a; b ^= ( a << 16 ); \
  c -= a; c -= b; c ^= ( b >> 5 );  \
  a -= b; a -= c; a ^= ( c >> 3 );  \
  b -= c; b -= a; b ^= ( a << 10 ); \
  c -= a; c -= b; c ^= ( b >> 15 ); \
}

/*****/

/****f* Configuration/Hash_Keyword
 * FUNCTION
 *   Calculate the hash value for a string
 *
 *   The hash value is a 32 bit integer based on the hash function written by
 *   Bob Jenkins. It is then reduced by an AND operation to the actual size of
 *   the hash table.
 * SYNOPSIS
 */
unsigned long Hash_Keyword(
    char *key,
    unsigned long keylen )
/*
 * INPUTS
 *   - key    -- The keyword string
 *   - keylen -- The length of the keyword string
 * RETURN VALUE
 *   The hash value for the keyword.
 * SOURCE
 */
{
    unsigned long       bkt, i, j, k;

    bkt = 0xfeedbeef;
    i = j = 0x9e3779b9;
    k = keylen;

    while ( k >= 12 )
    {
        i += ( key[0] + ( ( unsigned ) key[1] << 8 )
               + ( ( unsigned ) key[2] << 16 )
               + ( ( unsigned ) key[3] << 24 ) );
        j += ( key[4] + ( ( unsigned ) key[5] << 8 )
               + ( ( unsigned ) key[6] << 16 )
               + ( ( unsigned ) key[7] << 24 ) );
        bkt += ( key[8] + ( ( unsigned ) key[9] << 8 )
                 + ( ( unsigned ) key[10] << 16 )
                 + ( ( unsigned ) key[11] << 24 ) );

        HASH_MIX( i, j, bkt );

        key += 12;
        k -= 12;
    }

    bkt += keylen;

    switch ( k )
    {
    case 11:
        bkt += ( ( unsigned ) key[10] << 24 );
    case 10:
        bkt += ( ( unsigned ) key[9] << 16 );
    case 9:
        bkt += ( ( unsigned ) key[8] << 8 );
    case 8:
        j += ( ( unsigned ) key[7] << 24 );
    case 7:
        j += ( ( unsigned ) key[6] << 16 );
    case 6:
        j += ( ( unsigned ) key[5] << 8 );
    case 5:
        j += key[4];
    case 4:
        i += ( ( unsigned ) key[3] << 24 );
    case 3:
        i += ( ( unsigned ) key[2] << 16 );
    case 2:
        i += ( ( unsigned ) key[1] << 8 );
    case 1:
        i += key[0];
    }

    HASH_MIX( i, j, bkt );

    return ( bkt & keywords_hash_mask );
}

/*****/


/****f* Configuration/add_to_keywords_hash_table
 * FUNCTION
 *   Add a keyword to the hash table
 * SYNOPSIS
 */
void add_to_keywords_hash_table(
    char *keyword )
/*
 * INPUTS
 *   keyword -- The keyword string
 * SOURCE
 */
{
    struct keywords_hash_s *tmp, **curr;
    unsigned long       hash;

    /* Allocate space for new entry in hash table */
    tmp = RB_malloc( sizeof( struct keywords_hash_s ) );
    /* and initialise it */
    tmp->keyword = keyword;
    tmp->next = NULL;

    /* Calculate hash value */
    hash = Hash_Keyword( keyword, strlen( keyword ) );

    /* Seek to last element in hash table row */
    for ( curr = &( keywords_hash[hash] ); *curr;
          curr = &( ( *curr )->next ) );

    /* Insert entry into hash table */
    *curr = tmp;
}

/*****/


/****f* Configuration/Find_Keyword
 * FUNCTION
 *   Find a keyword in the hash table
 * SYNOPSIS
 */
char               *Find_Keyword(
    char *keyword,
    int len )
/*
 * INPUTS
 *   - keyword -- The keyword string
 *   - len     -- The length of the keyword string
 * RETURN VALUE
 *   - pointer to the found keyword string in hash table or
 *   - NULL if the keyword is not found
 * SOURCE
 */
{
    unsigned long       hash;
    struct keywords_hash_s *curr;

    /* Calculate hash value */
    hash = Hash_Keyword( keyword, len );

    /* Seek through hash table row */
    for ( curr = keywords_hash[hash]; curr; curr = curr->next )
    {
        /* Check for keyword in row element */
        if ( !strncmp( keyword, curr->keyword, len ) )
        {
            /* Found it! */
            return curr->keyword;
        }
    }

    /* Keyword not found */
    return NULL;
}

/*****/

/****f* Configuration/add_keywords_to_hash_table
 * FUNCTION
 *   Initalize hash table and add all keywords from configuration.keywords
 *   to the hash table
 * SOURCE
 */
void add_keywords_to_hash_table(
    void )
{
    unsigned int        i;

    /* If nothing to add, exit */
    if ( !configuration.keywords.number )
        return;

    /* Make some allocations */
    allocate_keywords_hash_table(  );

    /* Add keywords to hash table */
    for ( i = 0; i < configuration.keywords.number; i++ )
    {
        add_to_keywords_hash_table( configuration.keywords.names[i] );
    }
}

/*****/


/* TODO Documentation */
static void AllocOptions(
    unsigned int argc,
    char **argv )
{
    unsigned int        i;

    Alloc_Parameters( &( configuration.options ), argc );
    for ( i = 0; i < argc; ++i )
    {
        AddParameter( argv[i], &( configuration.options ) );
    }
}


/* TODO Documentation */

char               *Get_rc(
    char *rcname )
{
    char               *s = NULL;
    char               *s2 = NULL;
    char               *path = NULL;

    if ( Stat_Path( 'e', rcname ) && Stat_Path( 'f', rcname ) )
    {
        return RB_StrDup( rcname );
    }
    else
    {
        if ( strchr( rcname, ':' ) || strchr( rcname, '/' ) )
        {
            /* The rc names is a proper path, and not just a filename,
             * we stop searching */
        }
        else
        {
            s = getenv( "HOME" );
            if ( !s )
            {
                if ( ( s = getenv( "HOMEDRIVE" ) )
                     && ( s2 = getenv( "HOMEPATH" ) ) )
                {
                    /* HOMEDRIVE includes backslash */
                    path =
                        ( char * ) malloc( sizeof( char ) *
                                           ( strlen( s ) + strlen( s2 ) + 1 +
                                             1 + strlen( rcname ) ) );
                    sprintf( path, "%s%s%c%s", s, s2, '\\', rcname );
                }
                else
                {
                    return NULL;
                }
            }
            else
            {
                path =
                    ( char * ) malloc( sizeof( char ) *
                                       ( strlen( s ) + 1 + 1 +
                                         strlen( rcname ) ) );
                sprintf( path, "%s%c%s", s, '/', rcname );
            }

            if ( path && Stat_Path( 'e', path ) && Stat_Path( 'f', path ) )
            {
                return path;
            }
            else
            {
                char* sitespecific =
#ifdef ROBO_PREFIX
                ROBO_PREFIX
                "/share/robodoc/";
#else
                "/usr/local/etc/robodoc/";
#endif

                if ( path )
                {
                    free( path );
                }

                path =
                    ( char * ) malloc( sizeof( char ) *
                                       ( strlen( sitespecific ) + 1 +
                                         strlen( rcname ) ) );
                sprintf( path, "%s%s", sitespecific, rcname );

                /* default failed -- try site-specific config file */
                if ( Stat_Path( 'e', path ) && Stat_Path( 'f', path ) )
                {
                    /* site-specific file can be stat'ed */
                    return path;
                }
                else
                {
                    free( path );
                    return NULL;
                }
            }
        }
    }
    return NULL;
}


/****f* Configuration/ReadConfiguration
 * FUNCTION
 *   Read the robodoc configuration file, and create
 *   a RB_Configuration structure.
 * SYNOPSIS
 */

char               *ReadConfiguration(
    unsigned int argc,
    char **argv,
    char *filename )
/*
 * INPUTS
 *   o argc -- the arg count as received by main()
 *   o argv -- the arg valules as received by main()
 *   o filename -- an optional filename.  If none is given,
 *               "robodoc.rc" is used.
 * RESULT
 *   An initialized configuration (a global).
 * SOURCE
 */
{
    FILE               *f = NULL;
    char               *path = NULL;

    if ( filename )
    {
        path = Get_rc( filename );
        if ( path )
        {
            f = fopen( path, "r" );
        }
        if ( !f )
        {
            /* It should open as the user claimed it exists somewhere */
            RB_Panic( "Can't open %s\n", filename );
        }
    }
    else
    {
        /* Try the default rc file */
        path = Get_rc( "robodoc.rc" );
        if ( path )
        {
            f = fopen( path, "r" );
        }
    }

    AllocOptions( argc, argv );
    Alloc_Parameters( &( configuration.items ), 10 );
    Alloc_Parameters( &( configuration.ignore_items ), 10 );
    Alloc_Parameters( &( configuration.source_items ), 10 );
    Alloc_Parameters( &( configuration.preformatted_items ), 10 );
    Alloc_Parameters( &( configuration.format_items ), 10 );
    Alloc_Parameters( &( configuration.item_order ), 10 );

    Alloc_Parameters( &( configuration.custom_headertypes ), 10 );
    Alloc_Parameters( &( configuration.ignore_files ), 10 );
    Alloc_Parameters( &( configuration.accept_files ), 10 );
    Alloc_Parameters( &( configuration.header_markers ), 10 );
    Alloc_Parameters( &( configuration.remark_markers ), 10 );
    Alloc_Parameters( &( configuration.end_markers ), 10 );
    Alloc_Parameters( &( configuration.remark_begin_markers ), 10 );
    Alloc_Parameters( &( configuration.remark_end_markers ), 10 );
    Alloc_Parameters( &( configuration.keywords ), 10 );
    Alloc_Parameters( &( configuration.source_line_comments ), 10 );
    Alloc_Parameters( &( configuration.header_ignore_chars ), 10 );
    Alloc_Parameters( &( configuration.header_separate_chars ), 10 );

    if ( f )
    {
        SecondScan( f );
        fclose( f );
    }
    else
    {
        /* No .rc file found.  That's OK */
    }
    ComplementItemNames(  );
    ComplementHeaderMarkers(  );
    Complement_Remark_Markers(  );
    Install_Custom_HeaderTypes(  );

    /* Make keywords hash table (if necessarry) */
    add_keywords_to_hash_table(  );

    assert( configuration.items.number );

    return path;
}

/******/


/* TODO Documentation */
static void Complement_Remark_Markers(
    void )
{
    unsigned int        i;

    if ( configuration.remark_begin_markers.number )
    {
        /* The user specified his own remark_begin_markers */
    }
    else
    {
        for ( i = 0; default_remark_begin_markers[i]; ++i )
        {
            AddParameter( default_remark_begin_markers[i],
                          &( configuration.remark_begin_markers ) );
        }
    }

    if ( configuration.remark_end_markers.number )
    {
        /* The user specified his own remark_end_markers */
    }
    else
    {
        for ( i = 0; default_remark_end_markers[i]; ++i )
        {
            AddParameter( default_remark_end_markers[i],
                          &( configuration.remark_end_markers ) );
        }
    }
}


/****f* Configuration/Find_Parameter_Exact
 * FUNCTION
 *   Checks for the existence of a given configuration parameter
 *   (exact string match)
 * SOURCE
 */
char               *Find_Parameter_Exact(
    struct Parameters *params,
    char *paramname )
{
    unsigned int        i;

    /* we are looking for an exact match */
    for ( i = 0; i < params->number; i++ )
    {
        if ( !strcmp( params->names[i], paramname ) )
        {
            /* found it */
            return params->names[i];
        }
    }

    /* parameter not found */
    return NULL;
}

/******/

/****f* Configuration/Find_Parameter_Partial
 * FUNCTION
 *   Checks for the existence of a given configuration parameter
 *   (partial string match)
 * SOURCE
 */
char               *Find_Parameter_Partial(
    struct Parameters *params,
    char *paramname )
{
    unsigned int        i;

    /* we are looking for a not exact match */
    for ( i = 0; i < params->number; i++ )
    {
        if ( !strncmp
             ( params->names[i], paramname, strlen( params->names[i] ) ) )
        {
            /* found it */
            return params->names[i];
        }
    }

    /* parameter not found */
    return NULL;
}

/******/


/****f* Configuration/Find_Parameter_Char
 * FUNCTION
 *   Checks for the existence of a given configuration parameter
 *   (Character match)
 * SOURCE
 */
char               *Find_Parameter_Char(
    struct Parameters *params,
    char param )
{
    unsigned int        i;

    for ( i = 0; i < params->number; i++ )
    {
        if ( params->names[i][0] == param )
        {
            /* found it */
            return params->names[i];
        }
    }

    /* parameter not found */
    return NULL;
}

/******/


/****f* Configuration/Install_C_Syntax
 * FUNCTION
 *   Install default C keywords and comments
 * SOURCE
 */
void Install_C_Syntax(
    void )
{
    unsigned int        i;

    /* Check if we can install our default C keywords */
    if ( !configuration.keywords.number )
    {
        for ( i = 0; c_keywords[i]; i++ )
        {
            AddParameter( c_keywords[i], &( configuration.keywords ) );
        }

        /* Make keywords hash table (if necessarry) */
        add_keywords_to_hash_table(  );
    }

    /* Make sure that C line comment is present */
    if ( Find_Parameter_Exact
         ( &( configuration.source_line_comments ), C_LINE_COMMENT ) == NULL )
    {
        AddParameter( C_LINE_COMMENT,
                      &( configuration.source_line_comments ) );
    }


    /* Make sure that C block comment start is present */
    if ( Find_Parameter_Exact
         ( &( configuration.remark_begin_markers ),
           C_BLOCK_COMMENT_START ) == NULL )
    {
        AddParameter( C_BLOCK_COMMENT_START,
                      &( configuration.remark_begin_markers ) );
    }

    /* Make sure that C block comment end is present */
    if ( Find_Parameter_Exact
         ( &( configuration.remark_end_markers ),
           C_BLOCK_COMMENT_END ) == NULL )
    {
        AddParameter( C_BLOCK_COMMENT_END,
                      &( configuration.remark_end_markers ) );
    }
}

/******/




/* TODO Documentation */
static void ComplementHeaderMarkers(
    void )
{
    unsigned int        i;

    if ( configuration.header_markers.number )
    {
        /* The user specified his own header_markers */
    }
    else
    {
        for ( i = 0; header_markers[i]; ++i )
        {
            AddParameter( header_markers[i],
                          &( configuration.header_markers ) );
        }
    }

    if ( configuration.remark_markers.number )
    {
        /* The user specified his own remark_markers */
    }
    else
    {
        for ( i = 0; remark_markers[i]; ++i )
        {
            AddParameter( remark_markers[i],
                          &( configuration.remark_markers ) );
        }
    }

    if ( configuration.end_markers.number )
    {
        /* The user specified his own end_markers */
    }
    else
    {
        for ( i = 0; end_markers[i]; ++i )
        {
            AddParameter( end_markers[i], &( configuration.end_markers ) );
        }
    }

    if ( configuration.header_separate_chars.number )
    {
        /* The user specified his own header_separate_chars */
    }
    else
    {
        for ( i = 0; default_header_separate_chars[i]; ++i )
        {
            AddParameter( default_header_separate_chars[i],
                          &( configuration.header_separate_chars ) );
        }
    }

    if ( configuration.header_ignore_chars.number )
    {
        /* The user specified his own header_ignore_chars */
    }
    else
    {
        for ( i = 0; default_header_ignore_chars[i]; ++i )
        {
            AddParameter( default_header_ignore_chars[i],
                          &( configuration.header_ignore_chars ) );
        }
    }
}



/****if* Config/ConfigLineKind
 * FUNCTION
 *   Deterimine the kind of line we a currently processing.
 * SYNOPSIS
 */

static T_Line_Kind ConfigLineKind(
    char *line )
/*
 * INPUTS
 *   line -- the current line.
 * RETURN
 *   The kind of line.
 * SOURCE
 */
{
    T_Line_Kind         kind = CFL_UNKNOWN;

    if ( *line == '\0' )
    {
        kind = CFL_EMPTYLINE;
    }
    else if ( *line == '#' )
    {
        kind = CFL_REMARK;
    }
    else if ( utf8_isspace( *line ) )
    {
        char               *cur_char = line;

        for ( ; *cur_char && utf8_isspace( *cur_char ); ++cur_char )
        {
            /* Empty */
        }
        if ( *cur_char == '\0' )
        {
            kind = CFL_EMPTYLINE;
        }
        else
        {
            /* There is atleast one non-space character */
            kind = CFL_PARAMETER;
        }
    }
    else
    {
        kind = CFL_SECTION;
    }
    return kind;
}

/********/


static T_Block_Kind BlockKind(
    char *line )
{
    T_Block_Kind        section_kind = SK_UNKNOWN;

    if ( strcmp( line, "items:" ) == 0 )
    {
        section_kind = SK_ITEMS;
    }
    else if ( strcmp( line, "options:" ) == 0 )
    {
        section_kind = SK_OPTIONS;
    }
    else if ( strcmp( line, "extensions:" ) == 0 )
    {
        printf
            ( "Warning:  the 'extensions:' block is obsolete, use 'ignore files:' instead\n" );
    }
    else if ( strcmp( line, "ignore items:" ) == 0 )
    {
        section_kind = SK_IGNOREITEMS;
    }
    else if ( strcmp( line, "source items:" ) == 0 )
    {
        section_kind = SK_SOURCE_ITEMS;
    }
    else if ( strcmp( line, "headertypes:" ) == 0 )
    {
        section_kind = SK_HEADERTYPES;
    }
    else if ( strcmp( line, "ignore files:" ) == 0 )
    {
        section_kind = SK_IGNORE_FILES;
    }
    else if ( strcmp( line, "accept files:" ) == 0 )
    {
        section_kind = SK_ACCEPT_FILES;
    }
    else if ( strcmp( line, "header markers:" ) == 0 )
    {
        section_kind = SK_HEADER_MARKERS;
    }
    else if ( strcmp( line, "remark markers:" ) == 0 )
    {
        section_kind = SK_REMARK_MARKERS;
    }
    else if ( strcmp( line, "end markers:" ) == 0 )
    {
        section_kind = SK_END_MARKERS;
    }
    else if ( strcmp( line, "remark begin markers:" ) == 0 )
    {
        section_kind = SK_REMARK_BEGIN_MARKERS;
    }
    else if ( strcmp( line, "remark end markers:" ) == 0 )
    {
        section_kind = SK_REMARK_END_MARKERS;
    }
    else if ( strcmp( line, "keywords:" ) == 0 )
    {
        section_kind = SK_KEYWORDS;
    }
    else if ( strcmp( line, "source line comments:" ) == 0 )
    {
        section_kind = SK_SOURCE_LINE_COMMENTS;
    }
    else if ( strcmp( line, "header ignore characters:" ) == 0 )
    {
        section_kind = SK_HEADER_IGNORE_CHARS;
    }
    else if ( strcmp( line, "header separate characters:" ) == 0 )
    {
        section_kind = SK_HEADER_SEPARATE_CHARS;
    }
    else if ( strcmp( line, "preformatted items:" ) == 0 )
    {
        section_kind = SK_PREFORMATTED_ITEMS;
    }
    else if ( strcmp( line, "format items:" ) == 0 )
    {
        section_kind = SK_FORMAT_ITEMS;
    }
    else if ( strcmp( line, "item order:" ) == 0 )
    {
        section_kind = SK_ITEM_ORDER;
    }
    else
    {
        RB_Panic( "unknown block kind \"%s\"\n", line );
    }
    return section_kind;
}


static void Install_Custom_HeaderTypes(
    void )
{
    unsigned int        i;
    struct Parameters   headertype;
    unsigned int        priority = 0;

    /* Install custom header types */
    for ( i = 0; i < configuration.custom_headertypes.number; i++ )
    {
        /* Allocate some default space for parameters */
        Alloc_Parameters( &headertype, 10 );
        /* Break current line into parameters */
        GetParameters( configuration.custom_headertypes.names[i],
                       &headertype );

        /* Check how many parameters do we have */
        switch ( headertype.number )
        {
            /* 3 parameters -> no priority specified, assign default */
        case 3:
            priority = 0;
            break;

            /* 4 parameters -> priority specified, convert it */
        case 4:
            priority = atoi( headertype.names[3] );
            break;

            /* Any more or less parameters are illegal */
        default:
            RB_Panic( "Error near header type: '%s'\n"
                      "You must have either 3 or 4 parameters there !\n",
                      headertype.names[0] );
        }

        /* Check if type character is legal */
        if ( strlen( headertype.names[0] ) > 1 )
        {
            RB_Panic( "Error near header type: '%s'\n"
                      "Type character can only be one character long !\n",
                      headertype.names[0] );
        }

        /* Add custom header type */
        RB_AddHeaderType( headertype.names[0][0], headertype.names[1],
                          headertype.names[2], priority );

        /* Free temporary space */
        free( headertype.names );
    }
}



static void Alloc_Parameters(
    struct Parameters *parameters,
    unsigned int size )
{
    parameters->size = size;
    parameters->number = 0;
    parameters->names = calloc( size, sizeof( char * ) );
}


/* TODO Documentation */
static void AddParameter(
    char *name,
    struct Parameters *parameters )
{
    parameters->names[parameters->number] = RB_StrDup( name );
    parameters->number++;
    if ( parameters->number >= parameters->size )
    {
        parameters->size *= 2;
        parameters->names =
            realloc( parameters->names, parameters->size * sizeof( char * ) );
    }
}


/****f* Configuration/GetParameters
 * FUNCTION
 *   Parse a line of text and store the individual words in
 *   a Parameters structure.  Words are seperated by spaces,
 *   the exception are words surrounded by quotes. So:
 *      aap noot mies "back to the future"
 *   contains four words.
 * INPUTS
 *   o line -- the line of text.
 *   o parameters  -- the set of parameters
 * SOURCE
 */

static void GetParameters(
    char *line,
    struct Parameters *parameters )
{
    int                 i;
    int                 n = strlen( line );

    /* Remove any spaces at the end of the line */
    for ( i = n - 1; i >= 0 && utf8_isspace( line[i] ); --i )
    {
        line[i] = '\0';
    }

    assert( i > 0 );            /* If i <= 0 then the line was empty
                                   and that cannot be, because this
                                   is supposed to be a parameter */

    /* Skip any white space at the begin of the line. */
    n = strlen( line );
    for ( i = 0; i < n && utf8_isspace( line[i] ); ++i )
    {
        /* Empty */
    }
    line += i;

    n = strlen( line );
    for ( i = 0; i < n; /* empty */  )
    {
        char               *name = line;

        if ( line[i] == '"' )
        {
            /* It is quoted string, fetch everything until
             * the next quote */
            ++name;             /* skip the double quote */
            for ( ++i; ( i < n ) && ( line[i] != '"' ); ++i )
            {
                /* empty */
            }
            if ( i == n )
            {
                RB_Panic( "Missing quote in your .rc file in line:\n  %s\n",
                          line );
            }
            else
            {
#if defined(__APPLE__)
                /* hacked because of error when compiling on Mac OS X */
                assert( line[i] == 34 );
#else
                assert( line[i] == '"' );
#endif
                line[i] = '\0';
                AddParameter( name, parameters );
            }
        }
        else
        {
            /* a single word, find the next space */
            for ( ; ( i < n ) && !utf8_isspace( line[i] ); ++i )
            {
                /* empty */
            }
            if ( i < n )
            {
                line[i] = '\0';
            }
            AddParameter( name, parameters );
        }
        /* Is there anything left? */
        if ( i < n )
        {
            /* skip any spaces until the next parameter */
            ++i;                /* first skip the nul character */
            line += i;
            n = strlen( line );
            for ( i = 0; ( i < n ) && utf8_isspace( line[i] ); ++i )
            {
                /* empty */
            }
            line += i;
            n = strlen( line );
            i = 0;
        }
    }
}

/*******/

void GetParameter(
    char *line,
    struct Parameters *parameters )
{
    int                 i;
    int                 n = strlen( line );

    /* Remove any spaces at the end of the line */
    for ( i = n - 1; i >= 0 && utf8_isspace( line[i] ); --i )
    {
        line[i] = '\0';
    }
    assert( i > 0 );            /* If i <= 0 then the line was empty
                                   and that cannot be, because this
                                   is supposed to be a parameter */
    /* Skip any white space at the begin of the line. */
    n = strlen( line );
    for ( i = 0; i < n && utf8_isspace( line[i] ); ++i )
    {
        /* Empty */
    }
    line += i;

    AddParameter( line, parameters );
}




void Free_Configuration(
    void )
{
    /* TODO  Deallocate custom_headertypes */
}




static void ComplementItemNames(
    void )
{
    if ( configuration.items.number )
    {
        char               *first_item = configuration.items.names[0];

        /* The SOURCE item is always included, and should be the
         * first one! */
        configuration.items.names[0] = RB_StrDup( "SOURCE" );
        AddParameter( first_item, &( configuration.items ) );
        free( first_item );
    }
    else
    {
        /* No item names were defined, so we use the default ones */
        unsigned int        i = 0;

        for ( ; default_item_names[i]; ++i )
        {
            AddParameter( default_item_names[i], &( configuration.items ) );
        }
    }
}


/* TODO Documentation */


static void SecondScan(
    FILE *f )
{
    char               *myConfLine = NULL;
    int                 readConfChars = 0;
    T_Block_Kind        section_kind = SK_UNKNOWN;
    T_Line_Kind         line_kind = CFL_UNKNOWN;

    while ( !feof( f ) )
    {
        free( myConfLine );
        readConfChars = 0;
        myConfLine = RB_ReadWholeLine( f, line_buffer, &readConfChars );
        RB_StripCR( myConfLine );
        line_kind = ConfigLineKind( myConfLine );
        switch ( line_kind )
        {
        case CFL_REMARK:
        case CFL_EMPTYLINE:    /* fall through */
            /* Do nothing */
            break;
        case CFL_SECTION:
            section_kind = BlockKind( myConfLine );
            break;
        case CFL_PARAMETER:
            {
                switch ( section_kind )
                {
                case SK_ITEMS:
                    GetParameter( myConfLine, &( configuration.items ) );
                    break;
                case SK_OPTIONS:
                    GetParameters( myConfLine, &( configuration.options ) );
                    break;
                case SK_IGNOREITEMS:
                    GetParameter( myConfLine,
                                  &( configuration.ignore_items ) );
                    break;
                case SK_SOURCE_ITEMS:
                    GetParameter( myConfLine,
                                  &( configuration.source_items ) );
                    break;
                case SK_HEADERTYPES:
                    /* Store all complete lines, they will be broken down later 
                     * in Install_Custom_HeaderTypes() */
                    GetParameter( myConfLine,
                                  &( configuration.custom_headertypes ) );
                    break;
                case SK_IGNORE_FILES:
                    GetParameters( myConfLine,
                                   &( configuration.ignore_files ) );
                    break;
                case SK_ACCEPT_FILES:
                    GetParameters( myConfLine,
                                   &( configuration.accept_files ) );
                    break;
                case SK_HEADER_MARKERS:
                    GetParameter( myConfLine,
                                  &( configuration.header_markers ) );
                    break;
                case SK_REMARK_MARKERS:
                    GetParameter( myConfLine,
                                  &( configuration.remark_markers ) );
                    break;
                case SK_END_MARKERS:
                    GetParameter( myConfLine,
                                  &( configuration.end_markers ) );
                    break;
                case SK_REMARK_END_MARKERS:
                    GetParameter( myConfLine,
                                  &( configuration.remark_end_markers ) );
                    break;
                case SK_REMARK_BEGIN_MARKERS:
                    GetParameter( myConfLine,
                                  &( configuration.remark_begin_markers ) );
                    break;
                case SK_KEYWORDS:
                    GetParameter( myConfLine, &( configuration.keywords ) );
                    break;
                case SK_SOURCE_LINE_COMMENTS:
                    GetParameter( myConfLine,
                                  &( configuration.source_line_comments ) );
                    break;
                case SK_HEADER_IGNORE_CHARS:
                    GetParameter( myConfLine,
                                  &( configuration.header_ignore_chars ) );
                    break;
                case SK_HEADER_SEPARATE_CHARS:
                    GetParameter( myConfLine,
                                  &( configuration.header_separate_chars ) );
                    break;
                case SK_PREFORMATTED_ITEMS:
                    GetParameter( myConfLine,
                                  &( configuration.preformatted_items ) );
                    break;
                case SK_FORMAT_ITEMS:
                    GetParameter( myConfLine,
                                  &( configuration.format_items ) );
                    break;
                case SK_ITEM_ORDER:
                    GetParameter( myConfLine,
                                  &( configuration.item_order ) );
                    break;
                case SK_UNKNOWN:
                    break;
                default:
                    assert( 0 );
                }
            }
            break;
        case CFL_UNKNOWN:
        default:
            assert( 0 );
        }
    }
    free( myConfLine );
}
