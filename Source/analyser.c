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



/****h* ROBODoc/Analyser
 * NAME
 *   Analyser -- Functions to scan source and collect headers
 * FUNCTION
 *   This module provides the functions to scan a sourcefile and
 *   collect all the headers.
 *
 *****
 * $Id: analyser.c,v 1.78 2009/03/23 21:15:04 gumpu Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "robodoc.h"
#include "globals.h"
#include "headers.h"
#include "headertypes.h"
#include "items.h"
#include "util.h"
#include "links.h"
#include "analyser.h"
#include "document.h"
#include "file.h"
#include "part.h"
#include "roboconfig.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int          ToBeAdded(
    struct RB_Document *document,
    struct RB_header *header );
static int          Find_Header_Name(
    FILE *,
    struct RB_header * );
static struct RB_header *Grab_Header(
    FILE *sourcehandle,
    struct RB_Document *arg_document );
static char        *Function_Name(
    char *header_name );
static char        *Module_Name(
    char *header_name );
static int          Find_End_Marker(
    FILE *document,
    struct RB_header *new_header );
struct RB_HeaderType *AnalyseHeaderType(
    char **cur_char,
    int *is_internal );
static struct RB_HeaderType *RB_Find_Marker(
    FILE *document,
    int *is_internal,
    int reuse_previous_line );
static int          Analyse_Items(
    struct RB_header *arg_header );
static int          Is_ListItem_Start(
    char *arg_line,
    int arg_indent );

/****f* Analyser/Is_Pipe_Marker
 * NAME
 *   Is_Pipe_Marker
 * FUNCTION
 *   Check for "pipe" markers e.g. "|html ". 
 * SYNOPSIS
 */
static char        *Is_Pipe_Marker(
    char *cur_char,
    int *pipe_mode )
/*
 * RESULT
 *   Pointer to the data to be piped to document or in case no pointers
 *   are found.
 * SEE ALSO
 *   RB_Check_Pipe
 * SOURCE
 */
{
    char               *s = cur_char + 1;

    *pipe_mode = -1;
    if ( *cur_char == '|' && *s )
    {
        if ( strncmp( "html ", s, 5 ) == 0 )
        {
            *pipe_mode = HTML;
            return ( s + 5 );
        }
        else if ( strncmp( "latex ", s, 6 ) == 0 )
        {
            *pipe_mode = LATEX;
            return ( s + 6 );
        }
        else if ( strncmp( "rtf ", s, 4 ) == 0 )
        {
            *pipe_mode = RTF;
            return ( s + 4 );
        }
        else if ( strncmp( "dbxml ", s, 6 ) == 0 )
        {
            *pipe_mode = XMLDOCBOOK;
            return ( s + 6 );
        }
        else if ( strncmp( "ascii ", s, 6 ) == 0 )
        {
            *pipe_mode = ASCII;
            return ( s + 6 );
        }
    }

    return ( char * ) NULL;
}

/*****/


/****f* Analyser/Is_Tool
 * SYNOPSIS
 */
static char        *Is_Tool(
    char *cur_char,
    enum ItemLineKind *itemkind,
    int *tool_active )
/*
 * FUNCTION
 *   Checks for tool start and end markers
 * SOURCE
 */
{
    char               *s = cur_char + 1;

    if ( *cur_char == '|' && *s )
    {
        /* Check if tool starts or ends */
        if ( !strncmp( "tool ", s, 5 ) )
        {
            if ( *tool_active )
            {
                *itemkind = ITEM_LINE_TOOL_END;
                *tool_active = 0;
            }
            else
            {
                *itemkind = ITEM_LINE_TOOL_START;
                *tool_active = 1;
            }

            return ( s + 5 );
        }
        /* Check if DOT starts or ends */
        if ( !strncmp( "dot ", s, 4 ) )
        {
            if ( *tool_active )
            {
                *itemkind = ITEM_LINE_DOT_END;
                *tool_active = 0;
            }
            else
            {
                *itemkind = ITEM_LINE_DOT_START;
                *tool_active = 1;
            }

            return ( s + 4 );
        }
        /* Check for DOT file includes */
        else if ( !strncmp( "dotfile ", s, 8 ) && !*tool_active )
        {
            *itemkind = ITEM_LINE_DOT_FILE;
            return ( s + 8 );
        }
        /* Check for exec items */
        else if ( !strncmp( "exec ", s, 5 ) && !*tool_active )
        {
            *itemkind = ITEM_LINE_EXEC;
            return ( s + 5 );
        }
    }

    return NULL;
}

/*****/

/****f* Analyser/RB_Analyse_Document
 *   foo
 * FUNCTION
 *   Scan all the sourcefiles of all parts of a document for
 *   headers.  Store these headers in each part (RB_Part).
 * SYNOPSIS
 */
void RB_Analyse_Document(
    struct RB_Document *arg_document )
/*
 * INPUTS
 *   o document -- document to be analysed.
 * RESULT
 *   Each part will contain the headers that were found in the
 *   sourcefile of the part.
 * SOURCE
 */
{
    struct RB_Part     *a_part;
    struct RB_Filename *a_filename;
    FILE               *filehandle;

    for ( a_part = arg_document->parts; a_part; a_part = a_part->next )
    {
        struct RB_header   *new_header = NULL;

        a_filename = a_part->filename;
        RB_Say( "analysing %s\n", SAY_DEBUG, Get_Fullname( a_filename ) );
        RB_SetCurrentFile( Get_Fullname( a_filename ) );

        RB_Header_Lock_Reset(  );
        filehandle = RB_Open_Source( a_part );
        line_number = 0;

        for ( new_header = Grab_Header( filehandle, arg_document );
              new_header;
              new_header = Grab_Header( filehandle, arg_document ) )
        {
            if ( ToBeAdded( arg_document, new_header ) )
            {
                /* The Add is required before the 
                 * Analyse because Add sets the owner of the header
                 * which is needed for error messages.
                 */
                RB_Part_Add_Header( a_part, new_header );
                Analyse_Items( new_header );
            }
            else
            {
                RB_Free_Header( new_header );
            }
        }
        fclose( filehandle );
    }
}

/*****/


/****f* Analyser/Is_Empty_Line
 * FUNCTION
 *   Check if line is empty. This assumes that 
 *   Copy_Lines_To_Item has been run on the item.
 * SYNOPSIS
 */
static int Is_Empty_Line(
    char *line )
/*
 * INPUTS
 *   * line -- the string to be analysed.
 * SOURCE
 */
{
    line = RB_Skip_Whitespace( line );
    return ( *line == '\0' );
}

/******/


