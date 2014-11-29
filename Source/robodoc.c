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

/****h* Docuwala/ROBODoc
 * FUNCTION
 *   ROBODoc is intended to be a replacement for the original AutoDocs
 *   program.  ROBODoc will extract the comment headers from a source
 *   file, and put them into a separate documentation file.
 *   General Flow
 *
 *      Sourcecode  ---> [ROBODoc] ---> Document.
 *
 *   The whole ROBODoc process consists of three steps: scanning,
 *   analysing, generating.
 *
 *   Scanning 
 *
 *   ROBODoc scans the source directory tree. This collects the names of
 *   all the source files.
 *
 *   Analysing
 *
 *   ROBODOc analyses all the sourcefiles. This reads the content of all
 *   source files and collects all the headers.
 *
 *   Generating
 *
 *   In this step the headers are written to one or more documentation files.
 *   In addition 
 *
 *
 *   The data collected during scanning and analysing is stored in a
 *   number of structures.
 *
 *   RB_Directory, it stores the names of the sourcefiles and directories in
 *   the source direcory tree.  File names are stored in a RB_Filename
 *   structure, directory names in a RB_Path structure.  Each RB_Filename
 *   contains a pointer (path) to a RB_Path structure that tells in which
 *   directory a file is located.  Each RB_Path has a pointer (parent) to
 *   another RB_Path structure that tells in which directory is a directory
 *   located (of which directory it is a subdirectory).  The only exception
 *   is the root directory.
 *   
 *   Besides the name of the sourcefile, the RB_Filename also stores the
 *   name of the documentation file.
 *   
 *   For each sourcefile there is an RB_Part structure.  It contains a
 *   pointer (filename) to the RB_Filename and a list (headers) of
 *   RB_Header structure containing the headers found in the sourcefile.
 *   
 *   Every RB_Header structure contains a pointer (owner) to the RB_Part
 *   structure to which it belongs.  Headers can form a hierarchy that is
 *   used to create sections and subsections in the documentation.  To
 *   store this hierarchy every RB_header structure contains a pointer
 *   (parent) to its parent header.  For instance, given the following two
 *   headers, SubModule is the parent of SubSubModule.
 *     ****h* TopModule/SubModule
 *     *
 *     **** 
 *   
 *     ****h* SubModule/SubSubModule
 *     *
 *     **** 
 *   
 *   In the documentation this creates the sections
 *      1.TopModule
 *      1.1 SubModule
 *      1.1.1 SubSubModule
 *   
 *   The RB_Directory and the linked list of RB_Part structures are
 *   stored in a RB_Document structure.
 *   
 *   During the generation of the documentation ROBODoc tries to create
 *   cross links between the mention of a header's name (an object) and the
 *   documentation generated from that header (the documentation for the
 *   object).
 *   
 *   To aid this proces there is an array of RB_link structures.  This
 *   array is sorted for quick searching.  RB_link structures the name
 *   of a header and the name of the label under which the documentation
 *   can be found in the documentation file.
 *
 * AUTHOR
 *   See AUTHORS in the archive.
 * CREATION DATE
 *   20-Dec-94  Jacco van Weert.
 * MODIFICATION HISTORY
 *   See ChangeLog in the archive. Latest version can be found on:
 *   o http://www.xs4all.nl/~rfsber/Robo/
 * BUGS
 *   Other bugs? Catch them in a jar and send them to rfsber -(at)- xs4all.nl
 *
 ****
 * $Id: robodoc.c,v 1.115 2010/05/02 20:16:14 gumpu Exp $
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <locale.h>
#ifdef __APPLE__
#include <unistd.h>
#endif

#include "robodoc.h"
#include "globals.h"
#include "headers.h"
#include "util.h"
#include "links.h"
#include "part.h"
#include "analyser.h"
#include "generator.h"
#include "document.h"
#include "directory.h"
#include "roboconfig.h"
#include "optioncheck.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* Functions local to this file. */

T_RB_DocType        Find_DocType(
    void );

actions_t           Find_Actions(
    void );

long                Find_DebugMode(
    void );

int                 Find_Option(
    char *option );

static int          PathBegin_Check(
    char *path );

static void         Path_Check(
    char *sourcepath,
    char *docpath );

char               *Find_Parameterized_Option(
    char *actionname );

char               *RB_Find_In_Argv_Parameterized_Option(
    int argc,
    char **argv,
    char *optionname );

static char        *General_Find_Parameterized_Option(
    int n,
    char **options,
    char *optionname );

static char        *Fix_Path(
    char *path );

static char        *Path_Convert_Win32_to_Unix(
    char *path );

static char        *Find_And_Fix_Path(
    char *option_name );

static void         RB_Summary(
    struct RB_Document *document );

static void         Find_Tabstops(
    void );

static void         Find_And_Install_Document_Name(
    char *option,
    unsigned char type );

