#ifndef ROBODOC_HEADERS_H
#define ROBODOC_HEADERS_H

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

/****s* RB_header/RB_header_line
 *  FUNCTION
 *    This structure holds the content of one header line and the original
 *    source line number of it.
 *  ATTRIBUTES
 *    * line_number -- The original source file line number for this line
 *    * line        -- The content of the source file line
 *  SOURCE
 */

struct RB_header_lines
{
    int                 line_number;
    char               *line;
};

/*********/


/****s* Headers/RB_header
 *  FUNCTION
 *    This structure is used to store the headers that are extracted
 *    from the source files.
 *  MODIFICATION HISTORY
 *    * 8. August 1995: Koessi changed int version to char *version
 *    * 5. December 2007: Budai changed the type of lines from char to
 *      struct RB_header_lines
 *  ATTRIBUTES
 *    * next          -- used to store RB_headers in a RB_Part as
 *                       a linked list.
 *    * parent        -- the parent of this header
 *    * name          -- the full name of the header
 *                       modulename/functionname [versioninfo]
 *    * owner         -- the Part that owns this header.
 *    * htype         -- the type of this header.
 *    * items         -- pointers to the items in this header.
 *    * version       -- unused
 *    * function_name -- the functionname ( a better name would
 *                       be objectname or  thingy name,
 *                       it is the name of the thing that is
 *                       being documented ).
 *    * module_name   -- the modulename
 *    * unique_name   -- unique name used for labels.
 *    * file_name     -- documentation file of this header.
 *    * lines         -- content of the header with line numbers
 *    * no_lines      -- number of lines in the content.
 *    * line_number   -- the line number at which the header was
 *                       found in the source file.
 *  SOURCE
 */

struct RB_header
{
    struct RB_header   *next;
    struct RB_header   *parent;
    struct RB_Part     *owner;
    struct RB_HeaderType *htype;
    struct RB_Item     *items;
    int                 is_internal;
    char               *name;
    char              **names;
    int                 no_names;
    char               *version;
    char               *function_name;
    char               *module_name;
    char               *unique_name;
    char               *file_name;
    struct RB_header_lines *lines;
    int                 no_lines;
    int                 line_number;
};

/*********/

/****d* Headers/src_constants
 * NAME
 *   src_constants -- numerals for header_markers
 * NOTE
 *   Most of them seem to be unused at the moment.
 *   But it's better to keep it up to date for the
 *   eventuality of a later use by robohdrs.
 * SOURCE
 */

#define SRC_C        0
#define SRC_ACM      1          /* Added by David White for Aspen Custom Modeller */
#define SRC_CPP      2          /* All values incremented 1 to allow for ACM */
#define SRC_PASCAL   3
#define SRC_PASCAL2  4
#define SRC_APLUS    5          /* David White for Aspen Plus */
#define SRC_ASM      6
#define SRC_ASM2     7
#define SRC_FORTRAN  8
#define SRC_BASIC    9
#define SRC_TEX      10
#define SRC_SCRIPT   11
#define SRC_COBOL    12
#define SRC_OCCAM    13
#define SRC_HTML     14
#define SRC_HTML2    15
#define SRC_GNUASM   16
#define SRC_F902     17
#define SRC_F90      18
#define SRC_VB       20
#define SRC_DBC      21

/*********/

/****d* Headers/src_remark_constants
 * NAME
 *   src_remark_constants -- numerals for remark_markers
 * NOTE
 *   Most of them seem to be unused at the moment.
 *   But it's better to keep it up to date for the
 *   eventuality of a later use by robohdrs.
 * SOURCE
 */

#define SRC_R_C        0
#define SRC_R_ACM      1        /* Added by David White for Aspen Custom Modeller */
#define SRC_R_CPP      2        /* All values incremented 1 to allow for ACM */
#define SRC_R_PASCAL   3        /* PASCAL and PASCAL2 were 1, should have been 2, incr to 3 */
#define SRC_R_PASCAL2  3
#define SRC_R_APLUS    4        /* David White for Aspen Plus */
#define SRC_R_ASM      5
#define SRC_R_ASM2     6
#define SRC_R_FORTRAN  7
#define SRC_R_BASIC    8
#define SRC_R_TEX      9
#define SRC_R_SCRIPT   10
#define SRC_R_COBOL    11
#define SRC_R_OCCAM    12
#define SRC_R_GNUASM   13
#define SRC_R_F902     14
#define SRC_R_F90      15
#define SRC_R_VB       17
#define SRC_R_DBC      18
#define SRC_R_HTML     19       /* NULL */
#define SRC_R_HTML2    19       /* NULL */

/*********/

/****d* Headers/end_remark_constants [3.0h]
 * NAME
 *   end_remark_constants -- numerals for end_markers
 * NOTE
 *   Most of them seem to be unused at the moment.  But it's better to
 *   keep it up to date for the eventuality of a later use by
 *   robohdrs.
 * SOURCE
 */

#define SRC_E_C        0
#define SRC_E_ACM      1        /* Added by David White for Aspen Custom Modeller */
#define SRC_E_CPP      2        /* All values incremented 1 to allow for ACM */
#define SRC_E_PASCAL   4
#define SRC_E_PASCAL2  5
#define SRC_E_APLUS    6        /* David White for Aspen Plus */
#define SRC_E_ASM      7
#define SRC_E_ASM2     8
#define SRC_E_FORTRAN  9
#define SRC_E_BASIC    10
#define SRC_E_TEX      11
#define SRC_E_SCRIPT   12
#define SRC_E_COBOL    13
#define SRC_E_OCCAM    14
#define SRC_E_HTML     15
#define SRC_E_HTML2    16
#define SRC_E_GNUASM   17
#define SRC_E_F902     18
#define SRC_E_F90      19
#define SRC_E_VB       21
#define SRC_E_DBC      22


/*********/

extern char        *robo_header;        /* Added by DavidCD */
extern char        *robo_end[]; /* Added by DavidCD */
extern char        *header_markers[];
extern char        *remark_markers[];
extern char        *end_markers[];
extern char        *end_remark_markers[];
extern char        *RB_header_type_names[];
extern char        *RB_internal_header_type_names[];

int                 RB_Is_Begin_Marker(
    char *cur_line,
    char **type );
int                 RB_Is_End_Marker(
    char *cur_line );
void                RB_Header_Lock_Reset(
    void );
void                RB_Item_Lock_Reset(
    void );
char               *RB_Skip_Remark_Marker(
    char *lline_buffer );
int                 RB_Has_Remark_Marker(
    char *lline_buffer );

int                 RB_Is_Remark_End_Marker(
    char *cur_line );
int                 RB_Is_Remark_Begin_Marker(
    char *cur_line );
char               *RB_Skip_Remark_Begin_Marker(
    char *cur_line );
char               *RB_Skip_Remark_End_Marker(
    char *cur_line );

#endif /* ROBODOC_HEADERS_H */
