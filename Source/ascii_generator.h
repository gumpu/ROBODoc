#ifndef ROBODOC_ASCII_GENERATOR_H
#define ROBODOC_ASCII_GENERATOR_H

/*
 * This file is part of ROBODoc, See COPYING for the license.
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
