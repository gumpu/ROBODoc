/* vi: ff=unix spell 
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


/****h* ROBODoc/Items
 * FUNCTION
 *   This module contains functions that deal with items.  The
 *   documentation consists of headers, and headers contains one of
 *   more items.  Each item has a name and a body.  All possible items
 *   are listed in configuration.items.  A uses can specify that
 *   certain items are not to be added to the documentation.  These
 *   items are listed in configuration.ignore_items.
 * AUTHOR
 *   Frans Slothouber
 *******
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "globals.h"
#include "robodoc.h"
#include "items.h"
#include "roboconfig.h"
#include "util.h"

/****v* Items/item_name_buffer
 * FUNCTION
 *   Stores the name of the last item that was found.
 * SOURCE
 */

#define MAX_ITEM_NAME_LENGTH 1024
char                item_name_buffer[MAX_ITEM_NAME_LENGTH];

/*****/

/* TODO Documentation */
char               *RB_Get_Item_Name(
    void )
{
    return item_name_buffer;
}

/****f* Items/RB_Create_Item
 *
 * SOURCE
 */

struct RB_Item     *RB_Create_Item(
    enum ItemType arg_item_type )
{
    struct RB_Item     *item = malloc( sizeof( struct RB_Item ) );

    assert( item );

    item->next = 0;
    item->type = arg_item_type;
    item->begin_index = 0;
    item->end_index = 0;
    item->max_line_number = 0;

    return item;
}

/*****/

/****f* Items/RB_Get_Item_Type [3.0b]
 * FUNCTION
 *   return the item_type represented by the given string.
 * SYNOPSIS
 *   int RB_Get_Item_Type( char *cmp_name )
 * INPUTS
 *   char *cmp_name -- item_name to evaluate
 * RESULT
 *   int            -- the right item_type or NO_ITEM
 * SOURCE
 */

int RB_Get_Item_Type(
    char *cmp_name )
{
    unsigned int        item_type;

    assert( configuration.items.number );
    for ( item_type = 0; item_type < configuration.items.number; ++item_type )
    {
        char               *item = configuration.items.names[item_type];

        /* Skip preformat mark */
        if ( *item == '-' )
            item++;
        if ( !strcmp( item, cmp_name ) )
        {
            return ( item_type );
        }
    }
    return ( NO_ITEM );
}

/*** RB_Get_Item_Type ***/



/****f* Items/RB_Is_ItemName
 * FUNCTION
 *   Is there an itemname in the line.  Ignores leading spaces and
 *   remark markers.
 * INPUTS
 *   line -- line to be searched.
 * RESULT
 *   The kind of item that was found or NO_ITEM if no item could be found.
 *   The name of the item will be stored in item_name_buffer.
 * NOTES
 *   We used to check for misspelled items names by testing if
 *   the item name buffer consists of only upper case characters.
 *   However checking for a misspelled item name this way results in
 *   many false positives. For instance many warnings are given for
 *   FORTRAN code as all the keywords are in uppercase.  We need to
 *   find a better method for this.
 * SOURCE
 */

enum ItemType RB_Is_ItemName(
    char *line )
{
    char               *cur_char = line;
    int                 i = 0;

    cur_char = RB_Skip_Whitespace( cur_char );
    if ( RB_Has_Remark_Marker( cur_char ) )
    {
        cur_char = RB_Skip_Remark_Marker( cur_char );
        cur_char = RB_Skip_Whitespace( cur_char );
        /* It there anything left? */
        if ( strlen( cur_char ) )
        {
            enum ItemType       item_type = NO_ITEM;

            /* Copy the name */
            strcpy( item_name_buffer, cur_char );
            /* remove any trailing spaces */
            for ( i = strlen( item_name_buffer ) - 1;
                  i >= 0 && utf8_isspace( item_name_buffer[i] ); --i )
            {
                item_name_buffer[i] = '\0';
            }
            /* No check and see if this is an item name */
            if ( strlen( item_name_buffer ) )
            {
                item_type = RB_Get_Item_Type( item_name_buffer );
#if 0                           /* Until we find a better method */
                if ( item_type == NO_ITEM )
                {
                    /* Check if it is a misspelled item name */
                    item_type = POSSIBLE_ITEM;
                    for ( i = 0; i < strlen( item_name_buffer ); ++i )
                    {
                        if ( !( utf8_isupper( item_name_buffer[i] ) ||
                                utf8_isspace( item_name_buffer[i] ) ) )
                        {
                            /* No it is not */
                            item_type = NO_ITEM;
                            break;
                        }
                    }
                }
#endif
            }
            return item_type;
        }
        else
        {
            return NO_ITEM;
        }
    }
    else
    {
        return NO_ITEM;
    }
}