/****f* Analyser/Get_Indent
 * FUNCION
 *   Determine the indentation of a line by counting
 *   the number of spaces at the begining of the line.
 * SYNOPSIS
 */
static int Get_Indent(
    char *line )
/*
 * INPUTS
 *   * line -- the line
 * RESULT
 *   The indentation.
 * SOURCE
 */
{
    int                 i;

    for ( i = 0; line[i] && utf8_isspace( line[i] ); ++i )
    {
        /* empty */
    }
    return i;
}

/*****/


/****f* Analyser/Is_ListItem_Start
 * FUNCTION
 *   Test if a line starts with something that indicates a list item.
 *   List items start with '*', '-', or 'o'.
 * SYNPOPSIS
 */
static int Is_ListItem_Start(
    char *arg_line,
    int arg_indent )
/*
 * INPUTS
 *   * line -- the line
 * RESULT
 *   * TRUE  -- it did start with one of those characters
 *   * FALSE -- it did not.
 * SOURCE
 */
{
    char               *c = arg_line;
    int                 cur_indent = Get_Indent( arg_line );

    if ( cur_indent == arg_indent )
    {
        /* TODO  Think there is a function for this */
        for ( ; *c && utf8_isspace( *c ); ++c )
        {                       /* empty */
        };

        if ( *c && ( strlen( c ) >= 3 ) )
        {
            if ( strchr( "*-o", *c ) && utf8_isspace( *( c + 1 ) ) )
            {
                return TRUE;
            }
        }
    }
    else
    {
        /* The line is indented so it must be
         * the start of a pre block  */
    }

    return FALSE;
}

/*****/


/****f* Analyser/Is_List_Item_Continuation
 * FUNCTION
 *   Is it like the second line in something like:
 *     * this is a list item
 *       that continues 
 * SYNPOPSIS
 */
static int Is_List_Item_Continuation(
    char *arg_line,
    int indent )
/*
 * INPUTS
 *   * arg_line  -- the current line
 *   * indent    -- the indent of the current item.
 * RESULT
 *   * TRUE  -- it is.
 *   * FALSE -- it is not.
 * SOURCE
 */
{
    int                 this_indent = Get_Indent( arg_line );

    return ( this_indent > indent );
}

/*****/


/****f* Analyser/Is_Start_List
 * FUNCTION
 *   Check... (TODO) 
 * INPUTS
 *   * arg_line --
 *   * indent  --
 * SOURCE
 */

static int Is_Start_List(
    char *arg_line,
    int indent )
{
    int                 this_indent = Get_Indent( arg_line );
    char               *c = strrchr( arg_line, ':' );

    if ( ( this_indent == indent ) && c )
    {
        for ( ++c; *c; ++c )
        {
            if ( !utf8_isspace( *c ) )
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

/*******/


/****f* Analyser/Remove_List_Char
 * FUNCTION
 *   Remove... (TODO) 
 * INPUTS
 *   * arg_item    -- the item to be analysed.
 *   * start_index -- 
 * SOURCE
 */

static void Remove_List_Char(
    struct RB_Item *arg_item,
    int start_index )
{
    char               *c = arg_item->lines[start_index]->line;

    for ( ; *c && utf8_isspace( *c ); ++c )
    {                           /* empty */
    };
    if ( *c && ( strlen( c ) >= 3 ) )
    {
        if ( strchr( "*-o", *c ) && utf8_isspace( *( c + 1 ) ) )
        {
            char               *temp = arg_item->lines[start_index]->line;

            *c = ' ';
            arg_item->lines[start_index]->line = RB_StrDup( temp + 2 );
            free( temp );
        }
    }
}

/*******/

/****f* Analyser/Analyse_ListBody
 * FUNCTION
 *   Analyse... (TODO) 
 * SYNPOPSIS
 */
static int Analyse_ListBody(
    struct RB_Item *arg_item,
    int start_index,
    int arg_indent )
/*
 * INPUTS
 *   * arg_item    -- the item to be analysed.
 *   * start_index --
 *   * arg_index   --
 * SOURCE
 */
{
    int                 i = start_index;

    for ( ; i < arg_item->no_lines; i++ )
    {
        char               *line = arg_item->lines[i]->line;

        if ( ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN ) ||
             ( arg_item->lines[i]->kind == ITEM_LINE_END ) )
        {
            if ( Is_ListItem_Start( line, arg_indent ) )
            {
                arg_item->lines[i]->format |= RBILA_END_LIST_ITEM;
                arg_item->lines[i]->format |= RBILA_BEGIN_LIST_ITEM;
                Remove_List_Char( arg_item, i );
            }
            else if ( Is_List_Item_Continuation( line, arg_indent ) )
            {
                /* Nothing */
            }
            else
            {
                /* Must be the end of the list */
                arg_item->lines[i]->format |= RBILA_END_LIST_ITEM;
                arg_item->lines[i]->format |= RBILA_END_LIST;
                break;
            }
        }
    }
    return i;
}

/*******/

/****f* Analyser/Analyse_List
 * FUNCTION 
 *   Parse the item text to see if there are any lists.
 *   A list is either case I:
 *      ITEMNAME
 *         o bla bla
 *         o bla bla
 *   or case II:
 *      some text:     <-- begin of a list
 *      o bla bla      <-- list item
 *        bla bla bla  <-- continuation of list item.
 *      o bla bla      <-- list item
 *                     <-- end of a list 
 *      bla bla        <-- this can also be the end of a list.
 * SYNPOPSIS
 */
static void Analyse_List(
    struct RB_Item *arg_item,
    int indent )
/*
 * INPUTS
 *   * arg_item  -- the item to be parsed.
 *   * indent    -- the indent of this item.
 * OUTPUT
 *   * arg_item  -- the itemlines will contain formatting information 
 *                  for all the lists that were found.
 * SOURCE
 */
{
    if ( arg_item->no_lines >= 1 )
    {
        int                 i = 0;
        char               *line = arg_item->lines[i]->line;

        /* Case I */
        if ( ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN ) &&
             Is_ListItem_Start( line, indent ) )
        {
            /* Case I, the is a list item right after the item name */
            arg_item->lines[i]->format |= RBILA_BEGIN_LIST;
            arg_item->lines[i]->format |= RBILA_BEGIN_LIST_ITEM;
            Remove_List_Char( arg_item, i );
            /* Now try to find the end of the list */
            i = Analyse_ListBody( arg_item, 1, indent );
        }

        /* Now search for case II cases */
        for ( ; i < arg_item->no_lines; i++ )
        {
            line = arg_item->lines[i]->line;
            if ( ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN ) &&
                 Is_Start_List( line, indent ) )
            {
                ++i;
                if ( i < arg_item->no_lines )
                {
                    line = arg_item->lines[i]->line;
                    if ( ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN ) &&
                         Is_ListItem_Start( line, indent ) )
                    {
                        arg_item->lines[i]->format |= RBILA_BEGIN_LIST;
                        arg_item->lines[i]->format |= RBILA_BEGIN_LIST_ITEM;
                        Remove_List_Char( arg_item, i );
                        ++i;
                        i = Analyse_ListBody( arg_item, i, indent );


                        /* One list might be immediately followed
                         * by another. In this case we have to
                         * analyse the last line again. */
                        line = arg_item->lines[i]->line;
                        if ( ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN )
                             && Is_Start_List( line, indent ) )
                        {
                            --i;
                        }

                    }
                }
            }
        }
    }
}

