#ifndef ROBODOC_CONFIG_H
#define ROBODOC_CONFIG_H
/* vi: spell ff=unix */

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

typedef enum
{
    CFL_REMARK = 0,
    CFL_PARAMETER,
    CFL_SECTION,
    CFL_EMPTYLINE,
    CFL_UNKNOWN
} T_Line_Kind;

typedef enum
{
    SK_ITEMS = 0,
    SK_IGNOREITEMS,
    SK_OPTIONS,
    SK_HEADERTYPES,
    SK_IGNORE_FILES,
    SK_ACCEPT_FILES,
    SK_HEADER_MARKERS,
    SK_REMARK_MARKERS,
    SK_END_MARKERS,
    SK_REMARK_BEGIN_MARKERS,
    SK_REMARK_END_MARKERS,
    SK_SOURCE_ITEMS,
    SK_KEYWORDS,
    SK_SOURCE_LINE_COMMENTS,
    SK_HEADER_IGNORE_CHARS,
    SK_HEADER_SEPARATE_CHARS,
    SK_PREFORMATTED_ITEMS,
    SK_FORMAT_ITEMS,
    SK_ITEM_ORDER,
    SK_UNKNOWN
} T_Block_Kind;


/****s* Configuration/keywords_hash_s
 * FUNCTION
 *    Structure for a keyword hash table row.
 * ATTRIBUTES
 *    o keyword -- pointer to the keyword
 *    o next    -- pointer to next entry in the row
 * SOURCE
 */
struct keywords_hash_s
{
    struct keywords_hash_s *next;
    char               *keyword;
};

/*****/


/****s* Configuration/Parameters
 * FUNCTION
 *    Structure to store all the paramters found in a block in the
 *    robodoc configuation file.
 * ATTRIBUTES
 *    o number -- the number of parameters found.
 *    o size   -- the maximum size of the names array.
 *    o names  -- an array with the values of the parameters.
 * NOTES
 *    Find a better name for the attribute 'names'
 * SOURCE
 */

struct Parameters
{
    unsigned int        number;
    unsigned int        size;
    char              **names;
};

/*****/


/****s* Configuration/RB_Configuration
 * FUNCTION
 *   All the data from the robodoc.rc file is stored in this
 *   structure.
 * ATTRIBUTES
 *   o items                 -- an array with names that robodoc recognizes as
 *                              items.  Alsways includes the name "SOURCE" as
 *                              the first element.
 *   o ignore_items          -- an array with the names of items that ROBODoc
 *                              should ignore.
 *   o source_items          -- an array with the names of items that work
 *                              similar to the built-in SOURCE item.
 *   o preformatted_items    -- item names that will be automatically
 *                              preformatted
 *   o format_items          -- item names that should be formatted by the
 *                              browser
 *   o item_order            -- an array with item names that
 *                              indicates which items should be displayed first.
 *   o options               -- Array with all options specified both on the
 *                              commandline as well as in the robodoc.rc file.
 *   o custom_headertypes    -- list with custom header types.
 *   o ignore_files          -- list with wildcard expressions that specifies
 *                              files and directories that robodoc should skip
 *                              while scanning the source tree.
 *   o header_markers        -- list with markers that mark the begin of a
 *                              header.
 *   o remark_markers        -- list with markers that mark a remark.
 *   o end_markers           -- list with markers that markt the end of a
 *                              header.
 *   o remark_begin_markers  -- list of markers that mark the begin of
 *                              a remark.  For instance (*
 *   o remakr_end_markers    -- list of markers that mark the end of a
 *                              remark.  For instance   *)
 *   o keywords              -- source keywords to recognise (and colorise)
 *   o source_line_comments  -- comment markers that span until the end of line
 *   o header_ignore_chars   -- characters for beginning of header remarks
 *   o header_separate_chars -- characters that separates header artifacts
 *
 * SOURCE
 */

struct RB_Configuration
{
    struct Parameters   items;
    struct Parameters   ignore_items;
    struct Parameters   source_items;
    struct Parameters   preformatted_items;
    struct Parameters   format_items;
    struct Parameters   item_order;

    struct Parameters   options;

    struct Parameters   ignore_files;
    struct Parameters   accept_files;

    struct Parameters   custom_headertypes;
    struct Parameters   header_markers;
    struct Parameters   remark_markers;
    struct Parameters   end_markers;
    struct Parameters   remark_begin_markers;
    struct Parameters   remark_end_markers;

    struct Parameters   keywords;
    struct Parameters   source_line_comments;
    struct Parameters   header_ignore_chars;
    struct Parameters   header_separate_chars;
};

/*******/

char               *ReadConfiguration(
    unsigned int argc,
    char **argv,
    char *filename );
void                Free_Configuration(
    void );
void                Install_C_Syntax(
    void );
char               *Find_Keyword(
    char *keyword,
    int len );
char               *Find_Parameter_Exact(
    struct Parameters *params,
    char *paramname );
char               *Find_Parameter_Partial(
    struct Parameters *params,
    char *paramname );
char               *Find_Parameter_Char(
    struct Parameters *params,
    char param );

extern struct RB_Configuration configuration;

#endif
