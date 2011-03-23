#ifndef ROBODOC_LATEX_GENERATOR_H
#define ROBODOC_LATEX_GENERATOR_H
/*
 * This file is part of ROBODoc, See COPYING for the license.
 */


#include "headers.h"

void                RB_LaTeX_Generate_Index_Entry(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_LaTeX_Generate_Item_Begin(
    FILE *dest_doc );
void                RB_LaTeX_Generate_Item_End(
    FILE *dest_doc );
void                RB_LaTeX_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header );
void                RB_LaTeX_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name );

/* size_t RB_LaTeX_Get_Len_Extension(); */
/* void RB_LaTeX_Add_Extension(char* name); */
char               *RB_LaTeX_Get_Default_Extension(
    void );
void                RB_LaTeX_Generate_Item_Name(
    FILE *dest_doc,
    char *name );
void                RB_LaTeX_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char *charset );
void                RB_LaTeX_Generate_Doc_End(
    FILE *dest_doc,
    char *name );
void                RB_LaTeX_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_LaTeX_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header );
void                RB_LaTeX_Generate_Index_Table(
    FILE *dest,
    int type,
    char *title );
void                RB_LaTeX_Generate_Item_Doc(
    FILE *dest_doc,
    char *dest_name,
    char *begin_of_item,
    char *end_of_item,
    char *function_name,
    int item_type );
void                RB_LaTeX_Generate_Empty_Item(
    FILE *dest );
void                RB_LaTeX_Generate_Char(
    FILE *dest_doc,
    int c );
void                RB_LaTeX_Generate_False_Link(
    FILE *dest_doc,
    char *name );

void                RB_LaTeX_Generate_String(
    FILE *dest_doc,
    char *a_string );
void                RB_LaTeX_Generate_EscapedChar(
    FILE *dest_doc,
    int c );

void                LaTeX_Generate_Begin_Paragraph(
    FILE *dest_doc );
void                LaTeX_Generate_End_Paragraph(
    FILE *dest_doc );
void                LaTeX_Generate_Begin_Preformatted(
    FILE *dest_doc );
void                LaTeX_Generate_End_Preformatted(
    FILE *dest_doc );
void                LaTeX_Generate_Begin_List(
    FILE *dest_doc );
void                LaTeX_Generate_End_List(
    FILE *dest_doc );
void                LaTeX_Generate_Begin_List_Item(
    FILE *dest_doc );
void                LaTeX_Generate_End_List_Item(
    FILE *dest_doc );

void                RB_LaTeX_Generate_Label(
    FILE *dest_doc,
    char *name );
void                RB_LaTeX_Generate_Link(
    FILE *cur_doc,
    char *cur_name,
    char *filename,
    char *labelname,
    char *linkname );

void RB_LaTeX_Generate_Item_Line_Number(
    FILE *,
    char * );

#endif /* ROBODOC_LATEX_GENERATOR_H */