/******/


/****f* Analyser/Preformat_All
 * FUNCTION
 *   Process... (TODO) 
 * SYNPOPSIS
 */
static void Preformat_All(
    struct RB_Item *arg_item,
    int source )
/*
 * INPUTS
 *   * arg_item -- the item to be pre-formatted.
 *   * source   -- is it a source item ?
 * SOURCE
 */
{
    int                 i;
    int                 preformatted = FALSE;
    char               *line = NULL;

    if ( arg_item->no_lines > 0 )
    {
        i = 0;
        /* Skip any pipe stuff */
        for ( ;
              ( i < arg_item->no_lines )
              && ( arg_item->lines[i]->kind == ITEM_LINE_PIPE ); ++i )
        {
            /* Empty */
        }

        line = arg_item->lines[i]->line;
        if ( ( arg_item->lines[i]->kind == ITEM_LINE_RAW ) ||
             ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN ) )
        {
            arg_item->lines[i]->format |=
                RBILA_BEGIN_PRE | ( source ? RBILA_BEGIN_SOURCE : 0 );
            preformatted = TRUE;

            for ( ++i; i < arg_item->no_lines; i++ )
            {
                if ( arg_item->lines[i]->kind == ITEM_LINE_PIPE )
                {
                    /* Temporarily end the preformatting to allow
                     * the piping to happen
                     */
                    arg_item->lines[i]->format |=
                        RBILA_END_PRE | ( source ? RBILA_END_SOURCE : 0 );
                    /* Find the end of the pipe stuff */
                    for ( ; ( i < arg_item->no_lines ) &&
                          ( arg_item->lines[i]->kind == ITEM_LINE_PIPE );
                          ++i )
                    {           /* Empty */
                    };
                    /* Every item ends with an ITEM_LINE_END, so: */
                    assert( i < arg_item->no_lines );
                    /* And re-enable preformatting */
                    arg_item->lines[i]->format |=
                        RBILA_BEGIN_PRE | ( source ? RBILA_BEGIN_SOURCE : 0 );
                }

                if ( arg_item->lines[i]->kind == ITEM_LINE_END )
                {
                    /* If the last line ends with a begin_pre remove
                     * it, otherwise a begin and end pre will be
                     * generated, in the wrong order, on the same line in the output.
                     */
                    if ( arg_item->lines[i]->format & RBILA_BEGIN_PRE )
                    {
                        arg_item->lines[i]->format &= ~( RBILA_BEGIN_PRE );
                    }
                    else
                    {
                        arg_item->lines[i]->format |= RBILA_END_PRE;
                    }
                    arg_item->lines[i]->format |=
                        ( source ? RBILA_END_SOURCE : 0 );
                }
            }
        }
    }
}

/******/

/*
 *     aaaaa
 *       aa
 *               </p>
 *       aa      <p>
 *     aaa
 *     aaaa
 *
 *
 */

/****f* Analyser/Analyse_Preformatted
 * FUNCTION
 *   Analyse preformatted text ... (TODO) 
 * SYNPOPSIS
 */
static void Analyse_Preformatted(
    struct RB_Item *arg_item,
    int indent )
/*
 * INPUTS
 *   * arg_item -- the item to be analysed.
 *   * indent   -- 
 * SOURCE
 */
{
    int                 i;
    int                 in_list = FALSE;
    int                 new_indent = -1;
    int                 preformatted = FALSE;
    char               *line = NULL;

    /* preformatted blocks */
    if ( arg_item->no_lines > 0 )
    {
        i = 0;
        /* Skip any pipe stuff */
        for ( ;
              ( i < arg_item->no_lines )
              && ( arg_item->lines[i]->kind == ITEM_LINE_PIPE ); ++i )
        {
            /* Empty */
        }

        line = arg_item->lines[i]->line;

        if ( ( !in_list )
             && ( arg_item->lines[i]->format & RBILA_BEGIN_LIST ) )
        {
            in_list = TRUE;
        }
        if ( ( in_list ) && ( arg_item->lines[i]->format & RBILA_END_LIST ) )
        {
            in_list = FALSE;
        }

        for ( ++i; i < arg_item->no_lines; i++ )
        {
            if ( arg_item->lines[i]->kind == ITEM_LINE_PIPE )
            {
                if ( preformatted )
                {
                    arg_item->lines[i]->format |= RBILA_END_PRE;
                }
                for ( ; ( i < arg_item->no_lines ) &&
                      ( arg_item->lines[i]->kind == ITEM_LINE_PIPE ); ++i )
                {               /* Empty */
                };
                /* Every item ends with an ITEM_LINE_END, so: */
                assert( i < arg_item->no_lines );
                if ( preformatted )
                {
                    arg_item->lines[i]->format |= RBILA_BEGIN_PRE;
                }
            }

            line = arg_item->lines[i]->line;
            new_indent = Get_Indent( line );

            if ( ( !in_list )
                 && ( arg_item->lines[i]->format & RBILA_BEGIN_LIST ) )
            {
                in_list = TRUE;
            }
            if ( ( in_list )
                 && ( arg_item->lines[i]->format & RBILA_END_LIST ) )
            {
                in_list = FALSE;
            }

            if ( !in_list )
            {
                if ( ( new_indent > indent ) && !preformatted )
                {
                    preformatted = TRUE;
                    arg_item->lines[i]->format |= RBILA_BEGIN_PRE;
                }
                else if ( ( new_indent <= indent ) && preformatted )
                {
                    preformatted = FALSE;
                    arg_item->lines[i]->format |= RBILA_END_PRE;
                }
                else
                {
                    /* An empty line */
                }
            }
            else
            {
                /* We are in a list, do nothing */
            }
        }
    }
}

/******/

/****f* Analyser/Analyse_Paragraphs
 * FUNCTION
 *   Analyse paragraphs... (TODO) 
 * SYNPOPSIS
 */
static void Analyse_Paragraphs(
    struct RB_Item *arg_item )
