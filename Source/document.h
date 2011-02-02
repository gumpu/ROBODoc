#ifndef ROBODOC_DOCUMENT_H
#define ROBODOC_DOCUMENT_H
/* vi: spell ff=unix 
 */


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
#include "robodoc.h"

/****s* Document/RB_Document
 * NAME
 *   RB_Document -- Information store.
 * FUNCTION
 *   A document is a collection of source files and documentation
 *   files.  Depending on the mode that is used there is either a
 *   single documentation file or there is one documentation file for
 *   each source file.  This mapping is stored in RB_Document.  For
 *   each source file there is an RB_Part. It points to the source
 *   file, the documentation file, and contains all the headers that
 *   were found in the source file.
 *
 * ATTRIBUTES
 *   * links    -- linked list of all links.
 *   * parts    -- linked list of all parts.
 *   * no_headers -- total number of headers
 *   * headers  -- array of pointers to all the headers.
 *   * srctree  -- the list of all sourcefiles in the srcroot.
 *   * doctype  -- the kind of documentation to be generated.
 *   * actions  -- what to de while analysing en generating.
 *   * srcroot  -- root dir for the sourcecode.
 *   * docroot  -- root dir for the documentation.
 *   * charset  -- the character set used for HTML and XML
 *               documentation.
 *   * first_section_level -- level of the first section, 
 *                            Defaults to 1 so the first section will
 *                            be 1.
 *                            If set to 2 the first section will be 1.1
 *   * extension -- the extension used for the documentation
 *                files.
 *   * css      -- the cascading style sheet to be used.
 *   * cur_part -- unused   TODO remove.
 * SOURCE
 */

struct RB_Document
{
    struct RB_Part     *cur_part;
    struct RB_Part     *parts;
    struct RB_link     *links;
    unsigned long       no_headers;
    struct RB_header  **headers;
    struct RB_Directory *srctree;
    T_RB_DocType        doctype;        /* HTML RTF etc */
    actions_t           actions;
    int                 first_section_level;  /* TODO document use of first_section_level in manual */
    long                debugmode;      /* TODO This should not be in document */
    char               *singledoc_name;
    struct RB_Path     *srcroot;        /* TODO Better make this a char* */
    struct RB_Path     *docroot;        /* TODO Better make this a char* */
    char               *charset;        /* HTML, XML? */
    char               *css;
    char               *extension;
    char               *compress;
    char               *section;
    /* Docbook specific */
    char               *doctype_name;    /* name part of the <!DOCTYPE> to be used with docbook output */
    char               *doctype_location;/* location part of the <!DOCTYPE> to be used with docbook output */
};

/*****/

void RB_Loop_Check( struct RB_Document *document );

struct RB_Document *RB_Get_RB_Document(
    void );
void                RB_Free_RB_Document(
    struct RB_Document *document );
void                RB_Document_Add_Part(
    struct RB_Document *document,
    struct RB_Part *part );
void                RB_Document_Dump(
    struct RB_Document *document );
void                RB_Document_Determine_DocFilePaths(
    struct RB_Document *document );
void                RB_Document_Determine_DocFileNames(
    struct RB_Document *document );
void                RB_Document_Create_DocFilePaths(
    struct RB_Document *document );
FILE               *RB_Open_SingleDocumentation(
    struct RB_Document *document );
void                RB_Document_Create_Parts(
    struct RB_Document *document );
void                RB_Document_Collect_Headers(
    struct RB_Document *document );
void                RB_Document_Link_Headers(
    struct RB_Document *document );
void                RB_Fill_Header_Filename(
    struct RB_Document *document );

struct RB_header   *RB_Document_Check_For_Duplicate(
    struct RB_Document *arg_document,
    struct RB_header *hdr );

void                RB_Document_Sort_Headers(
    struct RB_Document *document );

void                RB_Document_Split_Parts(
    struct RB_Document *document );

#endif /* ROBODOC_DOCUMENT_H */
