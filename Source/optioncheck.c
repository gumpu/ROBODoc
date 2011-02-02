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

/* $Id: optioncheck.c,v 1.23 2009/03/24 07:16:05 r_zaumseil Exp $*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "robodoc.h"
#include "optioncheck.h"
#include "roboconfig.h"
#include "util.h"


char                short_use[] =
    "Usage:\n"
    "   robodoc --src <directory> --doc <directory> --multidoc   [type] [options]\n"
    "   robodoc --src <directory> --doc <file>      --singledoc  [type] [options]\n"
    "   robodoc --src <file>      --doc <file>      --singlefile [type] [options]\n"
    "\n" "Use robodoc --help for more information\n";


void Print_Short_Use(
    void )
{
    fprintf( stdout, "%s\n", short_use );
}

/****t* UserInterface/Option_Test_Kind
 * FUNCTION
 *   Enumeration for the kind of tests that are carried out on the 
 *   options that the user specifies.
 * SOURCE
 */
typedef enum
{
    TEST_INVALID = 0,
    TEST_MUTUAL_EXCLUDE = 1,
    TEST_SHOULD_EXISTS,         /* TODO Create this kind of test */
    TEST_NO_EFFECT,             /* TODO Create this kind of test */
    TEST_COUNT,                 /* TODO Create this kind of test */
    TEST_SPELLING               /* TODO Create this kind of test */
} Option_Test_Kind;

/*****/

typedef enum
{
    OPTION_FATAL = 1,
    OPTION_WARNING
} Option_Error_Severity;

/****s* UserInterface/RB_Option_Name
 * FUNCTION
 *   Element in a list of option names.
 *   Used in a RB_Option_Test to specify to what
 *   options a test applies.
 * SOURCE
 */

struct RB_Option_Name
{
    /* linked list administration */
    struct RB_Option_Name *next;
    /* name of the option */
    char               *name;
    /* use by the Count test */
    int                 count;
};

/*****/

/****s* UserInterface/RB_Option_Test
 * FUNCTION
 *   A test specification for options. This
 *   stores information about the kind of test and
 *   the options it applies to, and the message that
 *   is given to the user.
 * SOURCE
 */

struct RB_Option_Test
{
    /* tests are stored in a linked list */
    struct RB_Option_Test *next;
    /* the group of options the test applies to. */
    struct RB_Option_Name *option_group;
    /* the kind of test */
    Option_Test_Kind    kind;
    /* More information for the user */
    char               *more_information;       /* TODO Fill and use */

    Option_Error_Severity severity;
};

/******/

/****v* UserInterface/ok_options
 * FUNCTION
 *   An list of all allowed command-line options.  If you add any
 *   options add its name here too.  This list is used to verify
 *   the options specified by the user.
 * SOURCE
 */

static char        *ok_options[] = {
    "--rc",
    "--src",
    "--config",
    "--doc",
    "--html",
    "--latex",
    "--ascii",
    "--rtf",
    "--troff",
    "--dbxml",
    "--doctype_name",
    "--doctype_location",
    "--singledoc",
    "--singlefile",
    "--multidoc",
    "--one_file_per_header",
    "--first_section_level",
    "--sections",
    "--internal",
    "--internalonly",
    "--ignore_case_when_linking",
    "--toc",
    "--index",
    "--nosource",
    "--tabsize",
    "--tell",
    "--debug",
    "--test",                   /* Special output mode for testing */
    "--nodesc",
    "--nopre",
    "--nogeneratedwith",
    "--no_subdirectories",
    "--cmode",
    "--charset",
    "--ext",
    "--help",
    "--css",
    "--version",
    "-c",
    "--lock",
    "--nosort",
    "--headless",
    "--footless",
    "--sectionnameonly",
    "--compress",
    "--mansection",
    "--verbal",                 /*FS TODO */
    "--documenttitle",
    "--altlatex",
    "--latexparts",
    "--tabstops",
    "--syntaxcolors",
    "--syntaxcolors_enable",
    "--dotname",
    "--masterindex",
    "--sourceindex",
    "--header_breaks",
    "--source_line_numbers",
    "--use_source_comments",
    ( char * ) NULL
};

/*****/