/*
 * INPUTS
 *   * arg_item -- the item to be analysed.
 * SOURCE
 */
{
    int                 i;
    int                 in_par = FALSE;
    int                 in_list = FALSE;
    int                 in_pre = FALSE;
    int                 is_empty = FALSE;
    int                 prev_is_empty = FALSE;

    for ( i = 0;
          ( i < arg_item->no_lines )
          && ( arg_item->lines[i]->kind == ITEM_LINE_PIPE ); ++i )
    {
        /* Empty */
    }
    assert( i < arg_item->no_lines );

    if ( ( arg_item->lines[i]->format == 0 ) )
    {
        arg_item->lines[i]->format |= RBILA_BEGIN_PARAGRAPH;
        in_par = TRUE;
    }
    for ( ; i < arg_item->no_lines; i++ )
    {
        char               *line = arg_item->lines[i]->line;

        prev_is_empty = is_empty;
        is_empty = Is_Empty_Line( line );
        if ( arg_item->lines[i]->format & RBILA_BEGIN_LIST )
        {
            in_list = TRUE;
        }
        if ( arg_item->lines[i]->format & RBILA_BEGIN_PRE )
        {
            in_pre = TRUE;
        }
        if ( arg_item->lines[i]->format & RBILA_END_LIST )
        {
            in_list = FALSE;
        }
        if ( arg_item->lines[i]->format & RBILA_END_PRE )
        {
            in_pre = FALSE;
        }
        if ( in_par )
        {
            if ( ( arg_item->lines[i]->format & RBILA_BEGIN_LIST ) ||
                 ( arg_item->lines[i]->format & RBILA_BEGIN_PRE ) ||
                 is_empty )
            {
                in_par = FALSE;
                arg_item->lines[i]->format |= RBILA_END_PARAGRAPH;
            }
        }
        else
        {
            if ( ( arg_item->lines[i]->format & RBILA_END_LIST ) ||
                 ( arg_item->lines[i]->format & RBILA_END_PRE ) ||
                 ( !is_empty && prev_is_empty && !in_list && !in_pre ) )
            {
                in_par = TRUE;
                arg_item->lines[i]->format |= RBILA_BEGIN_PARAGRAPH;
            }
        }
    }
    if ( in_par )
    {
        arg_item->lines[arg_item->no_lines - 1]->format |=
            RBILA_END_PARAGRAPH;
    }
}

/******/


/****f* Analyser/Analyse_Indentation
 * FUNCTION
 *  Figure out how text is indented. 
 * SYNPOPSIS
 */
static int Analyse_Indentation(
    struct RB_Item *arg_item )
/*
 * INPUTS
 *   * arg_item -- the item to be analysed.
 * SOURCE
 */
{
    int                 i;
    int                 indent = -1;

    assert( arg_item->no_lines > 0 );

    for ( i = 0; i < arg_item->no_lines; ++i )
    {
        if ( arg_item->lines[i]->kind == ITEM_LINE_PLAIN )
        {
            char               *line = arg_item->lines[i]->line;

            if ( Is_Empty_Line( line ) )
            {
                /* Empty */
                indent = 0;
            }
            else
            {
                indent = Get_Indent( line );
                break;
            }
        }
    }
    assert( indent >= 0 );
    return indent;
}

/******/

/****f* Analyser/Analyse_Item_Format
 * FUNCTION
 *   Try to determine the formatting of an item.
 *   An empty line generates a new paragraph
 *   Things that are indented more that the rest of the text
 *   are preformatted.
 *   Things that start with a '*' or '-' create list items
 *   unless they are indented more that the rest of the text.
 * SYNPOPSIS
 */
static void Analyse_Item_Format(
    struct RB_Item *arg_item )
/*
 * INPUTS
 *   * arg_item -- the item to be analysed.
 * SOURCE
 */
{
    /* If item is not empty */
    if ( arg_item->no_lines )
    {
        /* If it is a SOURCE item */
        if ( Works_Like_SourceItem( arg_item->type ) )
        {
            /* Preformat it like a SOURCE item */
            Preformat_All( arg_item, TRUE );
        }
        /* Check if we have to analyse this item */
        else if ( ( course_of_action.do_nopre
                    || Is_Format_Item( arg_item->type ) )
                  && !Is_Preformatted_Item( arg_item->type ) )
        {
            /* analyse item */
            int                 indent = Analyse_Indentation( arg_item );

            Analyse_List( arg_item, indent );
            Analyse_Preformatted( arg_item, indent );
            Analyse_Paragraphs( arg_item );
        }
        /* If none of above, preformat item */
        else
        {
            /* Preformat it */
            Preformat_All( arg_item, FALSE );
        }
    }
    /* Item is empty */
    else
    {
        /* Do nothing */
    }
}

/*****/



static int Trim_Empty_Item_Begin_Lines(
    struct RB_header *arg_header,
    struct RB_Item *arg_item,
    int current_index )
{

    char               *c;

    if ( Works_Like_SourceItem( arg_item->type ) )
    {
        /* We skip the first line after the source item, if
         * it an remark end marker -- such as '*)'
         */
        c = arg_header->lines[current_index].line;
        if ( RB_Is_Remark_End_Marker( c ) )
        {
            c = RB_Skip_Remark_End_Marker( c );
            c = RB_Skip_Whitespace( c );
            if ( *c != '\0' )
            {
                c = arg_header->lines[current_index].line;
                RB_Warning( "text following a remark end marker:\n%s\n", c );
            }
            ++current_index;
        }
    }

    if ( current_index > arg_item->end_index )
    {
        /* item is empty */
    }
    else
    {
        do
        {
            c = arg_header->lines[current_index].line;
            c = RB_Skip_Whitespace( c );
            if ( RB_Has_Remark_Marker( c ) )
            {
                c = RB_Skip_Remark_Marker( c );
            }
            c = RB_Skip_Whitespace( c );
            if ( *c == '\0' )
            {
                ++current_index;
            }
        }
        while ( ( *c == '\0' ) && ( current_index < arg_item->end_index ) );
    }

    return current_index;
}



static int Trim_Empty_Item_End_Lines(
    struct RB_header *arg_header,
    struct RB_Item *arg_item,
    int current_index )
{
    char               *c;

    if ( Works_Like_SourceItem( arg_item->type ) )
    {
        c = arg_header->lines[current_index].line;
        if ( RB_Is_Remark_Begin_Marker( c ) )
        {
            c = RB_Skip_Remark_Begin_Marker( c );
            c = RB_Skip_Whitespace( c );
            if ( *c != '\0' )
            {
                c = arg_header->lines[current_index].line;
                RB_Warning( "text following a remark begin marker:\n%s\n",
                            c );
            }
            --current_index;
        }
    }

    do
    {
        c = arg_header->lines[current_index].line;
        c = RB_Skip_Whitespace( c );
        if ( RB_Has_Remark_Marker( c ) )
        {
            c = RB_Skip_Remark_Marker( c );
        }
        c = RB_Skip_Whitespace( c );
        if ( *c == '\0' )
        {
            --current_index;
        }
    }
    while ( ( *c == '\0' ) && ( current_index > arg_item->begin_index ) );

    return current_index;
}