char                RB_VER[] =
    "$VER: robodoc " VERSION " (" __DATE__
    ") (c) by Maverick Software Development 1994-2006";


/****h* ROBODoc/UserInterface
 * FUNCTION
 *   This module contains functions to parse the
 *   command line and inform the user about any errors.
 *****
 */


/****v* UserInterface/use
 * NAME
 *   use -- usage string
 * FUNCTION
 *   Inform the user how to use ROBODoc.
 * AUTHOR
 *   Koessi
 * SOURCE
 */

char                use[] =
/*           1         2         3         4         5         6         7         8
    12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    "ROBODoc Version " VERSION "    autodocs formatter ($Revision: 1.115 $)\n"
    "(c) 1994-2010 Frans Slothouber, Petteri Kettunen, \n"
    "              Gergely Budai and Jacco van Weert\n"
    "ROBODoc comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; type `robodoc -c' for details.\n" "\n";
char                use_usage[] =
    "Usage:\n"
    "   robodoc --src <directory> --doc <directory> --multidoc   [type] [options]\n"
    "   robodoc --src <directory> --doc <file>      --singledoc  [type] [options]\n"
    "   robodoc --src <file>      --doc <file>      --singlefile [type] [options]\n"
    "\n"
    "Type:\n" "   --html, --rtf, --latex, --ascii, --dbxml, --troff\n" "\n";
char                use_options1[] =
    "Options:\n"
    "   --charset NAME   Add character encoding information (html only).\n"
    "   --cmode          Use ANSI C grammar in source items (html only).\n"
    "   --cobol          Allow hyphen in identifiers. Breaks cmode indexing.\n"
    "   --config         Print configuration info and exit.\n"
    "   --css            Specify the stylesheet to use.\n"
    "   --ext EXTENSION  Set extension for generated files.\n"
    "   --footless       Do not create the foot of a document.\n"
    "   --headless       Do not create the head of a document.\n"
    "   --index          Add an index.\n";
char                use_options2[] =
    "   --internal       Also include internal headers.\n"
    "   --internalonly   Only include internal headers.\n"
    "   --lock           Recognize only one header marker per file.\n"
    "   --nodesc         Do not descent into subdirectories.\n"
    "   --no_subdirectories\n"
    "                    Do no create documentation subdirectories.\n"
    "   --nopre          Do not use <PRE> </PRE> in the HTML output.\n"
    "   --nosort         Do not sort the headers.\n"
    "   --nosource       Do not include SOURCE items.\n"
    "   --nogeneratedwith\n"
    "                    Do not add the 'generated by robodoc' message\n"
    "                    at the top of each documentation file.\n"
    "   --ignore_case_when_linking\n"
    "                    Ignore the case of the symbols when trying\n"
    "                    to create crosslinks.\n";
char                use_options3[] =
    "   --rc             Specify an alternate configuration file.\n"
    "   --sections       Add sections and subsections.\n"
    "   --first_section_level NUMBER\n"
    "                    Start the first section not at 1 but at \n"
    "                    level NUMBER.\n"
    "   --tabsize NUMBER Set the tab size.\n"
    "   --tabstops a,b,..,n\n"
    "                    Set TAB stops \n"
    "   --tell           ROBODoc will tell you what it is doing.\n"
    "   --debug          same as --tell, but with lots more details.\n"
    "   --toc            Add a table of contents.\n"
    "   --version        Print version info and exit.\n"
    "   --sectionnameonly\n"
    "                    Generate section header with name only.\n"
    "   --compress       Only supported by TROFF output format. Defines by which\n"
    "                    program manpages will be compressed. Either bzip2 or gzip.\n"
    "   --mansection     Manual section where pages will be inserted (default: 3).\n"
    "   --documenttitle TITLE\n"
    "                    Set the document title\n"
    "   --altlatex       Alternate LaTeX file format (bigger / clearer than normal)\n"
    "   --latexparts     Make the first module level as PART in LaTeX output\n"
    "   --syntaxcolors   Turn on all syntax highlighting features in SOURCE items\n"
    "                    (html only)\n"
    "   --syntaxcolors_enable quotes,squotes,line_comments,block_comments,\n"
    "                         keywords,non_alpha\n"
    "                    Enable only specific syntax highlighting features in\n"
    "                    SOURCE items (html only)\n"
    "   --dotname NAME   Specify the name (and path / options) of DOT tool\n"
    "   --masterindex title,filename\n"
    "                    Specify the tile and filename for master index page\n"
    "   --sourceindex title,filename\n"
    "                    Specify the tile and filename for source files index page\n"
    "   --one_file_per_header\n"
    "                    Create a separate documentation file for each header\n"
    "   --header_breaks NUMBER\n"
    "                    Insert a linebreak after every NUMBER header names\n"
    "                    (default value: 2, set to zero to disable)\n"
    "   --source_line_numbers\n"
    "                    Display original source line numbers for SOURCE items\n"
    "   --use_source_comments ITEM\n"
    "                    Add found source comment lines to documenatation of ITEM\n"
    "\n";
char                use_authors[] =
    "Authors/Contributors:\n"
    "   Frans Slothouber, Jacco van Weert, Petteri Kettunen, Bernd Koesling,\n"
    "   Thomas Aglassinger, Anthon Pang, Stefan Kost, David Druffner, Sasha Vasko,\n"
    "   Kai Hofmann, Thierry Pierron, Friedrich Haase, Gergely Budai.\n";
/********/