/****v* UserInterface/option_tests
 * FUNCTION
 *   A linked lists of tests that check the options specified
 *   by the user.
 * SOURCE
 */
static struct RB_Option_Test *option_tests = NULL;

/*****/


/****f* UserInterface/Add_Option_Test
 * FUNCTION
 *   Add a test to the linked list of options tests.
 * SYNOPSIS
 */
static void Add_Option_Test(
    struct RB_Option_Test *option_test )
/*
 * INPUTS
 *   option_test -- the test to be added.
 * SOURCE
 */
{
    option_test->next = option_tests;
    option_tests = option_test;
}

/*****/

/****f* UserInterface/Add_Option_Name
 * FUNCTION
 *   Add the name of an option to the group of option names an option
 *   test applies to.
 * SYNOPSIS
 */
static void Add_Option_Name(
    struct RB_Option_Test *option_test,
    char *name )
/* 
 * INPUTS
 *   option_test -- the option test
 *   name -- the name of the option
 * SOURCE
 */
{
    struct RB_Option_Name *new_option_name =
        malloc( sizeof( struct RB_Option_Name ) );
    new_option_name->name = RB_StrDup( name );
    new_option_name->next = option_test->option_group;
    new_option_name->count = 0;
    option_test->option_group = new_option_name;
}

/****/

/****f* UserInterface/Create_New_Option_Test
 * FUNCTION
 *   Allocate and initialize a new option test.
 * SYNOPSIS
 */
static struct RB_Option_Test *Create_New_Option_Test(
    Option_Test_Kind kind,
    Option_Error_Severity severity )
/*
 * INPUTS
 *   kind -- the kind of test that has to be created.
 * SOURCE
 */
{
    struct RB_Option_Test *new_option_test =
        malloc( sizeof( struct RB_Option_Test ) );
    new_option_test->next = NULL;
    new_option_test->option_group = NULL;
    new_option_test->kind = kind;
    new_option_test->severity = severity;
    return new_option_test;
}

/*****/

/****f* UserInterface/Create_Test_Data
 * FUNCTION
 *   Create a linked list of tests.
 * SYNOPSIS
 */
static void Create_Test_Data(
    void )
/*
 * TODO
 *   Generate this code automatically from a set
 *   of high-level specifications.
 * SOURCE
 */
{
    struct RB_Option_Test *cur_option_test = NULL;
    int                 i;

    cur_option_test =
        Create_New_Option_Test( TEST_MUTUAL_EXCLUDE, OPTION_FATAL );
    Add_Option_Name( cur_option_test, "--singledoc" );
    Add_Option_Name( cur_option_test, "--multidoc" );
    Add_Option_Name( cur_option_test, "--singlefile" );
    Add_Option_Test( cur_option_test );

    cur_option_test =
        Create_New_Option_Test( TEST_MUTUAL_EXCLUDE, OPTION_FATAL );
    Add_Option_Name( cur_option_test, "--html" );
    Add_Option_Name( cur_option_test, "--rtf" );
    Add_Option_Name( cur_option_test, "--ascii" );
    Add_Option_Name( cur_option_test, "--dbxml" );
    Add_Option_Name( cur_option_test, "--troff" );
    Add_Option_Name( cur_option_test, "--latex" );
    Add_Option_Name( cur_option_test, "--test" );
    Add_Option_Test( cur_option_test );

    /* Order is important here */
    cur_option_test = Create_New_Option_Test( TEST_COUNT, OPTION_FATAL );
    for ( i = 0; ok_options[i]; i++ )
    {
        Add_Option_Name( cur_option_test, ok_options[i] );
    }
    Add_Option_Test( cur_option_test );

}

/******/

static int Do_Count_Test(
    struct RB_Option_Test *cur_option_test )
{
    unsigned int        parameter_nr = 0;
    int                 result = EXIT_SUCCESS;
    struct RB_Option_Name *option_name;

    assert( cur_option_test );

    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {
        for ( option_name = cur_option_test->option_group;
              option_name; option_name = option_name->next )
        {
            if ( RB_Str_Case_Cmp
                 ( configuration.options.names[parameter_nr],
                   option_name->name ) == 0 )
            {
                ( option_name->count )++;
            }
        }
    }