static void Trim_Empty_Item_Lines(
    struct RB_header *arg_header,
    struct RB_Item *arg_item )
{
    arg_item->no_lines = arg_item->end_index - arg_item->begin_index + 1;
    if ( arg_item->no_lines <= 1 )
    {
        /* item is empty */
        arg_item->no_lines = 0;
    }
    else
    {
        int                 current_index;

        /* trim all empty lines at the begin of an item */

        /* we skip the first line because that contains the item name.
         */
        current_index = arg_item->begin_index + 1;
        current_index =
            Trim_Empty_Item_Begin_Lines( arg_header, arg_item,
                                         current_index );

        /* Is there anything left? */
        if ( current_index <= arg_item->end_index )
        {
            arg_item->begin_index = current_index;

            /* trim all the empty lines at the end of an item */
            current_index = arg_item->end_index;
            current_index =
                Trim_Empty_Item_End_Lines( arg_header, arg_item,
                                           current_index );
            if ( current_index >= arg_item->begin_index )
            {
                arg_item->end_index = current_index;
                arg_item->no_lines =
                    arg_item->end_index - arg_item->begin_index + 1;
            }
            else
            {
                /* item is empty */
                arg_item->no_lines = 0;
            }
        }
        else
        {
            /* item is empty */
            arg_item->no_lines = 0;
        }
    }
}




/* TODO This routine is way too long */

static void Copy_Lines_To_Item(
    struct RB_header *arg_header,
    struct RB_Item *arg_item )
{
    Trim_Empty_Item_Lines( arg_header, arg_item );

    if ( arg_item->no_lines > 0 )
    {
        int                 i = 0;
        int                 j = 0;
        struct RB_Item_Line *itemline = NULL;
        int                 tool_active = 0;    /* Shows wether we are inside a tool body */

        /* Allocate enough memory for all the lines, plus one
         * extra line
         */
        ++arg_item->no_lines;
        arg_item->lines =
            calloc( arg_item->no_lines, sizeof( struct RB_Item_Line * ) );
        if ( !arg_item->lines )
        {
            RB_Panic( "Out of memory! %s\n", "Copy_Lines_To_Item" );
        }

        /* And create an RB_Item_Line for each of them, and add
         * those to the RB_Item
         */
        for ( i = 0; i < arg_item->no_lines - 1; ++i )
        {
            char               *c =
                arg_header->lines[arg_item->begin_index + i].line;
            /* TODO should be a Create_ItemLine() */
            itemline = malloc( sizeof( struct RB_Item_Line ) );
            if ( !itemline )
            {
                RB_Panic( "Out of memory! %s (2)\n", "Copy_Lines_To_Item" );
            }

            c = ExpandTab( c );
            c = RB_Skip_Whitespace( c );
            /* Lines with remark marker */
            if ( RB_Has_Remark_Marker( c )
                 && !Works_Like_SourceItem( arg_item->type ) )
            {
                char               *c2, *c3;
                int                 pipe_mode;
                enum ItemLineKind   item_kind;

                c = RB_Skip_Remark_Marker( c );
                c2 = RB_Skip_Whitespace( c );
                if ( *c2 )
                {
                    /* Check wether a tool starts or ends */
                    if ( ( c3 = Is_Tool( c2, &item_kind, &tool_active ) ) )
                    {
                        itemline->kind = item_kind;
                        c = c3;
                    }
                    /* If we have an active tool, copy the body lines
                     */
                    else if ( tool_active )
                    {
                        itemline->kind = ITEM_LINE_TOOL_BODY;
                        c++;    /* Skip space after the remark marker */
                    }
                    /* Check for pipes */
                    else if ( ( c3 = Is_Pipe_Marker( c2, &pipe_mode ) ) )
                    {
                        itemline->kind = ITEM_LINE_PIPE;
                        itemline->pipe_mode = pipe_mode;
                        c = c3;
                    }
                    /* Plain Items ... */
                    else
                    {
                        itemline->kind = ITEM_LINE_PLAIN;
                    }
                }
                /* Empty lines with remark markers and active tool */
                else if ( tool_active )
                {
                    itemline->kind = ITEM_LINE_TOOL_BODY;
                }
                /* Plain empty lines with remark markers... */
                else
                {
                    itemline->kind = ITEM_LINE_PLAIN;
                }
            }
            else
            {
                itemline->kind = ITEM_LINE_RAW;
                /* The is raw code, so we do not want to have the
                 * whitespace stripped of
                 */
                c = arg_header->lines[arg_item->begin_index + i].line;
                c = ExpandTab( c );
            }

            /* Copy source like items */
            if ( ( !Works_Like_SourceItem( arg_item->type ) &&
                   ( itemline->kind != ITEM_LINE_RAW ) ) ||
                 Works_Like_SourceItem( arg_item->type ) )
            {
                /* Copy item line */
                itemline->line = RB_StrDup( c );
                /* Copy original source line number */
                itemline->line_number =
                    arg_header->lines[arg_item->begin_index + i].line_number;
                /* Update the maximum source line number */
                if ( itemline->line_number > arg_item->max_line_number )
                {
                    arg_item->max_line_number = itemline->line_number;
                }
                itemline->format = 0;
                arg_item->lines[j] = itemline;
                ++j;
            }
            else
            {
                /* We dump the RAW item lines if we are not in a
                 * source item.
                 */
                free( itemline );
            }
        }

        if ( j > 0 )
        {
            /* And one empty line to mark the end of an item and
             * to be able to store some additional formatting actions
             */
            itemline = malloc( sizeof( struct RB_Item_Line ) );
            if ( !itemline )
            {
                RB_Panic( "Out of memory! %s (3)\n", "Copy_Lines_To_Item" );
            }

            itemline->kind = ITEM_LINE_END;
            itemline->line = RB_StrDup( "" );
            itemline->format = 0;
            arg_item->lines[j] = itemline;

            /* Store the real number of lines we copied */
            assert( arg_item->no_lines >= ( j + 1 ) );
            arg_item->no_lines = j + 1;
        }
        else
        {
            arg_item->no_lines = 0;
            free( arg_item->lines );
            arg_item->lines = NULL;
        }
    }
    else
    {
        arg_item->no_lines = 0;
        arg_item->lines = NULL;
    }
}