/****v* UserInterface/copying
 * FUNCTION
 *   Information about the ROBODoc licence.
 * AUTHOR
 *   Frans
 * HISTORY
 *   2003-02-25/petterik: corrected link to GNU copyleft.
 *******
 */

char                copying[] =
    "\n"
    " Distributed under the GNU GENERAL PUBLIC LICENSE\n"
    "   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n"
    " See the source archive for a copy of the complete licence\n"
    " If you do not have it you can get it from URL\n"
    " http://www.gnu.org/copyleft/gpl.html\n";


static void dump_version(
    void )
{
    printf( "%s\n", VERSION );
}

static void dump_config(
    void )
{
    printf( "ROBODoc version %s\n", VERSION );
    printf( "Configuration:\n" );
#ifdef HAVE_CONFIG_H
    printf( "  Compiled using ./configure; make;\n" );
#endif
#ifdef ROBO_MAKEFILE_PLAIN
    printf( "  Compiled using make -f makefile.plain\n" );
#endif
#ifdef ROBO_COMPILER
    printf( "  Compiled with %s\n", ROBO_COMPILER );
#endif
#ifdef ROBO_CFLAGS
    printf( "  Compiler flags %s\n", ROBO_CFLAGS );
#endif
#ifdef ROBO_HOST
    printf( "  Compiled for %s\n", ROBO_HOST );
#endif
#ifdef ROBO_PREFIX
    printf( "  Prefix for excutable and configuration %s\n", ROBO_PREFIX );
#endif
    /* TODO create an option to show robodoc configuration,
     * including compiler setting etc..
     printf( "  locale: %s", setlocale( LC_ALL, "" ) );
     */
}




/****f* UserInterface/main
 * FUNCTION
 *   Get and parse the arguments.  Analyse document and generate the
 *   documentation. Everything starts from here.
 * TODO
 *   This function is too long.
 * SYNOPSIS
 */
int main(
    int argc,
    char **argv )
