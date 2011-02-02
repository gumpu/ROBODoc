/* vi: ff=unix spell
 *
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

/****h* ROBODoc/Utilities
 * FUNCTION
 *   Set of general purpose utility functions that are used
 *   in more than one module.
 *****
 * $Id: util.c,v 1.63 2008/06/17 11:49:28 gumpu Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>             /* for RB_Say() */
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

#include "robodoc.h"
#include "globals.h"
#include "links.h"
#include "headers.h"
#include "path.h"
#include "util.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


static void         RB_Swap(
    void **array,
    int left,
    int right );


/*===============================================================================*/


/****f* Utilities/ExpandTab
 * FUNCTION
 *   Expand the tabs in a line of text.
 * SYNOPSIS
 */

char               *ExpandTab(
    char *line )
/*
 * INPUTS
 *   line -- the line to be expanded
 *   tab_size    -- global.
 * RETURN
 *   pointer to the expanded line.
 * NOTE
 *   This function is not reentrant.
 * SOURCE
 */
{
    char               *cur_char = line;
    int                 n = 0;
    int                 jump = 0;
    char               *newLine = NULL;
    int                 lineBufLen = 1;
    int                 actual_tab = 0;

    lineBufLen = strlen( line ) + 1;

    if ( ( newLine = malloc( lineBufLen * sizeof( char ) ) ) == NULL )
    {
        RB_Panic( "Out of memory! ExpandTab()\n" );
    }

    for ( ; *cur_char; ++cur_char )
    {
        if ( *cur_char == '\t' )
        {
            int                 i;

            /* Seek to actual tab stop position in tabstop table */
            while ( ( tab_stops[actual_tab] <= n )
                    && ( actual_tab < ( MAX_TABS - 1 ) ) )
            {
                actual_tab++;
            }

            jump = tab_stops[actual_tab] - n;

            /* If jump gets somehow negative fix it... */
            if ( jump < 0 )
            {
                jump = 1;
            }

            lineBufLen += jump;
            if ( ( newLine = realloc( newLine, sizeof( char ) * lineBufLen ) )
                 == NULL )
            {
                RB_Panic( "Out of memory! ExpandTab()\n" );
            }
            for ( i = 0; i < jump; i++ )
            {
                newLine[n] = ' ';
                ++n;
            }
        }
        else
        {
            newLine[n] = *cur_char;
            ++n;
        }
    }
    newLine[n] = '\0';

    return newLine;
}

/******/


/****f* Utilities/RB_Alloc_Header
 * FUNCTION
 *   allocate the struct RB_header
 * SYNOPSIS
 */

struct RB_header   *RB_Alloc_Header(
    void )
/*
 * RESULT
 *   struct RB_header *      -- all attributes/pointers set to zero
 * AUTHOR
 *   Koessi
 * SEE ALSO
 *   RB_Free_Header()
 * SOURCE
 */
{
    struct RB_header   *new_header;

    if ( ( new_header = malloc( sizeof( struct RB_header ) ) ) != NULL )
    {
        memset( new_header, 0, sizeof( struct RB_header ) );
    }
    else
    {
        RB_Panic( "out of memory! [Alloc Header]\n" );
    }
    return ( new_header );
}

/********/


/****f* Utilities/RB_Free_Header
 * NAME
 *   RB_Free_Header             -- oop
 * SYNOPSIS
 */
void RB_Free_Header(
    struct RB_header *header )
/*
 * FUNCTION
 *   free struct RB_header and associated strings
 * INPUTS
 *   struct RB_header *header -- this one
 * AUTHOR
 *   Koessi
 * SEE ALSO
 *   RB_Alloc_Header(), RB_Close_The_Shop()
 * SOURCE
 */
{
    if ( header )
    {
        if ( header->function_name )
        {
            free( header->function_name );
        }
        if ( header->version )
        {
            free( header->version );
        }
        if ( header->name )
        {
            free( header->name );
        }
        if ( header->unique_name )
        {
            free( header->unique_name );
        }
        if ( header->lines )
        {
            int                 i;

            for ( i = 0; i < header->no_lines; ++i )
            {
                free( header->lines[i].line );
            }
            free( header->lines );
        }
        free( header );
    }
}

/************/


/****if* Utilities/RB_StrDup
 * NAME
 *   RB_StrDup
 * SYNOPSIS
 */

char               *RB_StrDup(
    char *str )
