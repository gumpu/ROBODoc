#ifndef ROBODOC_ASCII_GENERATOR_H
#define ROBODOC_ASCII_GENERATOR_H

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


#define ASCII_MAX_SECTION_DEPTH 7

void                RB_ASCII_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char toc );
void                RB_ASCII_Generate_Doc_End(
    FILE *dest_doc,
    char *name );
void                RB_ASCII_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_ASCII_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_ASCII_Generate_Index(
    FILE *dest,
    char *source );
void                RB_ASCII_Generate_Empty_Item(
    FILE *dest );
void                RB_ASCII_Generate_False_Link(
    FILE *dest_doc,
    char *name );
char               *RB_ASCII_Get_Default_Extension(
    void );
void                RB_ASCII_Generate_Item_Name(
    FILE *dest_doc,
    char *name );
void                RB_ASCII_Generate_Item_Begin(
    FILE *dest_doc );

void                RB_ASCII_Generate_Char(
    FILE *dest_doc,
    int c );
void                RB_ASCII_Generate_Item_End(
    FILE *dest_doc );

void RB_ASCII_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header );

void RB_ASCII_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name );

void RB_ASCII_Generate_Item_Line_Number(
    FILE *,
    char * );

#endif /* ROBODOC_ASCII_GENERATOR_H */
