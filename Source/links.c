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

/****h* ROBODoc/Links
 * FUNCTION
 *   This module contains functions to manipulate links.
 *   Links are derived from headers.  They are used to create
 *   links in the documentation between a word and the part of
 *   the documentation that explains something about that word.
 *   (For instance a function name or variable name).
 *   In addition to the links derived from the headers links are
 *   also derived from the names of all the sourcefiles.
 * MODIFICATION HISTORY
 *   ????-??-??   Frans Slothouber  V1.0 
 *   2003-02-03   Frans Slothouber  Refactoring
 *******
 * $Header: /cvsroot/robodoc/robo/Source/links.c,v 1.43 2007/07/10 19:13:52 gumpu Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "globals.h"
#include "robodoc.h"
#include "headers.h"
#include "util.h"
#include "links.h"
#include "document.h"
#include "part.h"
#include "file.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* TODO Documentation */
unsigned int        link_index_size = 0;
struct RB_link    **link_index = NULL;
struct RB_link    **case_sensitive_link_index = NULL;


/* Local functions */

static struct RB_link *RB_Alloc_Link( char *label_name, char *object_name,
                                      char *file_name );

/* TODO Documentation */
int link_cmp( void *l1, void *l2 )
{
    struct RB_link *link1 = l1;
    struct RB_link *link2 = l2;

    return RB_Str_Case_Cmp( link1->object_name, link2->object_name );
}


int case_sensitive_link_cmp( void *l1, void *l2 )
{
    struct RB_link *link1 = l1;
    struct RB_link *link2 = l2;

    return strcmp( link1->object_name, link2->object_name );
}

char * function_name( char * full_name )
{
    char * name = strchr( full_name, '/' );

    if( name ) return name + 1;
    else return full_name;
}

/****f* Links/RB_CollectLinks
 * FUNCTION
 *   Convert header information into link information.
 *      RB_header -> RB_link conversion
 * SYNOPSIS
 */
void
RB_CollectLinks( struct RB_Document *document, 
                 struct RB_header **headers,
                 unsigned long count )
/*
 * INPUTS
 *   * document -- 
 *   * headers  -- the array with headers.
 *   * count    -- number of headers in the array
 * OUTPUT
 *   * link_index -- an array with links
 *   * link_index_size -- the number of links in the array.
 * SOURCE
 */

{
    unsigned long        i, j;
    int  k;
    struct RB_Part     *i_part;

    for ( i = j = 0; i < count; ++i ) 
    {
        j += headers[i]->no_names - 1;
    }

    link_index_size = count + j;

    if ( ( document->actions.do_multidoc ) &&
            ! ( document->actions.do_one_file_per_header )
       )
    {
        for ( i_part = document->parts; i_part; i_part = i_part->next )
        {
            if ( i_part->headers ) 
            {
                link_index_size++;
            }
        }
    }

    link_index =
        ( struct RB_link ** ) calloc( link_index_size,
                                      sizeof( struct RB_link ** ) );
    case_sensitive_link_index =
        ( struct RB_link ** ) calloc( link_index_size,
                                      sizeof( struct RB_link ** ) );

    for ( i = j = 0; i < count; ++i )
    {
        struct RB_link     *link;
        struct RB_header   *header;

        header = headers[i];
        assert( header->unique_name );
        assert( header->file_name );
        for( k = 0; k < header->no_names; j++, k++ )
        {
            link = RB_Alloc_Link( header->unique_name, function_name(header->names[k]),
                                  header->file_name );
            link->htype = header->htype;
            link->is_internal = header->is_internal;
            link_index[j] = link;
            case_sensitive_link_index[j] = link;
        }
    }

    /* If we are in multidoc mode, we also create links
     * for all the source files.
     */

    if ( ( document->actions.do_multidoc ) &&
            /* but not for one file per header multidocs */
       ! ( document->actions.do_one_file_per_header )
       )
    {
        for ( i_part = document->parts; i_part; i_part = i_part->next )
        {
            if ( i_part->headers ) 
            {
                struct RB_link     *link;

                link =
                    RB_Alloc_Link( "robo_top_of_doc", i_part->filename->name,
                            RB_Get_FullDocname( i_part->filename ) );
                i_part->filename->link = link;
                link->htype = RB_FindHeaderType( HT_SOURCEHEADERTYPE );
                link_index[j] = link;
                case_sensitive_link_index[j] = link;
                ++j;
            }
            else
            {
                i_part->filename->link = NULL;
            }
        }
    }

    /* Sort all the links so we can use a binary search */
    RB_QuickSort( (void **)link_index, 0, link_index_size - 1, link_cmp );
    RB_QuickSort( (void **)case_sensitive_link_index, 0, link_index_size - 1, case_sensitive_link_cmp );
}