/*
 * SOURCE
 */
{
    struct RB_Document *document = NULL;
    struct RB_Directory *srctree = NULL;
    char               *optstr = NULL;
    char               *used_rc_file = NULL;
    long                debug = 0;

/*
   TODO, make setlocale work.
    char * loc;

    if ( (loc = getenv("LC_CTYPE") ) != NULL ) 
    {
        printf( ".... %s\n", loc );
        setlocale( LC_ALL, loc);
    }
    else
    {
        setlocale(LC_ALL, "");
    }
*/

    whoami = argv[0];           /* global me,myself&i */

    /* Init Actions global */
    course_of_action = No_Actions(  );

    /* Read the configuration file. This might contain
       addition options. */
    RB_SetCurrentFile( NULL );
    used_rc_file = ReadConfiguration( argc, argv,
                                      RB_Find_In_Argv_Parameterized_Option
                                      ( argc, argv, "--rc" ) );

    /* Force debug mode early if the user wants the debug mode */
    debugmode = Find_DebugMode(  );

    if ( Check_Options(  ) == EXIT_FAILURE )
    {

        return EXIT_FAILURE;
    }

    if ( Find_Option( "-c" ) )
    {
        printf( "%s", copying );
        return EXIT_SUCCESS;
    }

    if ( Find_Option( "--version" ) )
    {
        dump_version(  );
        return EXIT_SUCCESS;
    }

    if ( Find_Option( "--config" ) )
    {
        dump_config(  );
        return EXIT_SUCCESS;
    }


    if ( Find_Option( "--help" ) )
    {
        printf( "%s%s%s%s%s%s", use, use_usage, use_options1, use_options2,
                use_options3, use_authors );
        return EXIT_SUCCESS;
    }

    output_mode = Find_DocType(  );     /* one of the globals that are still left */
    if ( output_mode == UNKNOWN )
    {
        Print_Short_Use(  );
        return EXIT_SUCCESS;
    }

    /* First the basics. */
    document = RB_Get_RB_Document(  );
    document->doctype = output_mode;
    document->actions = Find_Actions(  );
    debug = document->debugmode = Find_DebugMode(  );
    document->charset = Find_Parameterized_Option( "--charset" );
    document->extension = Find_Parameterized_Option( "--ext" );
    document->css = Find_Parameterized_Option( "--css" );
    document->compress = Find_Parameterized_Option( "--compress" );
    document->section = Find_Parameterized_Option( "--mansection" );
    document_title = Find_Parameterized_Option( "--documenttitle" );
    optstr = Find_Parameterized_Option( "--first_section_level" );
    if ( optstr )
    {
        document->first_section_level = atoi( optstr );
    }

    course_of_action = document->actions;       /* a global */
    debugmode = document->debugmode;    /* a global */

    RB_Say( "Using %s for defaults\n", SAY_INFO, used_rc_file );
    free( used_rc_file );       /* No longer necessary */
    used_rc_file = NULL;

    if ( document->css )
    {
        document->css = Path_Convert_Win32_to_Unix( document->css );
    }

    if ( ( document->actions.do_index ) && output_mode == TROFF )
    {
        RB_Warning( "Index generation not supported for TROFF format.\n" );
        document->actions.do_index = FALSE;
    }

    if ( Find_Parameterized_Option( "--doctype_name" ) &&
         Find_Parameterized_Option( "--doctype_location" ) )
    {
        document->doctype_name =
            Find_Parameterized_Option( "--doctype_name" );
        document->doctype_location =
            Find_Parameterized_Option( "--doctype_location" );
    }

    /* Find tab sizes and tab stops */
    Find_Tabstops(  );

    /* Find master index file name */
    Find_And_Install_Document_Name( "--masterindex", HT_MASTERINDEXTYPE );

    /* Find source index file name */
    Find_And_Install_Document_Name( "--sourceindex", HT_SOURCEHEADERTYPE );

    /* Find DOT tool name */
    optstr = Find_Parameterized_Option( "--dotname" );
    if ( optstr )
    {
        dot_name = optstr;
    }

    /* Find number of headers before linebreak  */
    optstr = Find_Parameterized_Option( "--header_breaks" );
    if ( optstr )
    {
        int                 breaks = atoi( optstr );

        if ( breaks == 0 )
        {
            breaks = MAX_HEADER_BREAKS;
        }
        header_breaks = breaks;
    }

    /* Get extension */
    if ( !document->extension )
    {
        document->extension = RB_Get_Default_Extension( document->doctype );
    }


    /* Test if there is a --src and --doc */
    if ( !Find_Parameterized_Option( "--src" ) )
    {
        printf( "Error: you need to specify a source"
                " file or directory with --src.\n" );
        Print_Short_Use(  );
        return EXIT_FAILURE;
    }
    if ( !Find_Parameterized_Option( "--doc" ) )
    {
        printf( "Error: you need to specify a documentation file"
                " or directory with --doc.\n" );
        Print_Short_Use(  );
        return EXIT_FAILURE;
    }

    /* What mode are we using? */
    if ( Find_Option( "--multidoc" ) )
    {
        char               *srcrootname;        /* directory */
        char               *docrootname;

        srcrootname = Find_And_Fix_Path( "--src" );
        if ( !Stat_Path( 'e', srcrootname ) )
        {
            printf( "Error: %s does not exists\n", srcrootname );
            Print_Short_Use(  );
            return EXIT_FAILURE;
        }

        if ( !Stat_Path( 'd', srcrootname ) )
        {
            printf( "Error: %s is not a directory\n", srcrootname );
            Print_Short_Use(  );
            return EXIT_FAILURE;
        }
        document->srcroot = RB_Get_RB_Path( srcrootname );

        docrootname = Find_And_Fix_Path( "--doc" );
        Path_Check( srcrootname, docrootname );

        document->docroot = RB_Get_RB_Path( docrootname );

        srctree = RB_Get_RB_Directory( srcrootname, docrootname );
        document->srctree = srctree;

        RB_Document_Create_Parts( document );
        RB_Analyse_Document( document );
        RB_Generate_Documentation( document );

        RB_Free_RB_Path( document->srcroot );
        document->srcroot = 0;
        RB_Free_RB_Path( document->docroot );
        document->docroot = 0;
        RB_Free_RB_Directory( srctree );
        document->srctree = 0;
    }
    else if ( output_mode == TROFF )
    {
        RB_Panic( "Only --multidoc is supported for TROFF output.\n" );
    }
    else if ( Find_Option( "--singledoc" ) )
    {
        char               *srcrootname;        /* directory */

        srcrootname = Find_And_Fix_Path( "--src" );
        if ( !Stat_Path( 'e', srcrootname ) )
        {
            printf( "Error: %s does not exists\n", srcrootname );
            Print_Short_Use(  );
            return EXIT_FAILURE;
        }

        if ( !Stat_Path( 'd', srcrootname ) )
        {
            printf( "Error: %s is not a directory\n", srcrootname );
            Print_Short_Use(  );
            return EXIT_FAILURE;
        };
        document->srcroot = RB_Get_RB_Path( srcrootname );

        document->docroot = 0;
        document->singledoc_name = Find_And_Fix_Path( "--doc" );

        srctree = RB_Get_RB_Directory( srcrootname, NULL );
        document->srctree = srctree;

        RB_Document_Create_Parts( document );
        RB_Analyse_Document( document );
        RB_Generate_Documentation( document );

        RB_Free_RB_Directory( srctree );
    }
    else if ( Find_Option( "--singlefile" ) )
    {
        char               *srcfile;    /* file */
        char               *docfile;    /* file */

        document->docroot = 0;
        docfile = Find_And_Fix_Path( "--doc" );
        document->singledoc_name = docfile;
        srcfile = Find_And_Fix_Path( "--src" );
        if ( !Stat_Path( 'e', srcfile ) )
        {
            printf( "Error: %s does not exists\n", srcfile );
            Print_Short_Use(  );
            return EXIT_FAILURE;
        };

        if ( !Stat_Path( 'f', srcfile ) )
        {
            printf( "Error: %s is not a file\n", srcfile );
            Print_Short_Use(  );
            return EXIT_FAILURE;
        };

        document->srctree = RB_Get_RB_SingleFileDirectory( srcfile );
        document->srcroot =
            RB_Get_RB_Path( document->srctree->first_path->name );

        RB_Document_Create_Parts( document );
        RB_Analyse_Document( document );
        RB_Generate_Documentation( document );

        RB_Free_RB_Directory( document->srctree );
    }
    else
    {
        Print_Short_Use(  );
        printf
            ( "\n\nError: Use either --multidoc, --singledoc, or --singlefile\n" );
        return EXIT_FAILURE;
    }

    RB_Summary( document );
    RB_Free_RB_Document( document );
    Free_Configuration(  );

#ifdef __APPLE__
    /* Mac OS X specific: print memory leaks */
    if ( debug & SAY_DEBUG )
    {
        char                cmd[32];

        sprintf( cmd, "/usr/bin/leaks %d", getpid(  ) );
        system( cmd );
    }
#endif /* __APPLE__ */

    return EXIT_SUCCESS;
}