/*RZ*/
static void Add_Lines_To_Item(
    struct RB_Item *arg_item,
    char *c )
{
  struct RB_Item_Line *itemline = NULL;

  if (arg_item->no_lines == 0) {
    arg_item->lines = malloc( sizeof( struct RB_Item_Line * ) );
    itemline = malloc( sizeof( struct RB_Item_Line ) );
    if ( !itemline || !arg_item->lines) {
      RB_Panic( "Out of memory! %s (2)\n", "Add_Lines_To_Item" );
    }
    itemline->line = RB_StrDup( "" );
    itemline->kind = ITEM_LINE_END;
    itemline->format = 0;
    arg_item->lines[0] = itemline;
    arg_item->no_lines = 1;
  }
  arg_item->no_lines++;
  arg_item->lines = realloc( arg_item->lines, arg_item->no_lines * sizeof( struct RB_Item_Line * ) );
  itemline = malloc( sizeof( struct RB_Item_Line ) );
  if ( !itemline || !arg_item->lines) {
    RB_Panic( "Out of memory! %s (2)\n", "Add_Lines_To_Item" );
  }
  itemline->line = RB_StrDup( c );
  itemline->kind = ITEM_LINE_PLAIN;
  itemline->format = 0;
  if ( arg_item->no_lines > 1 ) {
    arg_item->lines[arg_item->no_lines-1] = arg_item->lines[arg_item->no_lines-2];
  }
  arg_item->lines[arg_item->no_lines-2] = itemline;
}

/****f* Analyser/RB_Analyse_Items
 * FUNCTION
 *   Locate the items in the header and create RB_Item structures for
 *   them.
 * SYNPOPSIS
 */
static int Analyse_Items(
    struct RB_header *arg_header )
/*
 * SOURCE
 */
{
    int                 line_nr;
    enum ItemType       item_type = NO_ITEM;
    struct RB_Item     *new_item;
    struct RB_Item     *cur_item;
    /*RZ variables for source comment adding */
    int isSource=0;
    struct RB_Item *use_item = NULL;
    unsigned int marker;
    char *cur_mchar = NULL;
    char *cur_char = NULL;
    int length;

    RB_Item_Lock_Reset(  );

    /* find the first item */
    for ( line_nr = 0; line_nr < arg_header->no_lines; ++line_nr )
    {
        item_type = RB_Is_ItemName( arg_header->lines[line_nr].line );
        if ( item_type != NO_ITEM )
        {
            break;
        }
    }

    /* and all the others */
    while ( ( item_type != NO_ITEM ) && ( line_nr < arg_header->no_lines ) )
    {
        new_item = RB_Create_Item( item_type );
        new_item->begin_index = line_nr;
        /*RZ add source comments to this item */
        if (use_source_comments) {
            if (strcmp(configuration.items.names[item_type],use_source_comments)==0) {
                use_item = new_item;
            } else {
                isSource = !strcmp(configuration.items.names[item_type],"SOURCE");
            }
        }
        /* Add the item to the end of the list of items. */
        if ( arg_header->items )
        {
            for ( cur_item = arg_header->items; cur_item->next;
                    cur_item = cur_item->next )
            {
                /* Empty */
            }
            cur_item->next = new_item;
        }
        else
        {
            arg_header->items = new_item;
        }
        /* Find the next item */
        for ( ++line_nr; line_nr < arg_header->no_lines; ++line_nr )
        {
            /*RZ add comment line to item */
            if (isSource) {
                cur_char = RB_Skip_Whitespace (arg_header->lines[line_nr].line);
                for (marker = 0; marker < configuration.source_line_comments.number;
                        marker++) {
                    cur_mchar = configuration.source_line_comments.names[marker];
                    length = strlen(cur_mchar);
                    if (strncmp(cur_char,cur_mchar,length) == 0) {
                        /* create item if not already existing */
                        if (use_item == NULL) {
                            item_type = RB_Get_Item_Type (use_source_comments);
                            if ( item_type != NO_ITEM ) {
                                use_item = RB_Create_Item( item_type );
                                use_item->no_lines = 0;
                                /* Add the item to the end of the list of items. */
                                if ( arg_header->items ) {
                                    for ( cur_item = arg_header->items; cur_item->next;
                                            cur_item = cur_item->next ) {
                                        /* Empty */
                                    }
                                    cur_item->next = use_item;
                                } else {
                                    arg_header->items = use_item;
                                }
                            }
                        }
                        Add_Lines_To_Item( use_item, &cur_char[length]);
                    }
                }
            }
            item_type = RB_Is_ItemName( arg_header->lines[line_nr].line );
            if ( item_type != NO_ITEM )
            {
                break;
            }
        }

        /* This points to the last line in the item */
        new_item->end_index = line_nr - 1;

        assert( new_item->end_index >= new_item->begin_index );

        /* Now analyse and copy the lines */
        Copy_Lines_To_Item( arg_header, new_item );
        Analyse_Item_Format( new_item );
        /* Handy for debugging wiki formatting 
         *   Dump_Item( new_item );
         */
    }
    /*RZ*/
    if (use_item) {
        Analyse_Item_Format( use_item );
    }
    return 0;
}

/******/



/****f* Analyser/ToBeAdded
 * FUNCTION
 *   Test whether or not a header needs to be added to the
 *   list of headers. This implements the options 
 *      --internal 
 *   and
 *      --internalonly
 * SYNPOPSIS
 */
static int ToBeAdded(
    struct RB_Document *document,
    struct RB_header *header )
/*
 * INPUTS
 *   o document  -- a document (to determine the options)
 *   o header    -- a header
 * RESULT
 *   TRUE  -- Add header
 *   FALSE -- Don't add header
 * SOURCE
 */
{
    int                 add = FALSE;

    if ( header->is_internal )
    {
        if ( ( document->actions.do_include_internal ) ||
             ( document->actions.do_internal_only ) )
        {
            add = TRUE;
        }
        else
        {
            add = FALSE;
        }
    }
    else
    {
        if ( document->actions.do_internal_only )
        {
            add = FALSE;
        }
        else
        {
            add = TRUE;
        }
    }
    return add;
}

/******/



/****f* Analyser/Grab_Header
 * FUNCTION
 *   Grab a header from a source file, that is scan a source file
 *   until the start of a header is found.  Then search for the end
 *   of a header and store all the lines in between.
 * SYNPOPSIS
 */
static struct RB_header *Grab_Header(
    FILE *sourcehandle,
    struct RB_Document *arg_document )
