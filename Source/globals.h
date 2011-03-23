#ifndef ROBODOC_GLOBALS_H
#define ROBODOC_GLOBALS_H
/*
 * This file is part of ROBODoc, See COPYING for the license.
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