/*******/


/****if* UserInterface/Find_Tabstops
 * FUNCTION
 *   This function looks for the tab stop configuration and fills out the
 *   tab_stops table.
 * SYNOPSIS
 */
static void Find_Tabstops(
    void )
/*
 * SOURCE
 */
{
    int                 i;
    int                 tabsize = DEFAULT_TABSIZE;
    char               *optstr, *str;

    /* Find tab sizes */
    optstr = Find_Parameterized_Option( "--tabsize" );
    if ( optstr )
    {
        tabsize = atoi( optstr );
    }

    /* Fill tabstop table with default values */
    for ( i = 0; i < MAX_TABS; i++ )
    {
        tab_stops[i] = tabsize * ( i + 1 );
    }

    /* Find tab stops */
    optstr = Find_Parameterized_Option( "--tabstops" );
    if ( optstr )
    {
        /* Get TAB sizes and fill table */
        for ( str = strtok( optstr, TABSIZE_SEPARATOR ), i = 0;
              str != NULL && i < MAX_TABS;
              str = strtok( NULL, TABSIZE_SEPARATOR ), i++ )
        {
            tab_stops[i] = atoi( str ) - 1;
        }
    }
}

/*******/


/****if* UserInterface/Find_And_Install_Document_Name
 * FUNCTION
 *   Looks for specific option strings and installs special document titles
 *   and filenames.
 *
 *   Should be used to overwrite the HT_MASTERINDEXTYPE or HT_SOURCEHEADERTYPE
 *   entries in the header_type_lookup_table.
 * SYNOPSIS
 */
static void Find_And_Install_Document_Name(
    char *option,
    unsigned char type )
/*
 * INPUTS
 *   o option -- the option string to look for
 *   o type   -- the type entry in header_type_lookup_table to overwrite
 *               (Should be either HT_MASTERINDEXTYPE or HT_SOURCEHEADERTYPE)
 *
 * SOURCE
 */
{
    char               *title = NULL, *filename = NULL, *optstr = NULL;

    /* First find the option */
    optstr = Find_Parameterized_Option( option );
    if ( optstr )
    {

        /* Break the option into substrings */
        title = strtok( optstr, "," );
        filename = strtok( NULL, "," );

        /* If something missing */
        if ( title == NULL || filename == NULL )
        {
            RB_Panic( "Invalid %s option\n", option );
        }

        /* Install document title and filename */
        RB_AddHeaderType( type, title, filename, 0 );
    }
}

/*******/



/****f* UserInterface/Find_And_Fix_Path
 * FUNCTION
 *   Searches through the options to find a path.
 *   This path is converted to a propper path
 *   if it contains errors such as the use of
 *   '\' or when it does not start with ./ or a
 *   drive name.
 *   The option must exist.
 * SYNOPSIS
 */
static char        *Find_And_Fix_Path(
    char *option_name )