    for ( option_name = cur_option_test->option_group;
          option_name; option_name = option_name->next )
    {
        if ( option_name->count > 1 )
        {
            fprintf( stderr, "The option %s is used more than once.\n",
                     option_name->name );
            result = EXIT_FAILURE;
        }
    }

    return result;
}


/****f* UserInterface/Do_Mutual_Exlcude_Test
 * FUNCTION
 *   Check all the options to see if combinations of options
 *   are used that mutually exclude each other, such as
 *   --singledoc and --multidoc.
 * SYNOPSIS
 */
static int Do_Mutual_Exlcude_Test(
    struct RB_Option_Test *cur_option_test )
 /*
  * INPUTS
  *   * cur_option_test -- the test to be carried out.
  * SOURCE
  */
{
    int                 n = 0;
    unsigned int        parameter_nr = 0;
    int                 result = EXIT_SUCCESS;

    assert( cur_option_test );

    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {
        struct RB_Option_Name *option_name = cur_option_test->option_group;

        for ( ; option_name; option_name = option_name->next )
        {
            if ( RB_Str_Case_Cmp
                 ( configuration.options.names[parameter_nr],
                   option_name->name ) == 0 )
            {
                ++n;
            }
        }
    }

    /* Only one of the options in the group may be used */
    if ( n > 1 )
    {
        fprintf( stderr, "The options: " );
        for ( parameter_nr = 0;
              parameter_nr < configuration.options.number; parameter_nr++ )
        {
            struct RB_Option_Name *option_name =
                cur_option_test->option_group;
            for ( ; option_name; option_name = option_name->next )
            {
                if ( RB_Str_Case_Cmp
                     ( configuration.options.names[parameter_nr],
                       option_name->name ) == 0 )
                {

                    fprintf( stderr, "%s ",
                             configuration.options.names[parameter_nr] );
                }
            }
        }
        fprintf( stderr, "cannot be used together.\n" );
        result = EXIT_FAILURE;
    }

    return result;
}

/*****/


/****f* UserInterface/Do_Option_Tests
 * FUNCTION
 *   Run a series of tests on the options that the user
 *   specified.  These tests are specified in 
 *   option_tests.
 * SYNOPSIS
 */
static int Do_Option_Tests(
    void )
/*
 * RESULT
 *   * EXIT_FAILURE -- one of the tests failed.
 *   * EXIT_SUCCESS -- all test completed successfully
 * SOURCE
 */
{
    struct RB_Option_Test *cur_option_test = NULL;
    int                 result = EXIT_SUCCESS;
    int                 final_result = EXIT_SUCCESS;

    RB_Say( "Checking the option semantics.\n", SAY_INFO );
    Create_Test_Data(  );
    cur_option_test = option_tests;

    assert( cur_option_test );

    for ( ; cur_option_test; cur_option_test = cur_option_test->next )
    {
        switch ( cur_option_test->kind )
        {
        case TEST_MUTUAL_EXCLUDE:
            RB_Say( "Checking for mutual excluding options.\n", SAY_INFO );
            result = Do_Mutual_Exlcude_Test( cur_option_test );
            break;
        case TEST_COUNT:       /* TODO Create */
            RB_Say( "Checking for duplicate options.\n", SAY_INFO );
            result = Do_Count_Test( cur_option_test );
            break;
        case TEST_SHOULD_EXISTS:       /* TODO Create */
        case TEST_NO_EFFECT:   /* TODO Create */
        case TEST_SPELLING:    /* TODO Create */
        default:
            assert( 0 );
        }
        /* If one of the tests fails the final result is a fail. */
        if ( result == EXIT_FAILURE )
        {
            final_result = EXIT_FAILURE;
            if ( cur_option_test->severity == OPTION_FATAL )
            {
                break;
            }
        }
    }

    return final_result;
}

/****/

/****f* UserInterface/Check_Option_Spelling
 * FUNCTION
 *   Check for misspelled options specified by the user.
 * SYNOPSIS
 */

int Check_Option_Spelling(
    void )
