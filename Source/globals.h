#ifndef ROBODOC_GLOBALS_H
#define ROBODOC_GLOBALS_H
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

/* TODO  Try to get rid of as many as these as possible. */

#define MAX_LINE_LEN        512
#define MAX_TABS            256
#define DEFAULT_TABSIZE       8
#define TABSIZE_SEPARATOR   ","
#define DEFAULT_HEADER_BREAKS 2
#define MAX_HEADER_BREAKS   255

#define TEMP_BUF_SIZE      1024
#define DEFAULT_DOT_NAME    "dot"
#define DOT_GRAPH_NAME      "dot_graph_"
#define DOT_HTML_TYPE       "png"
#define DOT_LATEX_TYPE      "ps"

/* Class definitions for syntax highlighting */
/* Note: Should correlate with css file */
#define SOURCE_CLASS        "source"
#define KEYWORD_CLASS       "keyword"
#define COMMENT_CLASS       "comment"
#define QUOTE_CLASS         "quote"
#define SQUOTE_CLASS        "squote"
#define SIGN_CLASS          "sign"

/* The class for the line numbers */
#define LINE_NUMBER_CLASS   "line_number"

/* Default HTML charset */
#define DEFAULT_CHARSET     "ISO-8859-1"

/* Default document title */
#define DEFAULT_DOCTITILE   "API Reference"

/* Document title */
extern char        *document_title;

extern char        *source_file;        /* DCD */

/* extern T_RB_DocType output_mode; */
/* extern long course_of_action; */
/* extern int line_number; */
void                RB_Close_The_Shop(
    void );

/* More ugly globals... Any idea where should we place them? */
extern int          tab_stops[MAX_TABS];
extern char        *dot_name;
extern int          header_breaks;
extern char        *use_source_comments;

/* These are really necessary */
extern int          number_of_warnings;
extern unsigned int link_index_size;
extern struct RB_link **link_index;
extern char        *current_file;
extern T_RB_DocType output_mode;
extern actions_t    course_of_action;
extern int          line_number;
extern char         line_buffer[MAX_LINE_LEN];
extern char        *whoami;
extern char        *myLine;
extern int          readChars;
extern long         debugmode;

#endif /* ROBODOC_GLOBALS_H */

