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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "document.h"
#include "file.h"
#include "path.h"
#include "part.h"
#include "directory.h"
#include "globals.h"
#include "robodoc.h"
#include "generator.h"
#include "analyser.h"

int                 test_document(  );
int                 test_directory(  );
int                 test_file(  );

/*
 The output of this programmed if diffed with a reference
 file to see if robodoc still does what is supposed to do.
*/

int
main( int argc, char **argv )
{
    course_of_action = course_of_action | DO_TELL;
    whoami = argv[0];
    printf( "robodoc unittest\n" );
    test_file(  );
    test_directory(  );
    test_document(  );
    return EXIT_SUCCESS;
}


/*
 Test struct RB_Document and it's functions.
*/
int
test_document(  )
{
    struct RB_Document *document = NULL;
    struct RB_Filename *rbfilename = NULL;
    struct RB_Path     *rbpath = NULL;
    struct RB_Part     *rbpart = NULL;

    /* Try creating and deleting and empty document */ .
        document = RB_Get_RB_Document(  );
    assert( document );
    RB_Free_RB_Document( document );


    /* Now we fill it with some stuff. */
    document = RB_Get_RB_Document(  );
    assert( document );
    rbpath = RB_Get_RB_Path( "/home/robodoc/test2/" );
    rbfilename = RB_Get_RB_Filename( "robodoc.c", rbpath );

    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rbfilename ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rbfilename ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rbfilename ) );

    rbpart = RB_Get_RB_Part(  );
    RB_Part_Add_Source( rbpart, rbfilename );

    rbfilename = RB_Get_RB_Filename( "analyser.c", rbpath );
    RB_Part_Add_Source( rbpart, rbfilename );

    RB_Part_Dump( rbpart );

    RB_Document_Add_Part( document, rbpart );

    RB_Analyse_Document_NG( document );

    RB_Generate_Documentation_NG( document );

    RB_Free_RB_Document( document );

    return 0;
}

int
test_directory(  )
{
    struct RB_Directory *rb_directory = NULL;

    printf( "test_directory\n" );

    rb_directory = RB_Get_RB_Directory( "/home/robodoc/test2/" );
    assert( rb_directory );
    RB_Dump_RB_Directory( rb_directory );
    RB_Free_RB_Directory( rb_directory );
    return 0;
}


/* Test path and file functions. */

int
test_file(  )
{
    struct RB_Path     *rb_path1 = NULL;
    struct RB_Filename *rb_filename1 = NULL;

    printf( "test_file\n" );
    rb_path1 = RB_Get_RB_Path( "/home/robodoc/test2/" );
    rb_filename1 = RB_Get_RB_Filename( "test.c", rb_path1 );
    assert( rb_filename1 );
    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rb_filename1 ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rb_filename1 ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rb_filename1 ) );

    RB_Free_RB_Filename( rb_filename1 );

    rb_path1 = RB_Get_RB_Path( "/home/robodoc/test2/" );
    assert( rb_path1 );
    rb_filename1 = RB_Get_RB_Filename( "Makefile", rb_path1 );
    assert( rb_filename1 );
    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rb_filename1 ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rb_filename1 ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rb_filename1 ) );

    RB_Free_RB_Filename( rb_filename1 );

    rb_path1 = RB_Get_RB_Path( "./" );
    assert( rb_path1 );
    rb_filename1 = RB_Get_RB_Filename( "test.cpp", rb_path1 );
    assert( rb_filename1 );
    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rb_filename1 ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rb_filename1 ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rb_filename1 ) );

    RB_Free_RB_Filename( rb_filename1 );

    rb_path1 = RB_Get_RB_Path( "./" );
    assert( rb_path1 );
    rb_filename1 = RB_Get_RB_Filename( ".testrc", rb_path1 );
    assert( rb_filename1 );
    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rb_filename1 ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rb_filename1 ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rb_filename1 ) );

    RB_Free_RB_Filename( rb_filename1 );

    /* Try something with a lot of dots */
    rb_path1 = RB_Get_RB_Path( "./../../" );
    assert( rb_path1 );
    rb_filename1 = RB_Get_RB_Filename( "file.c.doc", rb_path1 );
    assert( rb_filename1 );
    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rb_filename1 ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rb_filename1 ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rb_filename1 ) );

    RB_Free_RB_Filename( rb_filename1 );

    /* Lets see if a slash gets added if we don't
       specify the end slash in a path name. */
    rb_path1 = RB_Get_RB_Path( "/home/robodoc/test2" );
    assert( rb_path1 );
    rb_filename1 = RB_Get_RB_Filename( "Makefile", rb_path1 );
    assert( rb_filename1 );
    printf( "Fullname  [ %s ]\n", RB_Get_Fullname( rb_filename1 ) );
    printf( "Path      [ %s ]\n", RB_Get_Path( rb_filename1 ) );
    printf( "Extension [ %s ]\n", RB_Get_Extension( rb_filename1 ) );

    RB_Free_RB_Filename( rb_filename1 );

    return 0;
}
