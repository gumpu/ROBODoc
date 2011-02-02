#ifndef ROBODOC_ITEMS_H
#define ROBODOC_ITEMS_H

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


#include "robodoc.h"

/****t* Items/ItemTypes 
 * NAME 
 *   ItemTypes -- enumeration of item types
 * FUNCTION
 *   Defines a number of item types. There are two kind of items:
 *   * the SOURCE item which is always available,
 *   * and items defined by the user (or through the default items).
 * NOTES
 *   Never check an item type against SOURCECODE_ITEM directily!
 *   Use Works_Like_SourceItem() function instead.
 * SOURCE
 */

enum ItemType
{ POSSIBLE_ITEM = -2, NO_ITEM = -1, SOURCECODE_ITEM = 0, OTHER_ITEM };

/*****/

/* This should be an enum */
#define RBILA_BEGIN_PARAGRAPH  ( 1 <<  1 )
#define RBILA_END_PARAGRAPH    ( 1 <<  2 )
#define RBILA_BEGIN_LIST       ( 1 <<  3 )
#define RBILA_END_LIST         ( 1 <<  4 )
#define RBILA_BEGIN_LIST_ITEM  ( 1 <<  5 )
#define RBILA_END_LIST_ITEM    ( 1 <<  6 )
#define RBILA_BEGIN_PRE        ( 1 <<  7 )
#define RBILA_END_PRE          ( 1 <<  8 )
#define RBILA_BEGIN_SOURCE     ( 1 <<  9 )
#define RBILA_END_SOURCE       ( 1 << 10 )

/****s* Items/ItemLineKind
 * FUNCTION
 *   Holds the type of an item line
 * SOURCE
 */

enum ItemLineKind
{
    ITEM_LINE_RAW,              /* A line that does not start with a remark marker */
    ITEM_LINE_PLAIN,            /* A line that starts with a remark marker */
    ITEM_LINE_PIPE,             /* A line that starts with a remark marked and is
                                   followed by a pipe marker. */
    ITEM_LINE_END,              /* The last line of an item */

    ITEM_LINE_TOOL_START,       /* Start line of a tool item */
    ITEM_LINE_TOOL_BODY,        /* Body of a tool item */
    ITEM_LINE_TOOL_END,         /* End line of a tool item */
    ITEM_LINE_EXEC,             /* Exec item */
    ITEM_LINE_DOT_START,        /* Similar to TOOL_START but use DOT tool */
    ITEM_LINE_DOT_END,          /* End line of a DOT item */
    ITEM_LINE_DOT_FILE          /* DOT file to include */
};

/*****/

/****s* Items/RB_Item_Line
 * FUNCTION
 *   Holds the attributes and the content of an item line
 *  ATTRIBUTES
 *    * line        -- The contents of the item line
 *    * kind        -- The type of the line (see: ItemLineKind)
 *    * format      -- Formatting attributes for the line
 *    * pipe_mode   -- The output mode
 *    * line_number -- The original source line number of the line
 * SOURCE
 */

struct RB_Item_Line
{
    char               *line;
    enum ItemLineKind   kind;
    long                format;
    T_RB_DocType        pipe_mode;
    int                 line_number;
};

/******/

/****s* Items/RB_Item
 * FUNCTION
 *   Keeps track of where items start end end in the header.
 *   The index numbers point to the lines array in
 *   RB_header.
 * SOURCE
 */

struct RB_Item
{
    struct RB_Item     *next;
    enum ItemType       type;
    int                 no_lines;
    struct RB_Item_Line **lines;
    int                 begin_index;
    int                 end_index;
    int                 max_line_number;
};

/******/



int                 RB_Get_Item_Type(
    char * );
int                 RB_Get_Item_Attr(
    char *cmp_name );
enum ItemType       RB_Is_ItemName(
    char *line );
int                 RB_Ignore_Last_Item(
    void );
char               *RB_Get_Item_Name(
    void );
struct RB_Item     *RB_Create_Item(
    enum ItemType arg_item_type );
int                 Is_Ignore_Item(
    char *name );
int                 Works_Like_SourceItem(
    enum ItemType item_type );

int                 Is_Preformatted_Item(
    enum ItemType item_type );

int                 Is_Format_Item(
    enum ItemType item_type );

#endif /* ROBODOC_ITEMS_H */
