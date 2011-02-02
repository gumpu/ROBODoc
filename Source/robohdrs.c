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

/* $Id: robohdrs.c,v 1.18 2008/03/12 20:52:48 gumpu Exp $ */

/****h* Docuwala/ROBOhdrs
*  NAME
*    robohdrs
*  DESCRIPTION
*    Standalone program to insert ROBODoc headers to source code files.
*    This program processes one source file at the time. Existing 
*    ROBODoc headers, if any, are not checked for. Beware since this 
*    may result in double headers. Current working directory should 
*    be the same as where the source file is located.
*  USES
*    Exuberant Ctags 5.3.1 or newer required
*  USAGE
*    robohdrs [options] <source file>
*  EXAMPLE
*    robohdrs -p myproj test1.c
*    robohdrs -s -p myproj -i "MODIFICATION HISTORY" -i IDEAS test2.c
*
*    Type `robohdrs -h' to see all command line options.
*  TODO
*    - garbage collection
*    - support for other languages which ctags program supports
*  SEE ALSO
*    ROBODoc         https://sourceforge.net/projects/robodoc/
*    Exuberant Ctags http://ctags.sourceforge.net/
*  COPYRIGHT
*    (c) 2003 Frans Slothouber and Petteri Kettunen
*    Copying policy: GPL
*  HISTORY
*    2003-08-08/petterik: #define `d' header (bug rep. from Anand Dhanakshirur)
*    2003-08-04/petterik: -t option (suggestion from Anand Dhanakshirur)
*    2003-02-21/petterik: -l option, script option tested
*******
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#ifndef HAVE_FORK

// Tell the user that this program is only a stub
#warning robohdrs will be compiled as a stub on this platform.
#warning It will not work as excepted. It needs fork() to operate correctly.

// Macro to get rid of some compiler warnings
#define USE( x ) ( x = x );

// Stub
int main( int argc, char** argv )
{
    USE( argc );

    printf( "%s: Not supported on this platform. This program requires fork()."
            "\n", argv[0] );
    return EXIT_SUCCESS;
}

#else



#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include "headers.h"

/****v* ROBOhdrs/PROGNAME
*  NAME
*    PROGNAME
*  SOURCE
*/
#define PROGNAME "robohdrs"
/********** PROGNAME */
/****v* ROBOhdrs/PROGVERSION
*  NAME
*    PROGVERSION
*  SOURCE
*/
#define PROGVERSION "0.01"
/********** PROGVERSION */
/****v* ROBOhdrs/MAXLINE
*  NAME
*    MAXLINE
*  SOURCE
*/
#define MAXLINE 10240
/********** MAXLINE */
/****v* ROBOhdrs/MAXNAME
*  NAME
*    MAXNAME
*  SOURCE
*/
#define MAXNAME 1024

/********** MAXNAME */

/****v* ROBOhdrs/ctagsBin
*  NAME
*    ctagsBin
*  SOURCE
*/
static char         ctagsBin[MAXNAME];

/********** ctagsBin */

/****v* ROBOhdrs/srcSta
*  NAME
*    srcSta
*  SEE ALSO
*    src_constants
*  SOURCE
*/

static short        srcSta = SRC_C;
static short        srcRem = SRC_R_C;
static short        srcEnd = SRC_E_C;

/********** srcSta */

typedef struct _ctag_t
{
    void               *prev;
    void               *next;
    char                fname[MAXNAME];
    char                name[MAXNAME];
    char                decl[MAXLINE];
    char                type[MAXNAME];
    int                 linenum;
}
ctag_t;

typedef struct _custhdr_t
{
    void               *next;
    char                name[MAXNAME];
}
custhdr_t;

typedef struct _ctags_t
{
    ctag_t             *ctag;
    int                 cnt;
}
ctags_t;

/****v* ROBOhdrs/myctags
*  NAME
*    myctags
*  SYNOPSIS
*    static ctags_t myctags;
*  SOURCE
*/
static ctags_t      myctags;