/*
 * INPUTS
 *   o option_name -- the option name for the path
 * RESULT
 *   o path -- the path.
 *
 * SOURCE
 */
{
    char               *temp;
    char               *temp2;

    temp = Find_Parameterized_Option( option_name );
    assert( temp );
    temp = Path_Convert_Win32_to_Unix( temp );
    temp2 = Fix_Path( temp );
    free( temp );
    return temp2;
}

/*******/


/****f* UserInterface/Path_Convert_Win32_to_Unix
 * FUNCTION
 *   Although people are supposed to specify all paths
 *   with a '/' as seperator, sometimes people on Win32
 *   use '\', this causes problems later on in some
 *   other function of robodoc that expect a '/'.
 *   So to prevent this we replace all the '\' in a path
 *   with '/'
 *    
 *   In addition people sometimes add a '/' at the
 *   end of the path. We remove it.
 *
 * SYNOPSIS
 */
static char        *Path_Convert_Win32_to_Unix(
    char *path )
/*
 * INPUTS
 *   o path -- the path.
 * RESULT
 *   o path -- the converted path (in a newly allocated
 *             block of memory).
 * SOURCE
 */
{
    size_t              i;

    /* First make a copy */
    path = RB_StrDup( path );
    for ( i = 0; i < strlen( path ); ++i )
    {
        if ( path[i] == '\\' )
        {
            path[i] = '/';
        }
    }

    /* Remove trailing '/' if there is one. */
    if ( path[strlen( path ) - 1] == '/' )
    {
        path[strlen( path ) - 1] = '\0';
    }

    return path;
}

/*****/

/****f* UserInterface/Path_Check
 * FUNCTION
 *   Test the validity of the doc and source path. The doc path should
 *   not be a sub directory of the source path otherwise the generated
 *   documentation will be part of the generated documentation if
 *   robodoc is run more than once.
 * SYNOPSIS
 */

static void Path_Check(
    char *sourcepath,
    char *docpath )
/*
 * INPUTS
 *   o sourcepath -- the path to the source files.
 *   o docpath    -- the path to the documentation files.
 * OUTPUT
 *   o error messages
 * SOURCE
 */
{
    if ( docpath )
    {
        int                 dl;
        int                 sl;

        dl = strlen( docpath );
        sl = strlen( sourcepath );
        if ( dl >= sl )
        {
            int                 i;
            int                 equal = TRUE;

            for ( i = 0; i < sl; ++i )
            {
                if ( docpath[i] != sourcepath[i] )
                {
                    equal = FALSE;
                    break;
                }
            }
            if ( equal && ( dl == sl ) )
            {
                RB_Panic
                    ( "The source path and document path can not be equal\n" );
            }
            else
            {
                /* OK  */
            }
        }
    }
}

/*****/


/****f* UserInterface/PathBegin_Check
 * FUNCTION
 *   Checks the validity of a path.
 *   A path should start with
 *     ./
 *   or
 *     /
 *   or
 *     have a ':' some where
 * SYNOPSIS
 */
static int PathBegin_Check(
    char *path )
/*
 * INPUTS
 *   o path -- the path to be cheked.
 * RESULT
 *   o FALSE -- path is not OK.
 *   o TRUE  -- path is OK.
 * SOURCE
 */
{
    int                 result = FALSE;
    int                 l = strlen( path );

    if ( l == 1 )
    {
        result = ( path[0] == '.' );
    }
    else if ( l >= 2 )
    {
        result = ( ( path[0] == '.' ) && ( path[1] == '/' ) ) ||
            ( path[0] == '/' ) || ( strchr( path, ':' ) != NULL );
    }
    else
    {
        /* Empty */
    }
    return result;
}

/******/



/****f* UserInterface/Find_Option
 * FUNCTION
 *   Search configuration.options for a specific option.
 * SYNOPSIS
 */
int Find_Option(
    char *option )
/* INPUTS
 *   o option -- the option to be found.
 * RESULT
 *   o TRUE  -- option does exist
 *   o FALSE -- option does not exist
 * SOURCE
 */
{
    unsigned int        parameter_nr;
    int                 found = FALSE;

    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {
        if ( !RB_Str_Case_Cmp
             ( configuration.options.names[parameter_nr], option ) )
        {
            found = TRUE;
            break;
        }
    }
    return found;
}

/******/

/****f* UserInterface/Fix_Path
 * FUNCTION
 *   Add a "./" to a path if it does not start with a "./" or does not
 *   contain a ":".  If the path was "." just add a "/".  Adding a
 *   "./" simplifies the creating of relative links during the
 *   generation process.
 * SYNOPSIS
 */

static char        *Fix_Path(
    char *path )