/*
 * INPUTS
 *   o sourcehandle -- an opened source file.
 * OUTPUT
 *   o sourcehandle -- will point to the line following the end marker.
 * RESULT
 *   0 if no header was found, or a pointer to a new header otherwise.
 * SOURCE
 */
{
    struct RB_header   *new_header = NULL;
    int                 is_internal = 0;
    struct RB_HeaderType *header_type = NULL;
    int                 good_header = FALSE;
    int                 reuse = FALSE;

    do
    {
        good_header = FALSE;
        header_type = RB_Find_Marker( sourcehandle, &is_internal, reuse );
        reuse = FALSE;
        if ( header_type )
        {
            struct RB_header   *duplicate_header = NULL;
            long                previous_line = 0;

            new_header = RB_Alloc_Header(  );
            new_header->htype = header_type;
            new_header->is_internal = is_internal;

            if ( Find_Header_Name( sourcehandle, new_header ) )
            {
                new_header->line_number = line_number;
                RB_Say( "found header [line %5d]: \"%s\"\n", SAY_DEBUG,
                        line_number, new_header->name );
                duplicate_header =
                    RB_Document_Check_For_Duplicate( arg_document,
                                                     new_header );
                if ( duplicate_header )
                {
                    /* Duplicate headers do not crash the program so
                     * we accept them.  But we do warn the user.
                     */
                    RB_Warning
                        ( "A header with the name \"%s\" already exists.\n  See %s(%d)\n",
                          new_header->name,
                          Get_Fullname( duplicate_header->owner->filename ),
                          duplicate_header->line_number );
                }

                if ( ( new_header->function_name =
                       Function_Name( new_header->name ) ) == NULL )
                {
                    RB_Warning( "Can't determine the \"function\" name.\n" );
                    RB_Free_Header( new_header );
                    new_header = NULL;
                }
                else
                {
                    if ( ( new_header->module_name =
                           Module_Name( new_header->name ) ) == NULL )
                    {
                        RB_Warning
                            ( "Can't determine the \"module\" name.\n" );
                        RB_Free_Header( new_header );
                        new_header = NULL;
                    }
                    else
                    {
                        previous_line = line_number;
                        if ( Find_End_Marker( sourcehandle, new_header ) ==
                             0 )
                        {
                            RB_Warning
                                ( "found header on line %d with name \"%s\"\n"
                                  "  but I can't find the end marker\n",
                                  previous_line, new_header->name );
                            /* Reuse the current line while finding the next
                             * Marking using RB_Find_Marker()
                             */
                            reuse = TRUE;
                            RB_Free_Header( new_header );
                            new_header = NULL;
                        }
                        else
                        {
                            RB_Say( "found end header [line %5d]:\n",
                                    SAY_DEBUG, line_number );
                            /* Good header found, we can stop */
                            good_header = TRUE;
                        }
                    }
                }
            }
            else
            {
                RB_Warning( "found header marker but no name\n" );
                RB_Free_Header( new_header );
                new_header = NULL;
            }
        }
        else
        {
            /* end of the file */
            good_header = TRUE;
        }
    }
    while ( !good_header );
    return new_header;
}

/*******/



/****f* Analyser/Module_Name
 * FUNCTION
 *   Get the module name from the header name.  The header name will be
 *   something like
 *
 *     module/functionname.
 *
 * SYNPOPSIS
 */
static char        *Module_Name(
    char *header_name )
/*
 * INPUTS
 *   o header_name -- a pointer to a nul terminated string.
 * RESULT
 *   Pointer to the modulename.  You're responsible for freeing it.
 * SEE ALSO
 *   Function_Name()
 * SOURCE
 */
{
    char               *cur_char;
    char                c;
    char               *name = NULL;

    assert( header_name );

    for ( cur_char = header_name; *cur_char && *cur_char != '/'; ++cur_char );
    if ( *cur_char )
    {
        c = *cur_char;
        *cur_char = '\0';
        name = RB_StrDup( header_name );
        *cur_char = c;
    }
    return name;
}

/******/



/****f* Analyser/Function_Name
 * FUNCTION
 *   A header name is consists of two parts. The module name and
 *   the function name. This returns a pointer to the function name.
 *   The name "function name" is a bit obsolete. It is really the name
 *   of any of objects that can be documented; classes, methods,
 *   variables, functions, projects, etc.
 * SYNOPSIS
 */
static char        *Function_Name(
    char *header_name )
/*
 * SOURCE
 */
{
    char               *cur_char;
    char               *name;

    name = NULL;
    if ( ( cur_char = header_name ) != NULL )
    {
        for ( ; *cur_char != '\0'; ++cur_char )
        {
            if ( '/' == *cur_char )
            {
                ++cur_char;
                if ( *cur_char )
                {
                    name = cur_char;
                    break;
                }
            }
        }
    }
    if ( name )
    {
        return RB_StrDup( name );
    }
    else
    {
        return ( name );
    }
}

/*** Function_Name ***/


/****f* Analyser/RB_Find_Marker
 * NAME
 *   RB_Find_Marker -- Search for header marker in document.
 * FUNCTION
 *   Read document file line by line, and search each line for 
 *   any of the headers defined in the array  header_markers (OR
 *   if using the -rh switch, robo_head)
 * SYNOPSIS
 */
static struct RB_HeaderType *RB_Find_Marker(
    FILE *document,
    int *is_internal,
    int reuse_previous_line )
/*
 * INPUTS
 *   document - pointer to the file to be searched.
 *   the gobal buffer line_buffer.
 * OUTPUT
 *   o document will point to the line after the line with 
 *     the header marker.
 *   o is_internal will be TRUE if the header is an internal
 *     header.
 * RESULT
 *   o header type
 * BUGS
 *   Bad use of feof(), fgets().
 * SEE ALSO
 *   Find_End_Marker
 * SOURCE
 */
{
    int                 found;
    char               *cur_char;
    struct RB_HeaderType *header_type = 0;

    cur_char = NULL;
    found = FALSE;
    while ( !feof( document ) && !found )
    {
        if ( reuse_previous_line )
        {
            /* reuse line in the line_buffer */
            reuse_previous_line = FALSE;
        }
        else
        {
            RB_FreeLineBuffer(  );
            myLine = RB_ReadWholeLine( document, line_buffer, &readChars );
        }
        if ( !feof( document ) )
        {
            line_number++;
            found = RB_Is_Begin_Marker( myLine, &cur_char );
            if ( found )
            {
                header_type = AnalyseHeaderType( &cur_char, is_internal );
                RB_Say( "found header marker of type %s\n", SAY_DEBUG,
                        header_type->indexName );
            }
        }
    }

    return header_type;
}

/******** END RB_Find_Marker ******/