/*
 * FUNCTION
 *   duplicate the given string.
 * INPUTS
 *   char *str               -- source
 * RESULT
 *   char *                  -- destination
 * AUTHOR
 *   Koessi
 * SOURCE
 */
{
    char               *dupstr;
    if ( ( dupstr =
           malloc( ( strlen( str ) + 1 ) * sizeof( char ) ) ) != NULL )
    {
        strcpy( dupstr, str );
    }
    else
    {
        RB_Panic( "out of memory! [StrDup]\n" );
    }
    return ( dupstr );
}


char               *RB_StrDupLen(
    char *str,
    size_t length )
{
    char               *new = malloc( length + 1 );

    if ( new )
    {
        memcpy( new, str, length );
        new[length] = 0;
    }
    else
    {
        RB_Panic( "out of memory! [StrDupLen]\n" );
    }
    return new;
}


/*** RB_StrDup ***/

/****f* Utilities/RB_Say [2.01]
 * NAME
 *   RB_Say                     -- varargs
 * SYNOPSIS
 */
void RB_Say(
    char *format,
    long mode,
    ... )
/*
 * FUNCTION
 *   Say what's going on.  Goes to stdout.
 * INPUTS
 *   * char *format    -- formatstring
 *   * long mode       -- SAY_INFO | SAY_DEBUG
 *   * ...             -- parameters
 * AUTHOR
 *   Koessi
 * HISTORY
 *   22. Nov. 2005   - Multiple mode support (Michel Albert)
 * SOURCE
 */
{
    va_list             ap;

    if ( course_of_action.do_tell && debugmode & mode )
    {
        va_start( ap, mode );
        printf( "%s: ", whoami );
        vprintf( format, ap );
        va_end( ap );
    }
}

/*** RB_Say ***/


/*x**f* Analyser/RB_SetCurrentFile
 * NAME
 *   RB_SetCurrentFile
 * FUNCTION
 *   Set... (TODO Documentation)
 * INPUTS
 *   * filename --
 * SOURCE
 */

void RB_SetCurrentFile(
    char *filename )
{
    current_file = filename;
}

/*******/


/****f* Analyser/RB_GetCurrentFile
 * NAME
 *   Get a copy of the name of the current file.
 *   Allocates memory.
 * SOURCE
 */

char               *RB_GetCurrentFile(
    void )
{
    if ( current_file )
    {
        return RB_StrDup( current_file );
    }
    else
    {
        return NULL;
    }
}

/*******/


/****f* Utilities/RB_Panic [2.01]
 * NAME
 *   RB_Panic -- free resources and shut down
 * SYNOPSIS
 */

void RB_Panic(
    char *format,
    ... )
/*
 * FUNCTION
 *   Print error message.  Frees all resources used by robodoc.
 *   Terminates program.  Output goes to stderr
 * INPUTS
 *   char *format            -- formatstring
 *   ...                     -- parameters
 * AUTHOR
 *   Koessi
 * SOURCE
 */
{
    va_list             ap;
    char               *name;

    va_start( ap, format );

    name = RB_GetCurrentFile(  );

    if ( name )
    {
        char               *buffer_copy = RB_StrDup( myLine );

        RB_StripCR( buffer_copy );
        fprintf( stderr, "%s:\n%s(%d) : Error E1:\n", whoami, name,
                 line_number );
        fprintf( stderr, "   %s\n%s: ", whoami, buffer_copy );
        free( buffer_copy );
        free( name );
    }
    else
    {
        fprintf( stderr, "%s: ", whoami );
    }
    vfprintf( stderr, format, ap );
    fprintf( stderr, "%s: closing down...\n", whoami );
    va_end( ap );
    RB_Close_The_Shop(  );
    exit( EXIT_FAILURE );
}

/*** RB_Panic ***/


/****f* Analyser/RB_Warning_Full
 * NAME
 *   RB_Warning_Full
 * FUNCTION
 *   Print warning to stdout.
 * INPUTS
 *   * arg_filename    --
 *   * arg_line_number --
 *   * arg_format      --
 *   * ...
 * SOURCE
 */

void RB_Warning_Full(
    char *arg_filename,
    int arg_line_number,
    char *arg_format,
    ... )
{
    va_list             ap;

    ++number_of_warnings;
    va_start( ap, arg_format );
    fprintf( stderr, "%s: Warning - %s:%d\n", whoami, arg_filename,
             arg_line_number );
    fprintf( stderr, "  " );
    vfprintf( stderr, arg_format, ap );
    va_end( ap );
}