/********** myctags */
/****v* ROBOhdrs/ctags
*  NAME
*    ctags
*  SYNOPSIS
*    static ctags_t *ctags;
*  SOURCE
*/
static ctags_t     *ctags;

/********** ctags */
/****v* ROBOhdrs/custhdrs
*  NAME
*    custhdrs
*  SYNOPSIS
*    static custhdr_t *custhdrs = 0;
*  SOURCE
*/
static custhdr_t   *custhdrs = 0;

/********** custhdrs */
/****v* ROBOhdrs/projName
*  NAME
*    projName
*  SYNOPSIS
*    static char projName[MAXNAME];
*  SOURCE
*/
static char         projName[MAXNAME];

/********** projName */
/****v* ROBOhdrs/vcTag
*  NAME
*    vcTag
*  DESCRIPTION
*    Version control tag string. This is always specified by the user.
*  SYNOPSIS
*    static char vcTag[MAXNAME];
*  SOURCE
*/
static char         vcTag[MAXNAME];

/********** projName */
/****v* ROBOhdrs/incSrc
*  NAME
*    incSrc
*  SYNOPSIS
*    static char incSrc = 0;
*  SOURCE
*/
static char         incSrc = 0;

/********** incSrc */
/****f* ROBOhdrs/usage
*  NAME
*    usage
*  SYNOPSIS
*    static void usage(void)
*  SOURCE
*/
static void
usage( void )
{
    printf( "%s version %s, robodoc header insertor\n", PROGNAME,
            PROGVERSION );
    printf( "(c) 2003 Frans Slothouber and Petteri Kettunen\n" );
    printf( "%s comes with ABSOLUTELY NO WARRANTY.\n", PROGNAME );
    printf
        ( "This is free software, and you are welcome to redistribute it\n" );
    printf( "under the GNU GENERAL PUBLIC LICENSE terms and conditions.\n" );
    printf( "usage: %s [options] <source file>\n", PROGNAME );
    printf( "Options are as follows:\n" );
    printf( "  -h   show this help text\n" );
    printf
        ( "  -i   specify header item (repeat to include multiple items)\n" );
    printf( "  -l   specify source code language (default C/C++)\n" );
    printf
        ( "       Supported options are: fortran, fortran90, script, and tex.\n" );
    printf( "  -p   specify project name\n" );
    printf( "  -s   include SOURCE item\n" );
    printf( "  -t   specify CVS/RCS tag to be inserted into a file\n" );
    printf( "  -x   specify path to ctags binary\n" );
    printf( "NOTE: requires Exuberant Ctags 5.3.1 (or newer)\n" );
    printf( "EXAMPLES:\n" );
    printf
        ( "robohdrs -s -p myproj -i \"MODIFICATION HISTORY\" -i IDEAS test.c\n" );
    printf
        ( "robohdrs -s -p myproj -l script -t '%cHeader:%c' test.tcl\n", '$', '$' );
    exit( 1 );
}

/********** usage */

