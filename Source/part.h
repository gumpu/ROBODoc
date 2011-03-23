#ifndef ROBODOC_PART_H
#define ROBODOC_PART_H
/*
 * This file is part of ROBODoc, See COPYING for the license.
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