/*****/


/****f* Links/RB_Free_Links
 * FUNCTION
 *   Deallocate all the memory used to store the links.
 * SYNOPSIS
 */
void RB_Free_Links( void )
/*
 * INPUTS
 *   o link_index_size
 *   o link_index[]
 * BUGS
 *   Should use RB_Free_Link instead of doing
 *   everything by it self.
 * SOURCE
 */
{
    struct RB_link     *cur_link;
    unsigned int        i;

    for ( i = 0; i < link_index_size; ++i )
    {
        cur_link = link_index[i];
        free( cur_link->object_name );
        free( cur_link->label_name );
        free( cur_link->file_name );
        free( cur_link );
    }
    free( link_index );
}

/*******/

/* TODO Documentation */

int RB_Number_Of_Links( struct RB_HeaderType* header_type, char* file_name, int internal )
{
    struct RB_link     *cur_link;
    int                 n = 0;
    unsigned int        i;

    for ( i = 0; i < link_index_size; ++i )
    {
        cur_link = link_index[i];
        if ( RB_CompareHeaderTypes( cur_link->htype, header_type ) &&
             ( ( cur_link->is_internal && internal ) ||
               ( !cur_link->is_internal && !internal ) ) )
        {
            if ( file_name )
            {
                if ( strcmp( file_name, cur_link->file_name ) == 0 )
                {
                    n++;
                }
            }
            else
            {
                n++;
            }
        }
    }
    return n;
}

/****f* Links/Find_Link [3.0h]
 * NAME
 *   Find_Link -- try to match word with a link
 * FUNCTION
 *   Searches for the given word in the list of links and
 *   headers.  There are three passes (or four, when the C option
 *   is selected). Each pass uses a different definition of "word":
 *   o In the first pass it is any thing that ends with a 'space', a '.' 
 *     or a ','.
 *   o In the second pass it is any string that consists of alpha
 *     numerics, '_', ':', '.', or '-'.  
 *   o In the third pass (for C) it is any string that consists 
 *     of alpha numerics or '_'.
 * SYNOPSIS
 */

int
Find_Link( char *word_begin, 
           char **object_name, 
           char **label_name,
           char **file_name )
/*
 * INPUTS
 *   o word_begin  - pointer to a word (a string).
 *   o object_name  - pointer to a pointer to a string
 *   o file_name   - pointer to a pointer to a string
 * SIDE EFFECTS
 *   label_name & file_name are modified
 * RESULT
 *   o object_name   -- points to the object if a match was found,
 *                      NULL otherwise.
 *   o file_name     -- points to the file name if a match was found,
 *                      NULL otherwise.
 *   o label_name    -- points to the labelname if a match was found,
 *   o TRUE          -- a match was found.
 *   o FALSE         -- no match was found.
 * NOTES
 *   This is a rather sensitive algorithm. Don't mess with it
 *   too much.
 * SOURCE
 */

