# ROBODoc Version 4.99.44 February 2021

ROBODoc is program documentation tool. The idea is to include for
every function or procedure a standard header containing all
sorts of information about the procedure or function.  ROBODoc
extracts these headers from the source file and puts them in a
separate autodocs-file.  ROBODoc thus allows you to include the
program documentation in the source code and avoid having to
maintain two separate documents.  Or as Petteri puts it: "robodoc
is very useful - especially for programmers who don't like
writing documents with Word or some other strange tool."

ROBODoc can format the headers in a number of different formats:
HTML, RTF, LaTeX, or XML DocBook. In HTML mode it can
generate cross links between headers. You can even include parts
of your source code.

ROBODoc works with many programming languages: For instance C,
Pascal, Shell Scripts, Assembler, COBOL, Occam, Postscript,
Forth, Tcl/Tk, C++, Java -- basically any program in which you
can use remarks/comments.

## More information

- For information on how to build and install see INSTALL
- For information on how to use ROBODoc see Docs/manual.html.
- Blank headers for various languages can be found in Headers/
- For an example on how ROBODoc can be used see the ROBODoc source code in Source/
- For licence information see COPYING
- For a change log see  Source/robodoc.c

## Contributors

Many people contributed to ROBODoc, to name a few:

- Petteri Kettunen <petterik -a-t- iki.fi>
  Bug fixes, FOLD, C features.
- Bernd Koesling <KOESSI -a-t- CHESSY.aworld.de>
  Bug fixes, functional improvements, code cleanup.
- Anthon Pang  <apang -a-t- mindlink.net>
  RTF support, Bug fixes.
- Thomas Aglassinger <agi -a-t- sbox.tu-graz.ac.at>
  Fixes and cleanup of HTML-output
- Stefan Kost <kost -a-t- imn.htwk-leipzig.de>
  Idea of the master index file and different header types.

Questions, found a bug or a typo; send an email to [rfsber -a-t- xs4all.nl]

(c) 1994-2021  Frans Slothouber, Petteri Kettunen, and Jacco van Weert