/****f* ROBOhdrs/cmdLine
*  NAME
*    cmdLine
*  SYNOPSIS
*    static void cmdLine(int argc, char **argv)
*  SOURCE
*/
static void
cmdLine( int argc, char **argv )
{
    int                 ch;
    custhdr_t          *c, *nc;

    while ( ( ch = getopt( argc, argv, "i:l:p:st:x:" ) ) != -1 )
        switch ( ch )
        {

        case 's':
            /* include source item */
            incSrc = 1;
            break;

        case 't':
            /* specify version control tag */
            strncpy( vcTag, optarg, MAXNAME );
            break;

        case 'p':
            /* specify project name */
            strncpy( projName, optarg, MAXNAME );
            break;

        case 'i':
            if ( !custhdrs )
            {
                assert( ( custhdrs =
                          ( custhdr_t * ) malloc( sizeof( custhdr_t ) ) ) );
                custhdrs->next = 0;
                c = custhdrs;
            }
            else
            {
                c = custhdrs;
                while ( c->next )
                {
                    c = c->next;
                }
                assert( ( nc =
                          ( custhdr_t * ) malloc( sizeof( custhdr_t ) ) ) );
                nc->next = 0;
                c = c->next = nc;
            }
            strncpy( c->name, optarg, MAXNAME );
            break;

        case 'l':
            if ( optarg[0] == 's' && strcmp( optarg, "script" ) == 0 )
            {
                srcSta = SRC_SCRIPT;
                srcRem = SRC_R_SCRIPT;
                srcEnd = SRC_E_SCRIPT;
            }
            else if ( optarg[0] == 'f' && strcmp( optarg, "fortran" ) == 0 )
            {
                srcSta = SRC_FORTRAN;
                srcRem = SRC_R_FORTRAN;
                srcEnd = SRC_E_FORTRAN;
            }
            else if ( optarg[0] == 'f' && strcmp( optarg, "fortran90" ) == 0 )
            {
                srcSta = SRC_F90;
                srcRem = SRC_R_F90;
                srcEnd = SRC_E_F90;
            }
            else if ( optarg[0] == 't' && strcmp( optarg, "tex" ) == 0 )
            {
                srcSta = SRC_TEX;
                srcRem = SRC_R_TEX;
                srcEnd = SRC_E_TEX;
            }
            else if ( optarg[0] == 'a' && strcmp( optarg, "acm" ) == 0 )
            {
                srcSta = SRC_ACM;
                srcRem = SRC_R_ACM;
                srcEnd = SRC_E_ACM;
            }
            else
            {
                usage(  );
            }
            break;

        case 'x':
            strncpy( ctagsBin, optarg, MAXNAME );
            break;

        case '?':
        case 'h':
        default:
            usage(  );
            break;
        }
}

/********** cmdLine */
/****f* ROBOhdrs/linenumCompare
*  NAME
*    linenumCompare
*  SYNOPSIS
*    static int linenumCompare(void const * a, void const * b)
*  SOURCE
*/
static int
linenumCompare( void const *a, void const *b )
{
    ctag_t             *ea = ( ctag_t * ) a, *eb = ( ctag_t * ) b;

    return ( ea->linenum - eb->linenum );
}

/********** linenumCompare */
/****f* ROBOhdrs/arrangeCtags
*  NAME
*    arrangeCtags
*  SYNOPSIS
*    static void arrangeCtags(ctags_t *e)
*  SOURCE
*/
static void
arrangeCtags( ctags_t * e )
{
    ctag_t             *tmp, *ctag = e->ctag, *ep;

    assert( e && e->cnt && e->ctag );
    tmp = ( ctag_t * ) malloc( e->cnt * sizeof( ctag_t ) );
    assert( tmp );

    for ( ep = tmp;; )
    {
        memcpy( ep++, ctag, sizeof( ctag_t ) );
        if ( ctag->next )
        {
            ctag = ctag->next;
        }
        else
        {
            break;
        }
    }

    qsort( tmp, ( size_t ) ( e->cnt ), sizeof( ctag_t ), linenumCompare );
    /* TODO: free ctag */
    e->ctag = tmp;
}

/********** arrangeCtags */
/****f* ROBOhdrs/typeOk
*  NAME
*    typeOk
*  SYNOPSIS
*    static int typeOk(char *t)
*  SOURCE
*/
static int
typeOk( char *t )
{
    /* return 1 if supported type for headers */
    if ( t[0] == 'f' && strncmp( t, "function", 8 ) == 0 )
    {
        return 1;
    }
    else if ( t[0] == 'v' && strncmp( t, "variable", 8 ) == 0 )
    {
        return 1;               /* global variable */
    }
#if 0
    else if ( t[0] == 's' && strncmp( t, "struct", 6 ) == 0 )
    {
        return 1;
    }
#endif
    else if ( t[0] == 'm' && strncmp( t, "macro", 5 ) == 0 )
    {
        return 1;
    }
    return 0;
}