/*******/


/****f* Analyser/RB_Warning
 * NAME
 *   RB_Warning
 * FUNCTION
 *   Print warning to stdout. (stderr better?)
 * INPUTS
 *   * format --
 *   * ...    --
 * SOURCE
 */

void RB_Warning(
    char *format,
    ... )
{
    static int          count = 1;
    va_list             ap;
    char               *name;

    ++number_of_warnings;
    va_start( ap, format );

    name = RB_GetCurrentFile(  );

    if ( name )
    {
        fprintf( stderr, "%s:\n%s(%d) : Warning R%d:\n", whoami, name,
                 line_number, count );
        free( name );
    }

    fprintf( stderr, "  " );
    vfprintf( stderr, format, ap );
    va_end( ap );

    ++count;
}

/*******/


/****f* Utilities/RB_Str_Case_Cmp
 * FUNCTION
 *   Compare two strings, regardless of the case of the characters.
 * SYNOPSIS
 */
int RB_Str_Case_Cmp(
    char *s,
    char *t )
/*
 * RESULT
 *    0  s == t
 *   -1  s < t
 *    1  s > t
 * SOURCE
 */
{
    assert( s );
    assert( t );
    for ( ; tolower( *s ) == tolower( *t ); s++, t++ )
    {
        if ( *s == '\0' )
        {
            return 0;
        }
    }
    return ( int ) ( tolower( *s ) - tolower( *t ) );
}

/*********/


/****f* Utilities/RB_TimeStamp
 * NAME
 *   RB_TimeStamp -- print a time stamp
 * SOURCE
 */

void RB_TimeStamp(
    FILE *f )
{
    time_t              ttp;
    char                timeBuffer[255];

    time( &ttp );
    strftime( timeBuffer, 255, "%a %b %d %Y %H:%M:%S\n", localtime( &ttp ) );
    fprintf( f, "%s", timeBuffer );
}

/******/


/****f* Utilities/RB_Skip_Whitespace
 * SYNOPSIS
 *   char * RB_Skip_Whitespace(char *buf)
 * FUNCTION
 *   Skip space and tab chars from the start *buf. This is needed when
 *   searching for indented headers and items.
 * NOTES
 *   We should extract some info about indentation level and save it to
 *   global variable in order to write out source items (that originate from
 *   indented headers) neatly.
 * SEE ALSO
 *   RB_Find_Marker, RB_Find_End_Marker, RB_Find_Item, RB_Generate_Item_Body
 * SOURCE
 */

char               *RB_Skip_Whitespace(
    char *buf )
{
    char               *c;

    for ( c = buf; *c; c++ )
    {
        if ( utf8_isspace( *c ) )
        {

        }
        else
        {
            return c;
        }
    }
    return c;                   /* buf was null */
}

/*** RB_Skip_Whitespace ***/


/****f* Utilities/RB_FputcLatin1ToUtf8
 * NAME
 *   RB_FputcLatin1ToUtf8
 * SYNOPSIS
 *   void RB_FputcLatin1ToUtf8(FILE *fp, int c)
 * BUGS
 *   This wrongly assumes that input is always Latin-1.
 * SOURCE
 */

void RB_FputcLatin1ToUtf8(
    FILE *fp,
    int c )
{
    if ( c < 0x80 )
    {
        if ( fputc( c, fp ) == EOF )
            RB_Panic( "RB_FputcLatin1ToUtf8: write error" );
    }
    else
    {
        if ( fputc( ( 0xC0 | ( c >> 6 ) ), fp ) == EOF )
            RB_Panic( "RB_FputcLatin1ToUtf8: write error" );
        if ( fputc( ( 0x80 | ( c & 0x3F ) ), fp ) == EOF )
            RB_Panic( "RB_FputcLatin1ToUtf8: write error" );
    }
}

/*** RB_FputcLatin1ToUtf8 ***/


/****f* Utilities/RB_CopyFile
 * NAME
 *   RB_CopyFile -- copy a file to another file
 * SYNOPSIS
 *   void RB_CopyFile( char* sourceFileName, char* destinationFileName )
 * RESULT
 *   Program Exit if one of the specified files did not open.
 * SOURCE
 */

