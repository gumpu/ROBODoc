#ifndef ROBODOC_TROFF_GENERATOR_H
#define ROBODOC_TROFF_GENERATOR_H
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
#include "items.h"

char               *RB_TROFF_Get_Default_Extension(
    void );
void                RB_TROFF_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char toc );
void                RB_TROFF_Generate_Doc_End(
    FILE *dest_doc,
    char *name );
FILE               *RB_TROFF_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_TROFF_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_TROFF_Generate_Empty_Item(
    FILE *dest );
void                RB_TROFF_Generate_Char(
    FILE *dest_doc,
    int c );
void                RB_TROFF_Generate_False_Link(
    FILE *out,
    char *name );
void                RB_TROFF_Generate_Item_Name(
    FILE *out,
    char *name,
    int pre );
int                 RB_TROFF_Generate_Extra(
    FILE *out,
    enum ItemType item_type,
    char *str );
void                TROFF_Generate_Begin_List_Item(
    FILE *out );
void                TROFF_Generate_End_List_Item(
    FILE *out );
void                TROFF_Generate_Begin_Preformatted(
    FILE *out );
void                TROFF_Generate_End_Preformatted(
    FILE *out );
void                TROFF_Generate_End_Paragraph(
    FILE *out );
void                TROFF_Generate_Begin_Paragraph(
    FILE *out );
void                RB_TROFF_Set_Param(
    char *compress,
    char *section );
void                RB_TROFF_Start_New_Line(
    FILE *out );

void RB_TROFF_Generate_Item_Line_Number(
    FILE *,
    char * );

#endif /* ROBODOC_TROFF_GENERATOR_H */