/********** typeOk */
/****f* ROBOhdrs/initMe
*  NAME
*    initMe
*  SYNOPSIS
*    static void initMe(void)
*  SOURCE
*/
static void
initMe( void )
{
    ctags = &myctags;
    memset( ctags, 0, sizeof( ctags_t ) );
    projName[0] = '\0';
    ctagsBin[0] = '\0';
    vcTag[0] = '\0';
}

/********** initMe */

/****f* ROBOhdrs/parseCtagsXLine
*  NAME
*    parseCtagsXLine
*  SYNOPSIS
*    static int parseCtagsXLine(char *buf, char *fname, char *name, char *decl, char *type, int *linenum)
*  SOURCE
*/
static int
parseCtagsXLine( char *buf, char *fname, char *name, char *decl, char *type,
                 int *linenum )
{
    char               *t, *s;

    /* ctags -x output is: */
    /* usage            function     56 test.c           void usage(void) */
    sscanf( buf, "%s%s%d%s", name, type, linenum, fname );
    s = strstr( buf, fname );
    while ( *s++ != ' ' )
    {
    }
    while ( *s == ' ' )
    {
        ++s;
    }
    t = decl;
    while ( ( *t = *s++ ) != '\n' )
    {
        ++t;
    }
    *t = '\0';

    return 0;
}

/********** parseCtagsXLine */

/****f* ROBOhdrs/addList
*  NAME
*    addList
*  SYNOPSIS
*    static void addList(ctags_t *e, char *fname, char *name, char *decl, char *type, int linenum)
*  SOURCE
*/
static void
addList( ctags_t * e, char *fname, char *name, char *decl, char *type,
         int linenum )
{
    ctag_t             *newctag, *ctag = e->ctag;

    if ( !ctag )
    {
        /* empty list */
        ctag = ( ctag_t * ) malloc( sizeof( ctag_t ) );
        assert( ctag );
        memset( ctag, 0, sizeof( ctag_t ) );
        e->ctag = ctag;
    }
    else
    {
        while ( ctag->next )
        {
            ctag = ctag->next;
        }
        newctag = ( ctag_t * ) malloc( sizeof( ctag_t ) );
        assert( newctag );
        memset( newctag, 0, sizeof( ctag_t ) );
        ctag->next = newctag;
        newctag->prev = ctag;
        ctag = newctag;
    }

    e->cnt++;

    strncpy( ctag->fname, fname, MAXNAME );
    strncpy( ctag->name, name, MAXNAME );
    strncpy( ctag->decl, decl, MAXLINE );
    strncpy( ctag->type, type, MAXNAME );
    ctag->linenum = linenum;
}

/********** addList */
/****f* ROBOhdrs/parseCtagsX
*  NAME
*    parseCtagsX
*  SYNOPSIS
*    static int parseCtagsX(FILE *fp)
*  SOURCE
*/
static int
parseCtagsX( FILE * fp )
{
    char                buf[MAXLINE + 1];
    int                 lnum = 0, tagsParsed = 0;

    while ( fgets( buf, MAXLINE, fp ) != NULL )
    {
        char                decl[MAXNAME + 1], name[MAXNAME + 1];
        char                fname[MAXNAME + 1], type[MAXNAME + 1];
        int                 linenum;

        lnum++;
        /* extract info from a line */
        if ( parseCtagsXLine( buf, fname, name, decl, type, &linenum ) )
        {
            printf( "error parsing line (%d)", lnum );
        }
        else
        {
            addList( ctags, fname, name, decl, type, linenum );
            tagsParsed++;
        }
    }                           /* end while() */
    fclose( fp );

    return tagsParsed;
}

/********** parseCtagsX */