void RB_CopyFile(
    char *sourceFileName,
    char *destinationFileName )
{
    FILE               *source;

    source = fopen( sourceFileName, "r" );
    if ( source )
    {
        FILE               *dest;

        dest = fopen( destinationFileName, "w" );
        if ( dest )
        {
            for ( ; fgets( line_buffer, MAX_LINE_LEN, source ); )
            {
                fputs( line_buffer, dest );
            }
        }
        else
        {
            fclose( source );
            RB_Panic( "Can't open file %s for writing.\n",
                      destinationFileName );
        }
    }
    else
    {
        RB_Panic( "Can't open file %s for reading\n", sourceFileName );
    }
}

/*****/


/****f* Utilities/RB_Match
 * FUNCTION
 *   See if a wildcard expression matches a target string.  The wildcard
 *   expression can consists of any literal character and the two
 *   wildcards characters '*' and '?'.  '*' matches the longest string
 *   of zero or more characters that fit.  '?' matches any single
 *   character.
 *
 *   Examples:
 *      "*aap"   matches "aapaapaapaap"
 *      "?inux"  matches "linux"
 *      "lin*ux" matches "linux"
 *      "linux*" matches "linux"
 * NOTES
 *   This is a recursive function.
 * SYNOPSIS
 *   int RB_Match( char* target, char* wildcard_expression )
 * INPUTS
 *   o target -- the string to be matched agains the
 *               wildcard_expression.
 *   o wildcard_expression -- the wildcard expression
 * RETURN VALUE
 *   TRUE  -- the target matches the wildcard expression
 *   FALSE -- it does not match.
 * SOURCE
 */

