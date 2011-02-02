#ifndef ROBODOC_XMLDB_GENERATOR_H
#define ROBODOC_XMLDB_GENERATOR_H
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


#include "headers.h"
#include "document.h"

#define MAX_SECTION_DEPTH 7

void                RB_XMLDB_Generate_Label(
    FILE *dest_doc,
    char *name );
void                RB_XMLDB_Generate_Item_Begin(
    FILE *dest_doc );
void                RB_XMLDB_Generate_Item_End(
    FILE *dest_doc );
void                RB_XMLDB_Generate_TOC_2(
    FILE *dest_doc,
    struct RB_header **headers,
    int count );
void                RB_XMLDB_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name );
void                RB_XMLDB_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name );
char               *RB_XMLDB_Get_Default_Extension(
    void );
void                RB_XMLDB_Generate_Doc_Start(
    struct RB_Document *document,
    FILE *dest_doc,
    char *charset );
void                RB_XMLDB_Generate_Doc_End(
    FILE *dest_doc,
    char *name );
void                RB_XMLDB_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_XMLDB_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_XMLDB_Generate_Index(
    struct RB_Document *document,
    FILE *dest,
    char *dest_name );
void                RB_XMLDB_Generate_Index_Table(
    FILE *dest,
    char *dest_name,
    int type,
    char *title );
void                RB_XMLDB_Generate_Empty_Item(
    FILE *dest );
void                RB_XMLDB_Generate_Link(
    FILE *dest,
    char *dest_name,
    char *filename,
    char *labelname,
    char *linkname );
void                RB_XMLDB_Generate_Char(
    FILE *dest_doc,
    int c );
void                RB_XMLDB_Generate_Item_Name(
    FILE *dest_doc,
    char *name );
char               *RB_XMLDB_RelativeAddress(
    char *thisname,
    char *thatname );
int                 RB_XMLDB_Generate_Extra(
    FILE *dest_doc,
    int item_type,
    char *cur_char );
void                RB_XMLDB_Generate_False_Link(
    FILE *dest_doc,
    char *name );

void                XMLDB_Generate_Begin_Paragraph(
    FILE *dest_doc );
void                XMLDB_Generate_End_Paragraph(
    FILE *dest_doc );
void                XMLDB_Generate_Begin_Preformatted(
    FILE *dest_doc );
void                XMLDB_Generate_End_Preformatted(
    FILE *dest_doc );
void                XMLDB_Generate_Begin_List(
    FILE *dest_doc );
void                XMLDB_Generate_End_List(
    FILE *dest_doc );
void                XMLDB_Generate_Begin_List_Item(
    FILE *dest_doc );
void                XMLDB_Generate_End_List_Item(
    FILE *dest_doc );

void RB_XMLDB_Generate_Item_Line_Number(
    FILE *,
    char * );

#endif /* ROBODOC_XMLDB_GENERATOR_H */