/****f* ROBOhdrs/roboFileHeader
*  NAME
*    roboFileHeader
*  FUNCTION
*    Insert source file header.
*  SYNOPSIS
*    static void roboFileHeader(FILE *fp, char *proj, char *fname)
*  SOURCE
*/
static void
roboFileHeader( FILE * fp, char *proj, char *fname )
{
    char               *s;

    s = remark_markers[srcRem];
    fprintf( fp, "%sh* %s/%s\n", header_markers[srcSta],
             ( proj[0] ? proj : fname ), fname );
    fprintf( fp, "%s  NAME\n", s );
    fprintf( fp, "%s    %s\n", s, fname );
    if ( *vcTag )
    {
        fprintf( fp, "%s    %s\n", s, vcTag );
    }
    fprintf( fp, "%s  DESCRIPTION\n", s );
    fprintf( fp, "%s*******%s\n", s,
             ( end_remark_markers[srcEnd] ? end_remark_markers[srcEnd] :
               "" ) );
}

/********** roboFileHeader */

/****f* ROBOhdrs/roboHeader
*  NAME
*    roboHeader
*  SYNOPSIS
*    static void roboHeader(FILE *fp, char *fname, char *name, char *type, char *decl)
*  SOURCE
*/
static void
roboHeader( FILE * fp, char *fname, char *name, char *type, char *decl )
{
    custhdr_t          *c;
    char               *s;

    s = remark_markers[srcRem];
    if ( type[0] == 'v' )
    {
        fprintf( fp, "%sv* %s/%s\n", header_markers[srcSta], fname, name );
    }
    else if ( type[0] == 'm' )
    {
        fprintf( fp, "%sd* %s/%s\n", header_markers[srcSta], fname, name );
    }
#if 0
    else if ( type[0] == 's' )
    {
        fprintf( fp, "/****s* %s/%s\n", fname, name );
    }
#endif
    else
    {
        fprintf( fp, "%sf* %s/%s\n", header_markers[srcSta], fname, name );
    }

    fprintf( fp, "%s  NAME\n%s    %s\n", s, s, name );
    if ( type[0] != 'm' )
    {
        fprintf( fp, "%s  SYNOPSIS\n%s    %s\n", s, s, decl );
    }
    if ( custhdrs )
    {
        for ( c = custhdrs;; )
        {
            fprintf( fp, "%s  %s\n", s, c->name );
            if ( c->next )
            {
                c = c->next;
            }
            else
            {
                break;
            }
        }
    }

    if ( incSrc )
    {
        fprintf( fp, "%s  SOURCE\n%s\n", s,
                 ( end_remark_markers[srcSta] ? end_remark_markers[srcSta] :
                   s ) );
    }
    else
    {
        fprintf( fp, "%s***%s\n", s,
                 ( end_remark_markers[srcSta] ? end_remark_markers[srcSta] :
                   "" ) );
    }
}

/********** roboHeader */

/****f* ROBOhdrs/insertSrcEnd
*  NAME
*    insertSrcEnd
*  SYNOPSIS
*    static void insertSrcEnd(FILE *fp, char *funcname)
*  SOURCE
*/

static void
insertSrcEnd( FILE * fp, char *funcname )
{
    fprintf( fp, "%s********* %s %s\n", end_markers[srcEnd], funcname,
             ( end_remark_markers[srcSta] ? end_remark_markers[srcSta] :
               "" ) );
}

/********** insertSrcEnd */