/******/

/* TODO Documentation */
int Is_Ignore_Item(
    char *name )
{
    unsigned int        i;

    for ( i = 0; i < configuration.ignore_items.number; ++i )
    {
        if ( !strcmp( configuration.ignore_items.names[i], name ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}


/****f* HeaderTypes/Works_Like_SourceItem
 * FUNCTION
 *   Tells wether this item works similar to the
 *   source item, that is weather it copies it's
 *   content verbatim to the output document.
 * SYNPOPSIS
 */
int Works_Like_SourceItem(
    enum ItemType item_type )
/*
 * INPUTS
 *   item_type -- Type of item (also the index to the item name)
 * RESULT
 *   TRUE  -- Item works like a SOURCE item
 *   FALSE -- Item does NOT work like a SOURCE item
 * SOURCE
 */
{
    unsigned int        i;

    /* Check if it is a SOURCE item */
    if ( item_type == SOURCECODE_ITEM )
    {
        return TRUE;
    }

    /* Lookup if it works like a SOURCE item */
    for ( i = 0; i < configuration.source_items.number; ++i )
    {
        if ( !strcmp
             ( configuration.source_items.names[i],
               configuration.items.names[item_type] ) )
        {
            return TRUE;
        }
    }

    /* Neither SOURCE item, nor works like it */
    return FALSE;
}

/******/

/****f* HeaderTypes/Is_Preformatted_Item
 * FUNCTION
 *   Tells wether this item should be automatically preformatted in the
 *   output.
 * SYNPOPSIS
 */
int Is_Preformatted_Item(
    enum ItemType item_type )
/*
 * INPUTS
 *   item_type -- Type of item (also the index to the item name)
 * RESULT
 *   TRUE  -- Item should be automatically preformatted
 *   FALSE -- Item should NOT be automatically preformatted
 * SOURCE
 */
{
    unsigned int        i;

    /* Lookup if item should be preformatted */
    for ( i = 0; i < configuration.preformatted_items.number; ++i )
    {
        if ( !strcmp
             ( configuration.preformatted_items.names[i],
               configuration.items.names[item_type] ) )
        {
            /* Item name match, it sould be preformatted */
            return TRUE;
        }
    }

    /* Do not automatically preformat item */
    return FALSE;
}

/******/

/****f* HeaderTypes/Is_Format_Item
 * FUNCTION
 *   Tells wether this item should be formatted by the browser
 * SYNPOPSIS
 */
int Is_Format_Item(
    enum ItemType item_type )
/*
 * INPUTS
 *   item_type -- Type of item (also the index to the item name)
 * RESULT
 *   TRUE  -- Item should be formatted by the browser
 *   FALSE -- Item should be left alone
 * SOURCE
 */
{
    unsigned int        i;

    /* Lookup if item should be formatted by the browser */
    for ( i = 0; i < configuration.format_items.number; ++i )
    {
        if ( !strcmp
             ( configuration.format_items.names[i],
               configuration.items.names[item_type] ) )
        {
            /* Item name match, it sould be formatted by the browser */
            return TRUE;
        }
    }

    /* Leave item alone */
    return FALSE;
}

/******/