/*
 * RESULT
 *   * EXIT_SUCCESS -- all options are correctly spelled.
 *   * EXIT_FAILURE -- one of more options are misspelled.
 * SOURCE
 */
{
    char                ok;
    char               *arg;
    char              **opts;
    unsigned int        parameter_nr;

    RB_Say( "Checking the option syntax.\n", SAY_INFO );
    for ( parameter_nr = 0;
          parameter_nr < configuration.options.number; parameter_nr++ )
    {
        arg = configuration.options.names[parameter_nr];
        if ( ( arg[0] == '-' ) && ( arg[1] == '-' ) )
        {
            /* this arg is an option */
            ok = 0;
            opts = ok_options;
            while ( *opts )
            {
                if ( strcmp( arg, *opts ) == 0 )
                {
                    ok = 1;
                    break;
                }
                opts++;
            }
            if ( !ok )
            {
                fprintf( stderr, "Invalid argument: %s\n", arg );
                fprintf( stderr,
                         "This might also be in your robodoc.rc file\n" );
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

/******/


static int Check_Item_Names(
    struct Parameters *arg_parameters,
    char *block_name )
{
    unsigned int        parameter_nr_1;
    unsigned int        parameter_nr_2;
    int                 name_is_ok = TRUE;

    RB_Say( "Checking the item names for %s block.\n", SAY_INFO, block_name );
    for ( parameter_nr_1 = 0;
          parameter_nr_1 < arg_parameters->number; parameter_nr_1++ )
    {
        name_is_ok = FALSE;
        for ( parameter_nr_2 = 0;
              parameter_nr_2 < configuration.items.number; parameter_nr_2++ )
        {
            if ( strcmp( configuration.items.names[parameter_nr_2],
                         arg_parameters->names[parameter_nr_1] ) == 0 )
            {
                name_is_ok = TRUE;
            }
        }
        if ( !name_is_ok )
        {
            RB_Say( "!! block.\n", SAY_INFO );
            fprintf( stderr, "Unknown item %s found in the\n",
                     arg_parameters->names[parameter_nr_1] );
            fprintf( stderr, "   %s block\nof your configuration file.\n",
                     block_name );
            break;
        }
    }

    RB_Say( "Is %d block.\n", SAY_INFO, name_is_ok );
    return name_is_ok;
}

/****f* UserInterface/Check_Item_Options
 * FUNCTION
 *   Check the validity of the item options.  Users can specify their
 *   own items, item order, and items that ar to be ignored.  This all
 *   should be consistent.
 * SYNOPSIS
 */
static int Check_Item_Options(
    void )
/*
 * RESULT
 *   * EXIT_SUCCESS -- all options are correct.
 *   * EXIT_FAILURE -- one of more options incorrect.
 * SOURCE
 */
{
    int                 item_OK = TRUE;
    int                 result = TRUE;

    RB_Say( "Checking the item names.\n", SAY_INFO );
    result = item_OK
        && Check_Item_Names( &( configuration.ignore_items ),
                             "ignore items:" );
    item_OK = item_OK && result;
    result =
        Check_Item_Names( &( configuration.source_items ), "source items:" );
    item_OK = item_OK && result;
    result =
        Check_Item_Names( &( configuration.preformatted_items ),
                          "preformatted items:" );
    item_OK = item_OK && result;
    result =
        Check_Item_Names( &( configuration.format_items ), "format items:" );
    item_OK = item_OK && result;
    result = Check_Item_Names( &( configuration.item_order ), "item order:" );
    item_OK = item_OK && result;
    if ( item_OK )
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

/*****/


/****f* UserInterface/Check_Options
 * FUNCTION
 *   Check the validity of all the options in configuration.options[].
 *   This runs a number of checks.
 * SYNOPSIS
 */

int Check_Options(
    void )
/*
 * RESULT
 *   * EXIT_SUCCESS -- all options are correct.
 *   * EXIT_FAILURE -- one of more options incorrect.
 * SOURCE
 */
{
    int                 status;

    RB_Say( "Checking the options.\n", SAY_INFO );
    status = Check_Option_Spelling(  );
    if ( status == EXIT_SUCCESS )
    {
        status = Do_Option_Tests(  );
        if ( status == EXIT_SUCCESS )
        {
            status = Check_Item_Options(  );
        }
        else
        {
            Print_Short_Use(  );
        }
    }
    else
    {
        Print_Short_Use(  );
    }

    /* TODO free option_tests */

    return status;
}

/*****/
