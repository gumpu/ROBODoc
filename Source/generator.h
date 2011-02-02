#ifndef ROBODOC_GENERATOR_H
#define ROBODOC_GENERATOR_H

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
#include "headers.h"
#include "document.h"

void                RB_Generate_Doc_Start(
    struct RB_Document *document,
    FILE *dest_doc,
    char *src_name,
    char *title,
    char toc,
    char *dest_name,
    char *charset );
char               *RB_Generate_Item_Body(
    FILE *,
    char *,
    char *,
    char *,
    char *,
    int,
    int );
void                RB_Generate_Item_Name(
    FILE *,
    int );
void                RB_Generate_Doc_End(
    FILE *,
    char *,
    char * );
FILE               *RB_Generate_Header_Start(
    FILE *,
    struct RB_header * );
void                RB_Generate_Header_End(
    FILE *,
    struct RB_header * );
int                 RB_HTML_Extra(
    FILE *dest_doc,
    int item_type,
    char *cur_char,
    char prev_char );
void                RB_Generate_Index_Table(
    FILE *dest,
    int type,
    char *source );
void                RB_Name_Headers(
    struct RB_header **headers,
    long count );
void                RB_Generate_Documentation(
    struct RB_Document * );
void                RB_Generate_Part(
    FILE *document_file,
    struct RB_Document *document,
    struct RB_Part *part );
void                RB_Generate_Header_NG(
    FILE *f,
    struct RB_Document *document,
    struct RB_header *header,
    char *srcname,
    char *docname );
char               *RB_Get_SubIndex_FileName(
    char *docroot,
    char *extension,
    struct RB_HeaderType *header_type );
void                RB_Add_Extension(
    char *extension,
    char *name );
size_t              RB_Get_Len_Extension(
    char *extension );
char               *RB_Get_Default_Extension(
    T_RB_DocType doctype );
void                RB_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header );
void                RB_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name );
void                RB_Generate_Section(
    FILE *document_file,
    struct RB_header *parent,
    struct RB_Document *document,
    int depth );
void                RB_Generate_Sections(
    FILE *document_file,
    struct RB_Document *document );
void                RB_Generate_SingleDoc(
    struct RB_Document *document );
void                RB_Generate_MultiDoc(
    struct RB_Document *document );
void                Generate_Link(
    FILE *dest_doc,
    char *docname,
    char *file_name,
    char *label_name,
    char *function_name );
void                RB_Generate_Item_Begin(
    FILE *dest_doc,
    char *name );
void                RB_Generate_Item_End(
    FILE *dest_doc,
    char *name );
void                RB_Generate_Nav_Bar(
    struct RB_Document *document,
    FILE *current_doc,
    struct RB_header *current_header );
void                RB_Generate_TOC_2(
    FILE *dest_doc,
    struct RB_header **headers,
    int count,
    struct RB_Part *owner,
    char *dest_name );


#endif /* ROBODOC_GENERATOR_H */
