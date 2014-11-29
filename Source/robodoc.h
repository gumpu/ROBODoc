#ifndef ROBODOC_ROBODOC_H
#define ROBODOC_ROBODOC_H

/*
Copyright (C) 1994-2013  Frans Slothouber, Jacco van Weert, Petteri Kettunen,
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef VERSION
#define VERSION "4.99.42"
#endif

#define COMMENT_ROBODOC \
    "Generated with ROBODoc Version " VERSION " (" __DATE__ ")\n"
#define COMMENT_COPYRIGHT\
    "ROBODoc (c) 1994-2014 by Frans Slothouber and many others.\n"

/* Semaphore bits for actions */
typedef struct actions_s
{
    /* General options */
    char                do_nosort;
    char                 do_nodesc;
    char                 do_toc;
    char                 do_include_internal;
    char                 do_internal_only;
    char                 do_tell;
    char                 do_index;
    char                 do_nosource;
    char                 do_robo_head;
    char                 do_sections;
    char                 do_lockheader;
    char                 do_footless;
    char                 do_headless;
    char                 do_nopre;
    char                 do_ignore_case_when_linking;
    char                 do_nogenwith;
    char                 do_sectionnameonly;
    char                 do_verbal;
    char                 do_source_line_numbers;

    /* Document modes */
    char                 do_singledoc;
    char                 do_multidoc;
    char                 do_singlefile;
    char                 do_one_file_per_header;
    char                 do_no_subdirectories;

    /* Latex options */
    char                 do_altlatex;
    char                 do_latexparts;

    /* Syntax coloring */
    char                 do_quotes;
    char                 do_squotes;
    char                 do_line_comments;
    char                 do_block_comments;
    char                 do_keywords;
    char                 do_non_alpha;

    /* Hyphens in identifiers */
    char                 do_hyphens;

} actions_t;

/* RB_Say modes */
#define SAY_DEBUG            (1<<0)
#define SAY_INFO             (1<<1)


/* Output Modes */

/****t* Generator/T_RB_DocType
 * FUNCTION
 *   Enumeration for the various output formats that are
 *   supported by ROBODoc.
 * NOTES
 *   These should be prefixed with RB_ 
 * SOURCE
 */

typedef enum
{
    TEST = 1,                   /* Special output mode for testing */
    ASCII,
    HTML,
    LATEX,
    RTF,
    TROFF,
    XMLDOCBOOK,
    /* SIZE_MODES, */
    /* Reserved for Future Use */
    /* ANSI, */
    /* GNUINFO, */
    /* XML, */
    UNKNOWN
} T_RB_DocType;

/*****/


#define USE( x ) ( x = x );

/* Evil macros !! */
#define skip_while(cond) { for (;*cur_char && (cond);cur_char++) ; }
#define find_eol   { for (;*cur_char && *cur_char!='\n';cur_char++) ; }
#define find_quote { for (;*cur_char && *cur_char!='\"';cur_char++) ; }

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

/* Prototypes */

actions_t           No_Actions(
    void );

#endif /* ROBODOC_ROBODOC_H */

