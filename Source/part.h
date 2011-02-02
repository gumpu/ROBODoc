#ifndef ROBODOC_PART_H
#define ROBODOC_PART_H
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


#include <stdio.h>

/****s* Part/RB_Part
 * NAME
 *   RB_Part -- a part of the total documentation
 * FUNCTION
 *   RB_Parts are stored in RB_Document.  For each source file there
 *   is an RB_Part. It points to the source file, the documentation
 *   file, and contains all the headers that were found in the source
 *   file.
 * ATTRIBUTES
 *   o next                   -- pointer to the next part
 *                             (to form a linked list).
 *   o filename               -- Information over the path to the
 *                             sourcefile and the correcsponding 
 *                             documentation file.
 *   o headers                -- All the headers that were
 *                             found in the sourcefile.
 *   o last_header            -- pointer to the last element in the
 *                             list of headers.
 *                             This is used to make it possible
 *                             to add the the heades in the 
 *                             same order as they were found in
 *                             the source file.
 *****
 */

struct RB_Part
{
    struct RB_Part     *next;
    struct RB_Filename *filename;
    struct RB_header   *headers;
    struct RB_header   *last_header;
};


struct RB_Part     *RB_Get_RB_Part(
    void );
void                RB_Free_RB_Part(
    struct RB_Part *part );
FILE               *RB_Open_Documentation(
    struct RB_Part *part );
FILE               *RB_Open_Source(
    struct RB_Part *part );

void                RB_Part_Add_Header(
    struct RB_Part *part,
    struct RB_header *header );
void                RB_Part_Add_Source(
    struct RB_Part *part,
    struct RB_Filename *sourcefilename );
void                RB_Part_Add_Doc(
    struct RB_Part *part,
    struct RB_Filename *docfilename );
void                RB_Part_Dump(
    struct RB_Part *part );
struct RB_Filename *RB_Part_Get_Source(
    struct RB_Part *part );

#endif /* ROBODOC_PART_H */