/****f* Analyser/AnalyseHeaderType
 * FUNCTION
 *   Determine the type of the header.
 * SYNOPSIS
 */
struct RB_HeaderType *AnalyseHeaderType(
    char **cur_char,
    int *is_internal )
/*
 * INPUTS
 *   o cur_char -- pointer to the header type character
 * OUTPUT
 *   o is_internal -- indicates if it is an internal header or not.*
 *   o cur_char -- points to the header type character
 * RESULT
 *   o pointer to a RB_HeaderType
 * SOURCE
 */
{
    struct RB_HeaderType *headertype = 0;

    *is_internal = RB_IsInternalHeader( **cur_char );

    if ( *is_internal )
    {
        /* Skip the character */
        ++( *cur_char );
    }
    headertype = RB_FindHeaderType( **cur_char );
    if ( !headertype )
    {
        RB_Panic( "Undefined headertype (%c)\n", **cur_char );
    }

    return headertype;
}

/*******/



/****f* Analyser/Find_End_Marker
 * FUNCTION
 *   Scan and store all lines from a source file until
 *   an end marker is found.
 * SYNOPSIS
 */
static int Find_End_Marker(
    FILE *document,
    struct RB_header *new_header )
/*
 * INPUTS
 *   o document -- a pointer to an opened source file.
 * OUTPUT
 *   o new_header -- the lines of source code will be added
 *                   here.
 * RESULT
 *   o TRUE  -- an end marker was found.
 *   o FALSE -- no end marker was found while scanning the
 *              source file.
 * SOURCE
 */
{
    int                 found = FALSE;
    unsigned int        no_lines = 0;
    unsigned int        max_no_lines = 10;
    struct RB_header_lines *lines = NULL;
    char               *dummy;

    lines = malloc( max_no_lines * sizeof( struct RB_header_lines ) );
    if ( lines == NULL )
    {
        RB_Panic( "Out of memory! %s()\n", "Find_End_Marker" );
    }

    while ( !feof( document ) )
    {
        RB_FreeLineBuffer(  );
        myLine = RB_ReadWholeLine( document, line_buffer, &readChars );
        ++line_number;          /* global linecounter, koessi */
        if ( RB_Is_Begin_Marker( myLine, &dummy ) )
        {
            /* Bad... found a begin marker but was expecting to
               find an end marker.  Panic... */
            found = FALSE;
            return found;
        }
        else if ( RB_Is_End_Marker( myLine ) )
        {
            RB_Say( "Found end marker \"%s\"", SAY_DEBUG, myLine );
            found = TRUE;
            break;
        }
        else
        {
            unsigned int        n;
            char               *line;

            line = RB_StrDup( myLine );
            n = strlen( line );
            assert( n > 0 );
            assert( line[n - 1] == '\n' );
            /* Strip CR */
            line[n - 1] = '\0';
            /* Copy the line itself */
            lines[no_lines].line = line;
            /* and also save the original source line number */
            lines[no_lines].line_number = line_number;
            ++no_lines;
            if ( no_lines == max_no_lines )
            {
                max_no_lines *= 2;
                lines =
                    realloc( lines,
                             max_no_lines *
                             sizeof( struct RB_header_lines ) );

                if ( lines == NULL )
                {
                    RB_Panic( "Out of memory! %s()\n", "Find_End_Marker" );
                }
            }
        }
    }

    new_header->no_lines = no_lines;
    new_header->lines = lines;

    return found;
}


/******/


/* TODO Documentation */
static void Remove_Trailing_Asterics(
    char *line )
{
    int                 i = strlen( line ) - 1;

    for ( ; ( i > 0 ) && utf8_isspace( line[i] ); i-- )
    {
        /* Empty */
    }
    for ( ; ( i > 0 ) && ( line[i] == '*' ); i-- )
    {
        line[i] = ' ';
    }
}


/****if* Utilities/RB_WordWithSpacesLen
 * SYNOPSIS
 */
static int RB_WordWithSpacesLen(
    char *str )
/*
 * FUNCTION
 *   get the amount of bytes until next separator character or ignore character
 *   or end of line
 * INPUTS
 *   char *str      -- the line
 * RESULT
 *   int -- length of the next word or 0
 * SEE ALSO
 *   RB_Find_Header_Name()
 * SOURCE
 */
{
    int                 len;
    char                c;

    for ( len = 0; ( ( c = *str ) != '\0' ) && ( c != '\n' ); ++str, ++len )
    {
        /* Look for header truncating characters */
        if ( Find_Parameter_Char( &( configuration.header_separate_chars ),
                                  c ) != NULL
             ||
             Find_Parameter_Char( &( configuration.header_ignore_chars ),
                                  c ) != NULL )
        {
            /* and exit loop if any found */
            break;
        }
    }
    /* Don't count any of the trailing spaces. */
    if ( len )
    {
        --str;
        for ( ; utf8_isspace( *str ); --len, --str )
        {
            /* empty */
        }
    }
    return ( len );
}

/*** RB_WordWithSpacesLen ***/


/* TODO Documentation */
static int Find_Header_Name(
    FILE *fh,
    struct RB_header *hdr )
{
    char               *cur_char = myLine;
    char              **names = NULL;
    int                 num = 0;

    Remove_Trailing_Asterics( cur_char );
    skip_while( *cur_char != '*' );
    skip_while( !utf8_isspace( *cur_char ) );
    skip_while( utf8_isspace( *cur_char ) );
    while ( *cur_char )
    {
        int                 length = RB_WordWithSpacesLen( cur_char );

        if ( length == 0 )
            break;
        names = realloc( names, ( ++num ) * sizeof *names );

        if ( !names )
        {
            RB_Panic( "Out of memory! %s()\n", "Find_Header_Name" );
        }

        names[num - 1] = RB_StrDupLen( cur_char, length );
        /* printf("%c adding name = %s\n", num > 1 ? ' ' : '*', names[ num - 1 ] ); */
        cur_char += length;
        if ( Find_Parameter_Char( &( configuration.header_separate_chars ),
                                  *cur_char ) )
        {
            for ( cur_char++; utf8_isspace( *cur_char ); cur_char++ );
            /* End of line reach, but comma encountered, more headernames follow on next line */
            if ( *cur_char == 0 )
            {
                /* Skip comment */
                RB_FreeLineBuffer(  );
                myLine = RB_ReadWholeLine( fh, line_buffer, &readChars );
                line_number++;
                for ( cur_char = myLine;
                      *cur_char && !utf8_isalpha( *cur_char ); cur_char++ );
            }
        }
    }
    hdr->names = names;
    hdr->no_names = num;
    hdr->name = num ? names[0] : NULL;
    return num;
}

/*****  Find_Header_Name  *****/