{
    char               *cur_char = NULL, old_char;
    int                 low_index, high_index, cur_index, state, pass;
    unsigned int        length = 0;

    for ( pass = 0; pass < 3; pass++ )
    {
        switch ( pass )
        {
        case 0:
            {
                for ( cur_char = word_begin;
                      ( *cur_char == '_' ) || utf8_isalnum( *cur_char ) || utf8_ispunct( *cur_char );
                      cur_char++ );
                break;
            }
        case 1:
            {
                for ( cur_char = word_begin;
                      utf8_isalnum( *cur_char ) || ( *cur_char == '_' ) ||
                      ( *cur_char == '-' ) || ( *cur_char == '.' ) ||
                      ( *cur_char == ':' ); cur_char++ );
                break;
            }
        case 2:
            {
                for ( cur_char = word_begin;
                      utf8_isalnum( *cur_char ) || ( *cur_char == '_'); 
                      cur_char++ );
                break;
            }
        }

        if ( ( ( *( cur_char - 1 ) ) == ',' ) || ( ( *( cur_char - 1 ) ) == '.' ) )
        {
            cur_char--;
        }

        old_char = *cur_char;
        *cur_char = '\0';       /*
                                 * End the word with a '\0' 
                                 */
        if ( strlen( word_begin ) == length )
        {
            /* Do not test the same word over and over. If
             * the current string and the string of the previous
             * pass are the same length, they are the same. */

            /* RB_Say ("Skipping (pass %d) \"%s\"\n", SAY_INFO, pass, word_begin);  */
        }
        else
        {
            length = strlen( word_begin );
            /* RB_Say ("Testing (pass %d) \"%s\"\n", SAY_INFO, pass, word_begin); */
            /*
             * Search case sensitive for a link 
             */
            for ( cur_index = 0, low_index = 0, high_index =
                    link_index_size - 1; high_index >= low_index; )
            {
                cur_index = ( high_index - low_index ) / 2 + low_index;
                state = strcmp( word_begin, case_sensitive_link_index[cur_index]->object_name );
                if ( state < 0 )
                {
                    high_index = cur_index - 1;
                }
                else if ( state == 0 )
                {
                    *object_name = case_sensitive_link_index[cur_index]->object_name;
                    *label_name = case_sensitive_link_index[cur_index]->label_name;
                    *file_name = case_sensitive_link_index[cur_index]->file_name;
                    RB_Say( "linking \"%s\"->\"%s\" from \"%s\"\n", SAY_DEBUG,
                            word_begin, *object_name, *file_name );
                    *cur_char = old_char;
                    return ( TRUE );
                }
                else if ( state > 0 )
                {
                    low_index = cur_index + 1;
                }
            }

            /*
             * Search case insensitive for a link.
             * But only when the user asks for this.
             */
            if ( course_of_action.do_ignore_case_when_linking ) 
            {

                for ( cur_index = 0, low_index = 0, high_index =
                        link_index_size - 1; high_index >= low_index; )
                {
                    cur_index = ( high_index - low_index ) / 2 + low_index;
                    state = RB_Str_Case_Cmp( word_begin, link_index[cur_index]->object_name );
                    if ( state < 0 )
                    {
                        high_index = cur_index - 1;
                    }
                    else if ( state == 0 )
                    {
                        *object_name = link_index[cur_index]->object_name;
                        *label_name = link_index[cur_index]->label_name;
                        *file_name = link_index[cur_index]->file_name;
                        RB_Say( "linking \"%s\"->\"%s\" from \"%s\"\n", SAY_DEBUG,
                                word_begin, *object_name, *file_name );
                        *cur_char = old_char;
                        return ( TRUE );
                    }
                    else if ( state > 0 )
                    {
                        low_index = cur_index + 1;
                    }
                }
            }
        }
        *cur_char = old_char;
        *file_name = NULL;
        *object_name = NULL;
        *label_name = NULL;
    }
    return ( FALSE );
}

/*****/


/****f* Links/RB_Alloc_Link [2.01]
 * NAME
 *   RB_Alloc_Link              -- oop
 * FUNCTION
 *   allocate struct + strings
 * SYNOPSIS
 */
static struct RB_link *
RB_Alloc_Link( char *label_name, char *object_name, char *file_name )
/* 
 * INPUTS
 *   char *label_name -- strings to copy into the link
 *   char *file_name
 * RESULT
 *   struct RB_link *  -- ready-to-use
 * AUTHOR
 *   Koessi
 * SEE ALSO
 *   RB_StrDup(), RB_Free_Link()
 * SOURCE
 */

{
    struct RB_link     *new_link;

    assert( object_name );
    assert( label_name );
    assert( file_name );
    RB_Say( "Allocating a link (%s %s %s)\n", SAY_DEBUG, object_name, label_name, file_name );
    new_link = malloc( sizeof( struct RB_link ) );
    memset( new_link, 0, sizeof( struct RB_link ) );

    new_link->file_name = RB_StrDup( file_name );
    new_link->object_name = RB_StrDup( object_name );
    new_link->label_name = RB_StrDup( label_name );
    return ( new_link );
}

/*****/

/****f* Links/RB_Free_Link
 * NAME
 *   RB_Free_Link               -- oop
 * FUNCTION
 *   free struct + strings
 * SYNOPSIS
 */
void RB_Free_Link( struct RB_link *arg_link )
/*
 * INPUTS
 *   struct RB_link *link
 * AUTHOR
 *   Koessi
 * SEE ALSO
 *   RB_Alloc_Link(), RB_Close_The_Shop()
 * SOURCE
 */

{
    if ( arg_link )
    {
        if ( arg_link->label_name )
        {
            free( arg_link->label_name );
        }
        if ( arg_link->file_name )
        {
            free( arg_link->file_name );
        }
        free( arg_link );
    }
}

/******/