/****f* ROBOhdrs/insertHeaders
*  NAME
*    insertHeaders
*  SYNOPSIS
*    static void insertHeaders(ctags_t *e, char *project, char *dstpath, char *srcpath)
*  SOURCE
*/
static void
insertHeaders( ctags_t * e, char *project, char *dstpath, char *srcpath )
{
    FILE               *ifp, *ofp;
    ctag_t             *ctag = e->ctag;
    int                 lnum = 0, funcline = 0;
    char                buf[MAXLINE], *funcname = 0;

    if ( !ctag || !dstpath || !srcpath )
    {
        return;
    }

    assert( ofp = fopen( dstpath, "w" ) );
    assert( ifp = fopen( srcpath, "r" ) );

    /* include file header only if project name is defined */
    if ( project )
    {
        roboFileHeader( ofp, project, dstpath );
    }

    while ( fgets( buf, MAXLINE, ifp ) != NULL )
    {
        lnum++;
        while ( ctag->prev )
        {
            ctag = ctag->prev;
        }
        for ( ;; )
        {
            if ( incSrc && funcline && lnum >= funcline
                 && ctag->linenum == lnum )
            {
                funcline = 0;
                insertSrcEnd( ofp, funcname );
            }
            if ( ctag->linenum == lnum )
            {
                if ( typeOk( ctag->type ) )
                {
                    roboHeader( ofp, ctag->fname, ctag->name, ctag->type,
                                ctag->decl );
                    funcline = lnum;
                    funcname = ctag->name;
                }
                break;
            }
            else if ( ctag->next )
            {
                ctag = ctag->next;
            }
            else
            {
                break;
            }
        }                       /* end ctag loop */
        fprintf( ofp, "%s", buf );
    }

    if ( incSrc && funcline )
    {
        insertSrcEnd( ofp, funcname );
    }

    fclose( ifp );
    fclose( ofp );
}

/********** insertHeaders */
/****f* ROBOhdrs/doCtagsExec
*  NAME
*    doCtagsExec
*  SYNOPSIS
*    static FILE * doCtagsExec(char *fname)
*  SOURCE
*/
static FILE        *
doCtagsExec( char *fname )
{
    int                 fd[2], pid;
    FILE               *incoming = NULL;
    char               *mybin, *bin = "ctags";

    mybin = ( ctagsBin[0] ? ctagsBin : bin );

    if ( pipe( fd ) == -1 )
    {
        fprintf( stderr, "pipe failed\n" );
        exit( 1 );
    }

    if ( ( pid = fork(  ) ) == 0 )
    {
        close( 1 );
        dup( fd[1] );
        close( fd[0] );
        if ( execlp( mybin, mybin, "-x", fname, NULL ) == -1 )
        {
            fprintf( stderr, "execlp failed\n" );
            exit( 1 );
        }
    }
    else if ( pid == -1 )
    {
        fprintf( stderr, "fork failed\n" );
        exit( 1 );
    }
    else
    {
        close( 0 );
        dup( fd[0] );
        close( fd[1] );

        if ( ( incoming = fdopen( 0, "r" ) ) == NULL )
        {
            fprintf( stderr, "fdopen failed\n" );
            exit( 1 );
        }
    }

    return incoming;
}

/********** doCtagsExec */
/****f* ROBOhdrs/doFile
*  NAME
*    doFile
*  SYNOPSIS
*    static void doFile(char *proj, char *fname)
*  SOURCE
*/
static void
doFile( char *proj, char *fname )
{
    char                buf[MAXLINE];

    /* backup */
    sprintf( buf, "/bin/cp -p %s %s~", fname, fname );
    system( buf );

    if ( parseCtagsX( doCtagsExec( fname ) ) < 1 )
    {
        fprintf( stderr, "no tags\n" );
        exit( 1 );
    }

    arrangeCtags( ctags );
    sprintf( buf, "%s~", fname );
    insertHeaders( ctags, proj, fname, buf );
}

/********** doFile */
/****f* ROBOhdrs/cleanUp
*  NAME
*    cleanUp
*  SYNOPSIS
*    static void cleanUp(void)
*  SOURCE
*/
static void
cleanUp( void )
{

}

/********** cleanUp */


/****f* ROBOhdrs/main
*  NAME
*    main
*  SYNOPSIS
*    int main(int argc, char **argv)
*  SOURCE
*/
int
main( int argc, char **argv )
{
    initMe(  );
    cmdLine( argc, argv );
    argc -= optind;
    argv += optind;
    if ( argc == 1 )
    {
        doFile( projName, argv[0] );
    }
    else
    {
        usage(  );
    }
    cleanUp(  );
    return 0;
}

/********** main */


#endif /*  HAVE_FORK  */