/*
 * INPUTS
 *   o path -- the path to be fixed.
 * RESULT
 *   A pointer to a newly allocated string containing the path.
 * SOURCE
 */
{
    char               *result = 0;

    if ( !PathBegin_Check( path ) )
    {
        char               *prefix = "./";

        if ( strcmp( path, "." ) == 0 )
        {
            result = RB_StrDup( prefix );
        }
        else
        {
            int                 l = strlen( path );

            l += strlen( prefix ) + 1;
            result = malloc( l );
            assert( result );
            result[0] = '\0';
            strcat( result, prefix );
            strcat( result, path );
        }
    }
    else
    {
        result = RB_StrDup( path );
    }
    return result;
}

/*****/


/* TODO: FS Documentation */

T_RB_DocType Find_DocType(
    void )
{
    T_RB_DocType        doctype = UNKNOWN;
    unsigned int        parameter_nr;

    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {

        if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                               "--html" ) )
        {
            doctype = HTML;
            break;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--latex" ) )
        {
            doctype = LATEX;
            break;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--ascii" ) )
        {
            doctype = ASCII;
            break;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--rtf" ) )
        {
            doctype = RTF;
            break;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--test" ) )
        {
            doctype = TEST;
            break;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--troff" ) )
        {
            doctype = TROFF;
            break;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--dbxml" ) )
        {
            doctype = XMLDOCBOOK;
            break;
        }
        else
        {
            /* Ignore */
        }
    }
    return doctype;
}


actions_t No_Actions(
    void )
{
    actions_t           actions;
    unsigned int        i;
    unsigned char      *actptr;

    for ( i = 0, actptr = ( unsigned char * ) &actions;
          i < sizeof( actions ); i++, actptr++ )
    {
        *actptr = 0;
    }

    return actions;
}

/* TODO: FS Documentation */
actions_t Find_Actions(
    void )
{
    actions_t           actions;
    unsigned int        parameter_nr;
    char               *optstr;

    actions = No_Actions(  );

    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {
        if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                               "--singledoc" ) )
        {
            actions.do_singledoc = 1 /* TRUE */;
        }
        if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                               "--singlefile" ) )
        {
            actions.do_singlefile = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--multidoc" ) )
        {
            actions.do_multidoc = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--no_subdirectories" ) )
        {
            actions.do_no_subdirectories = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--one_file_per_header" ) )
        {
            actions.do_one_file_per_header = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--sections" ) )
        {
            actions.do_sections = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--internal" ) )
        {
            actions.do_include_internal = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--ignore_case_when_linking" ) )
        {
            actions.do_ignore_case_when_linking = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--internalonly" ) )
        {
            actions.do_internal_only = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--toc" ) )
        {
            actions.do_toc = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--index" ) )
        {
            actions.do_index = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--nosource" ) )
        {
            actions.do_nosource = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--tell" ) )
        {
            actions.do_tell = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--debug" ) )
        {
            actions.do_tell = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--nodesc" ) )
        {
            actions.do_nodesc = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--nogeneratedwith" ) )
        {
            actions.do_nogenwith = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--cmode" ) )
        {
            actions.do_quotes = TRUE;
            actions.do_squotes = TRUE;
            actions.do_line_comments = TRUE;
            actions.do_block_comments = TRUE;
            actions.do_keywords = TRUE;
            actions.do_non_alpha = TRUE;

            Install_C_Syntax(  );
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--cobol" ) )
        {
            actions.do_quotes = TRUE;
            actions.do_squotes = TRUE;
            actions.do_line_comments = TRUE;
            actions.do_block_comments = TRUE;
            actions.do_keywords = TRUE;
            actions.do_non_alpha = TRUE;

            actions.do_hyphens = TRUE;

            Install_C_Syntax(  );
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--lock" ) )
        {
            actions.do_lockheader = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--footless" ) )
        {
            actions.do_footless = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--verbal" ) )
        {
            actions.do_verbal = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--headless" ) )
        {
            actions.do_headless = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--nosort" ) )
        {
            actions.do_nosort = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--nopre" ) )
        {
            actions.do_nopre = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--sectionnameonly" ) )
        {
            actions.do_sectionnameonly = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--altlatex" ) )
        {
            actions.do_altlatex = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--latexparts" ) )
        {
            actions.do_latexparts = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--syntaxcolors" ) )
        {
            actions.do_quotes = TRUE;
            actions.do_squotes = TRUE;
            actions.do_line_comments = TRUE;
            actions.do_block_comments = TRUE;
            actions.do_keywords = TRUE;
            actions.do_non_alpha = TRUE;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--source_line_numbers" ) )
        {
            actions.do_source_line_numbers = TRUE;
        }
        else
        {
            /* Not an action */
        }
    }

    // Find item to use source comments
    optstr = Find_Parameterized_Option( "--use_source_comments" );
    if ( optstr )
    {
        use_source_comments = optstr;
    }

    /* Find specific syntax colors enable options */
    optstr = Find_Parameterized_Option( "--syntaxcolors_enable" );
    if ( optstr )
    {
        char               *str;

        /* Parse options */
        for ( str = strtok( optstr, "," ); str; str = strtok( NULL, "," ) )
        {
            if ( !RB_Str_Case_Cmp( str, "quotes" ) )
            {
                actions.do_quotes = TRUE;
            }
            else if ( !RB_Str_Case_Cmp( str, "squotes" ) )
            {
                actions.do_squotes = TRUE;
            }
            else if ( !RB_Str_Case_Cmp( str, "line_comments" ) )
            {
                actions.do_line_comments = TRUE;
            }
            else if ( !RB_Str_Case_Cmp( str, "block_comments" ) )
            {
                actions.do_block_comments = TRUE;
            }
            else if ( !RB_Str_Case_Cmp( str, "keywords" ) )
            {
                actions.do_keywords = TRUE;
            }
            else if ( !RB_Str_Case_Cmp( str, "non_alpha" ) )
            {
                actions.do_non_alpha = TRUE;
            }
            else
            {
                /* Bad option specified */
                RB_Panic( "Invalid --syntaxcolors_enable option: %s\n", str );
            }
        }
    }

    return actions;
}

/* TODO: FS Documentation */
long Find_DebugMode(
    void )
{
    long                modes = 0;
    unsigned int        parameter_nr;

    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {
        if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                               "--debug" ) )
        {
            modes |= SAY_INFO;
            modes |= SAY_DEBUG;
        }
        else if ( !RB_Str_Case_Cmp( configuration.options.names[parameter_nr],
                                    "--tell" ) )
        {
            modes |= SAY_INFO;
        }
    }
    return modes;
}

/****f* UserInterface/Find_Parameterized_Option
 * FUNCTION
 *   Search for an option of the form
 *     --a_option_name a_value
 *   in configuration.options.
 * SYNOPSIS
 */
char               *Find_Parameterized_Option(
    char *optionname )
/*
 * INPUTS
 *   optionname -- the name of the option to search for.
 * RESULT
 *   NULL if the option is not found, a pointer to the value
 *   otherwise.
 * NOTES
 *   Results in a Panic if the option is found but
 *   no value is specified.
 * SOURCE
 */
{
    return General_Find_Parameterized_Option( configuration.options.number,
                                              &( configuration.options.
                                                 names[0] ), optionname );
}

/******/


/****f* UserInterface/RB_Find_In_Argv_Parameterized_Option
 * FUNCTION
 *   Search for an option of the form
 *     --a_option_name a_value
 *   in argv.   The function is used to look for the
 *     --rc  
 *   option that can be used to specify an 
 *   alternate robodoc configuration file.
 * SYNOPSIS
 */
char               *RB_Find_In_Argv_Parameterized_Option(
    int argc,
    char **argv,
    char *optionname )
/*
 * INPUTS
 *   * argc -- the argument count as received by main().
 *   * argv -- the array of argument values as received by main()
 *   * optionname -- the name of the option to search for.
 * RESULT
 *   NULL if the option is not found, a pointer to the value
 *   otherwise.
 * NOTES
 *   Results in a Panic if the option is found but
 *   no value is specified.
 * SOURCE
 */
{
    return General_Find_Parameterized_Option( argc, argv, optionname );
}

/*****/


/****f* UserInterface/General_Find_Parameterized_Option
 * FUNCTION
 *   Search for an option of the form
 *     --a_option_name a_value
 * SYNOPSIS
 */
static char        *General_Find_Parameterized_Option(
    int n,
    char **options,
    char *optionname )
/*
 * INPUTS
 *   o n -- the number of options in the options array.
 *   o options -- the options array
 *   o optionname -- the name of the option to search for.
 * RESULT
 *   NULL if the option is not found, a pointer to the value
 *   otherwise.
 * NOTES
 *   Results in a Panic if the option is found but
 *   no value is specified.
 * SOURCE
 */
{
    int                 parameter_nr;
    char               *value = NULL;

    for ( parameter_nr = 0; parameter_nr < n; parameter_nr++ )
    {
        if ( !RB_Str_Case_Cmp( options[parameter_nr], optionname ) )
        {
            if ( parameter_nr < n - 1 )
            {
                value = options[parameter_nr + 1];
                if ( ( value[0] == '-' ) && ( value[1] == '-' ) )
                {
                    value = NULL;
                }
            }
            else
            {
                /* to few parameters. */
            }
            if ( !value )
            {
                RB_Panic( "you must be more specific"
                          " with the %s option\n", optionname );
            }
        }
    }
    return value;
}

/******/


/* TODO Documentation */

static void RB_Summary(
    struct RB_Document *document )
{
    USE( document );
#if 0
    printf( "Ready\n" );
    if ( document )
    {
        printf( "Found %d headers\n", document->no_headers );
    }
    if ( number_of_warnings )
    {
        printf( "Found %d warnings\n", number_of_warnings );
    }
#endif
}