int RB_Match(
    char *target,
    char *wildcard_expression )
{
    if ( ( *wildcard_expression == '\0' ) && ( *target == '\0' ) )
    {
        /* a match, since both strings are now "" */
        return TRUE;
    }
    else if ( *wildcard_expression == '\0' )
    {
        /* we reached the end of the wildcard_expression,
         * but not the end of the target, this is not
         * a match.
         */
        return FALSE;
    }
    else if ( *target == '\0' )
    {
        /* we reached the end of the target but not the end of the
         * wildcard_expression.  Only if the whole wildcard_expression
         * consists of * we have a match.
         */
        unsigned int        i;

        for ( i = 0; i < strlen( wildcard_expression ); ++i )
        {
            if ( wildcard_expression[i] != '*' )
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    else
    {
        /* There are wildcard_expression characters left
         * and target characters left.
         */
        char                wildcard = wildcard_expression[0];

        if ( wildcard == '?' )
        {
            /* Match a single character and see if the
             * rest of the target matches.
             */
            return RB_Match( target + 1, wildcard_expression + 1 );
        }
        else if ( wildcard == '*' )
        {
            int                 match = FALSE;
            int                 l = strlen( target );
            int                 i;

            /* First try to match all of the target string, and
             * then work back to the begin of the target string.
             * ( including the "" string. )
             */
            for ( i = l; i >= 0; --i )
            {
                if ( RB_Match( target + i, wildcard_expression + 1 ) )
                {
                    match = TRUE;
                    break;
                }
            }
            return match;
        }
        else
        {
            int                 l_w = strlen( wildcard_expression );
            int                 l_t = strlen( target );

            /* The minimum of the length of the wildcard_expression
             * and target expression
             */
            int                 l = ( l_w <= l_t ) ? l_w : l_t;
            int                 i;

            for ( i = 0; i < l; ++i )
            {
                if ( ( wildcard_expression[i] != '*' ) &&
                     ( wildcard_expression[i] != '?' ) )
                {
                    /* Some OS-es are not case-sensitive when it comes
                     * to file names, and consider Readme to be equal
                     * to README.  On these OS-es it can be handy if
                     * robodoc is also not case-sensitive.
                     */
#ifdef IGNORE_CASE_FILENAMES
                    if ( tolower( wildcard_expression[i] ) !=
                         tolower( target[i] ) )
#else
                    if ( wildcard_expression[i] != target[i] )
#endif
                    {
                        return FALSE;
                    }
                }
                else
                {
                    return RB_Match( target + i, wildcard_expression + i );
                }
            }
            /* The first l characters of the target and
             * wildcard_expression matched, now see if the rest
             * matches too.
             */
            return RB_Match( target + l, wildcard_expression + l );
        }
    }
}

/******/


/****f* Utilities/RB_Swap
 * FUNCTION
 *   Swap two elements in a array of pointers.  This function is used
 *   by RB_QuickSort().
 * SOURCE
 */

static void RB_Swap(
    void **array,
    int left,
    int right )
{
    void               *p = array[left];

    array[left] = array[right];
    array[right] = p;
}

/*****/


/****f* Utilities/RB_QuickSort
 * FUNCTION
 *   Sort an array of pointers according to the lexical order
 *   of the elements the pointers point to.
 *   This is based on the quicksort routine in
 *   "The C programming language" by B Kerninghan en D Ritchie.
 * INPUTS
 *   * array -- the array of pointers.
 *   * left  -- the most left element in the array.
 *   * right -- the most right element in the array.
 *   * f     -- pointer to a function that can compare
 *              the objects two elements of the array
 *              point to.
 * RESULT
 *   array -- A sorted array of pointers.
 *
 * EXAMPLE
 *   The following is an example program that shows
 *   the use
 *    #define TEST_SIZE 10
 *
 *    char* test[ TEST_SIZE ] = { "ape", "zebra",
 *       "duck", "goofbal", "dodo", "rabit",
 *       "crow", "cow", "pig", "goat" };
 *
 *    int string_compare( void* p1, void* p2 )
 *    {
 *       char *cp1 = p1;
 *       char *cp2 = p2;
 *       return strcmp( cp1, cp2 );
 *    }
 *
 *    RB_QuickSort( test, 0, TEST_SIZE - 1, string_compare );
 *
 * SOURCE
 */

void RB_QuickSort(
    void **array,
    int left,
    int right,
    TCompare f )
{
    int                 i;
    int                 last;

    if ( left >= right )
    {
        return;
    }

    RB_Swap( array, left, ( left + right ) / 2 );
    last = left;
    for ( i = left + 1; i <= right; ++i )
    {
        if ( ( *f ) ( array[i], array[left] ) < 0 )
        {
            RB_Swap( array, ++last, i );
        }
    }
    RB_Swap( array, left, last );
    RB_QuickSort( array, left, last - 1, f );
    RB_QuickSort( array, last + 1, right, f );
}

/*******/


/****f* Utilities/RB_StripCR
 * FUNCTION
 *   Strip carriage return (CR) from line.
 * INPUTS
 *   * line -- line string to process
 * SOURCE
 */

void RB_StripCR(
    char *line )
{
    char               *c;

    for ( c = line; *c; ++c )
    {
        if ( *c == '\n' )
        {
            *c = '\0';
        }
    }
}

/*******/


/****f* Utilities/RB_ContainsNL
 * FUNCTION
 *   Check whether the provided line buffer contains
 *   a new line (NL) character.
 * INPUTS
 *   * line -- line string to process
 * RETURN VALUE
 *   * TRUE  -- the line contains a NL character
 *   * FALSE -- the line does not contain a NL character
 * SOURCE
 */

int RB_ContainsNL(
    char *line )
{
    int                 found = 0;

    for ( ; *line != '\0'; ++line )
    {
        if ( *line == '\n' )
        {
            found = 1;
        }
    }
    return found;
}

/*******/


/****f* Utilities/RB_FreeLineBuffer
 * FUNCTION
 *   Free the dynamically allocated line-buffer
 * INPUTS
 *   * works on the globals line_buffer, readChars, myLine
 * SOURCE
 */

void RB_FreeLineBuffer(
     )
{
    *line_buffer = '\0';
    free( myLine );
    myLine = NULL;
    readChars = 0;
}

/*******/

/****f* Utilities/CR_LF_Conversion
 * FUNCTION
 *   Fix CR/LF problems.
 *
 *   If ROBODoc reads a text file that was created on another OS
 *   line-endings might not be what ROBODoc expects of the current OS.
 *   This function tries to detect and fix this.
 *
 * INPUTS
 *   * line -- a line of text
 * RETURN VALUE
 *   * number of characters that were removed.
 * SOURCE
 */

static int CR_LF_Conversion(
    char *line )
{
    int                 n = strlen( line );

    if ( ( n > 1 ) &&
         ( ( ( line[n - 2] == '\r' ) && ( line[n - 1] == '\n' ) )
           || ( ( line[n - 2] == '\n' ) && ( line[n - 1] == '\r' ) ) ) )
    {
        line[n - 2] = '\n';
        line[n - 1] = '\0';
        return 1;
    }
    else
    {
        /* No problem */
        return 0;
    }
}

/******/


/****f* Utilities/RB_ReadWholeLine
 * FUNCTION
 *   Read a single line from the file using the provided buffer.
 * INPUTS
 *   * file -- file to read from
 *   * buf -- buffer of length MAX_LINE_LEN to read chunks of the line to
 *   * arg_readChars -- reference to the variable to store the read characters in
 * RETURN VALUE
 *   * returns a dynamically allocated buffer containing the complete line
 *     read
 * NOTES
 *   If the line did not end in a new line (NL) character one is added.
 * SOURCE
 */

char               *RB_ReadWholeLine(
    FILE *file,
    char *buf,
    int *arg_readChars )
{
    int                 foundNL = 0;
    char               *line = NULL;
    int                 curLineLen = 0;
    int                 chunkLen = 0;

    clearerr( file );
    while ( ( !feof( file ) ) && ( !foundNL ) )
    {
        *buf = '\0';
        /* read next chunk */
        fgets( buf, MAX_LINE_LEN, file );
        if ( ferror( file ) )
        {
            /* an error occurred */
            RB_Panic( "I/O error %d! RB_ReadWholeLine()", errno );
        }
        chunkLen = strlen( buf );
        curLineLen += chunkLen;
        /* make room for the chunk in our buffer  ( +1 for the '\0') */
        if ( ( line = realloc( line, sizeof( char ) * ( curLineLen + 1 ) ) ) == NULL )
        {
            /* we run out of memory */
            RB_Panic( "Out of memory! RB_ReadWholeLine()" );
        }
        /* append the chunk to our buffer */
        strcpy( ( line + curLineLen - chunkLen ), buf );

        if ( RB_ContainsNL( buf ) )
        {
            /* we are done - a line was read */
            foundNL = 1;
        }
    }

    if ( !foundNL )
    {
        /* last line has no NL - add one */
        ++curLineLen;

        /* + 1 for the '\0' */
        if ( ( line = realloc( line, sizeof( char ) * ( curLineLen + 1 ) ) ) == NULL )
        {
            /* we run out of memory */
            RB_Panic( "Out of memory! RB_ReadWholeLine()" );
        }
        line[curLineLen - 1] = '\n';
        line[curLineLen ] = '\0';
    }

    /* This fixes any cr/lf problems. */
    curLineLen -= CR_LF_Conversion( line );

    *arg_readChars = curLineLen;
    *buf = '\0';

    return line;
}

/*******/


/****f* Utilities/Stat_Path
 * FUNCTION
 *   Check the given path against required type.
 *   d -- directory, f -- file, e -- exists
 * RETURN VALUE
 *   TRUE if path is of the given type, otherwise FALSE.
 * BUGS
 *   Should check if symbolic link points to a directory or to a file.
 * SOURCE
 */

int Stat_Path(
    char required,
    char *path )
{
    struct stat         st;
    int                 res = FALSE;

    if ( stat( path, &st ) < 0 )
    {
        if ( required == 'e' )
        {
            res = FALSE;
        }
        else
        {
            if ( ( strcmp( path, "./" ) == 0 ) && ( required == 'd' ) )
            {
                /* This fixes a bug in Mingw, where ./ can not be
                   stat-ed under windows2000 and above,
                   we just assume that ./ always exists. */
                res = TRUE;
            }
            else
            {
                RB_Panic( "Stat_Path: can not stat '%s'\n", path );
            }
        }
    }
    else
    {
        switch ( ( ( st.st_mode ) & S_IFMT ) )
        {
        case S_IFDIR:
            if ( ( required == 'd' ) || ( required == 'e' ) )
            {
                res = TRUE;
            }
            break;
        case S_IFREG:
            if ( ( required == 'f' ) || ( required == 'e' ) )
            {
                res = TRUE;
            }
            break;
            /* TODO case S_IFLNK: chdir() */
        default:
            break;
        }                       /* end switch */
    }
    return res;
}

/*******/


/****f* Utilities/RB_malloc
 * FUNCTION
 *   like malloc, but exit if malloc failed
 * RETURN VALUE
 *   See malloc
 * SOURCE
 */

void               *RB_malloc(
    size_t bytes )
{
    void               *tmp;

    tmp = malloc( bytes );

    if ( tmp == NULL )
    {
        RB_Panic( "Unable to malloc %d bytes", bytes );
    }

    return tmp;
}

/*******/


/****f* Utilities/cwd
 * FUNCTION
 *   Holds current working directory
 * SOURCE
 */
static char        *cwd = NULL;

/*******/

/****f* Utilities/RB_Change_Back_To_CWD
 * FUNCTION
 *   Changes back to saved working directory and frees cwd string
 * SOURCE
 */
void RB_Change_Back_To_CWD(
    void )
{
    if ( cwd != NULL )
    {
        chdir( cwd );
        free( cwd );
        cwd = NULL;
    }
}

/*******/

/****f* Utilities/RB_Change_To_Docdir
 * FUNCTION
 *   Saves current working directory and then changes to document dir
 * SOURCE
 */
void RB_Change_To_Docdir(
    char *docname )
{
    char                tmp[TEMP_BUF_SIZE], *namestart;
    int                 len;

    /* Check if we have a valid directory name in the docname */
    namestart = strrchr( docname, '/' );
    if ( namestart == NULL )
        RB_Panic( "Unable to get the directory name of '%s'", docname );

    /* Just for sure */
    RB_Change_Back_To_CWD(  );

    /* Save CWD */
    getcwd( tmp, sizeof( tmp ) );
    cwd = RB_StrDup( tmp );

    /* Get the name of doc directory and change into it */
    len = namestart - docname;
    strncpy( tmp, docname, len );
    tmp[len] = 0;
    chdir( tmp );
}

/*******/


/****f* Utilities/RB_Open_Pipe
 * FUNCTION
 *   Opens a pipe and returns its handler
 * SOURCE
 */
FILE               *RB_Open_Pipe(
    char *pipe_name )
{
    FILE               *a_pipe;

    a_pipe = popen( pipe_name, "w" );

    if ( a_pipe == NULL )
    {
        RB_Panic( "Unable to open pipe to '%s'", pipe_name );
    }
    return a_pipe;
}

/*******/


/****f* Utilities/RB_Close_Pipe
 * FUNCTION
 *   Closes a given pipe
 * SOURCE
 */
void RB_Close_Pipe(
    FILE *arg_pipe )
{
    if ( arg_pipe != NULL )
    {
        pclose( arg_pipe );
    }
}

/*******/


/****f* Utilities/RB_Open_File
 * FUNCTION
 *   Opens a file and returns its handler
 * SOURCE
 */
FILE               *RB_Open_File(
    char *file_name,
    char *mode )
{
    FILE               *a_file;

    a_file = fopen( file_name, mode );

    if ( a_file == NULL )
    {
        RB_Panic( "Unable to open file '%s' with mode '%s'", file_name,
                  mode );
    }
    return a_file;
}

/*******/


/****f* Utilities/RB_Close_File
 * FUNCTION
 *   Closes a given file
 * SOURCE
 */
void RB_Close_File(
    FILE *arg_file )
{
    if ( arg_file != NULL )
    {
        fclose( arg_file );
    }
}

/*******/


/* TODO Move this to a separate file */
/* TODO grow this into a whole set of utf8 routines. */
/* TODO Documentation */

int utf8_isalnum(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isalnum( arg_c ) );
}

int utf8_isalpha(
    unsigned int arg_c )
{
    /* We assume here that all non ascii characters
     * are alphabetic characters. */
    return ( ( arg_c >= 128 ) || ( arg_c < 128 && isalpha( arg_c ) ) );
}

int utf8_iscntrl(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && iscntrl( arg_c ) );
}

int utf8_isdigit(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isdigit( arg_c ) );
}

int utf8_isgraph(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isgraph( arg_c ) );
}

int utf8_islower(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && islower( arg_c ) );
}

int utf8_isprint(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isprint( arg_c ) );
}

int utf8_ispunct(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && ispunct( arg_c ) );
}

int utf8_isspace(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isspace( arg_c ) );
}

int utf8_isupper(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isupper( arg_c ) );
}

int utf8_isxdigit(
    unsigned int arg_c )
{
    return ( ( arg_c < 128 ) && isxdigit( arg_c ) );
}


/****f* Utilities/snprintf
 * FUNCTION
 *   Mimic the library function snprintf using sprintf if it is absent.
 * SOURCE
 */
#ifndef HAVE_SNPRINTF
int snprintf(
    char *str,
    size_t size,
    const char *format,
    ... )
{
    va_list             ap;
    int                 retval;

    /* This variable remains unused... */
    USE( size );

    /* Mimic snprintf */
    va_start( ap, format );
    retval = sprintf( str, format, ap );
    va_end( ap );

    return ( retval );
}
#endif
/*******/
