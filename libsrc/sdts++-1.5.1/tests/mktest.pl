#!/usr/local/bin/perl -w
#
# usage:
#   def file
#
# Will create a test file for a builder class.
#

# get module name first

$_ = <> || die("no file man");
chop;

m/(^[A-Za-z]+)/;

$module_name = $1;

$_ = $';
m/([A-Za-z]+[A-Za-z \/]+[A-Za-z\/]+)/;       # chop leading and trailing spaces
$description = $1;              # for description string

# read all the function types, which will be defined with the following
# format:
#
# long name<ws>mnemonic<ws>type

$long_name = "";
$mnemonic  = "";
$type      = "";

@facet     = ();


while ( <> ) {
    ( $long_name, $mnemonic, $type ) = split;

    #print $long_name, " ", $mnemonic, " ", $type, "\n";

    push @facet, [ $long_name, $mnemonic, $type ];
}


print <<EOT;
//
// This file is part of the SDTS++ toolkit, written by the U.S.
// Geological Survey.  It is experimental software, written to support
// USGS research and cartographic data production.
// 
// SDTS++ is public domain software.  It may be freely copied,
// distributed, and modified.  The USGS welcomes user feedback, but makes
// no committment to any level of support for this code.  See the SDTS
// web site at http://mcmcweb.er.usgs.gov/sdts for more information,
// including points of contact.
//
// \$Id\$
//

EOT

print "#include <builder/sb_", $module_name, ".h>\n\n";

print <<EOS;

#ifdef HAVE_ISO_HEADERS
#include <iostream>
#include <fstream>
#else
#include <iostream.h>
#include <fstream.h>
#endif

#ifdef WIN32
using namespace std;
#endif

#include <cassert>


#include <io/sio_8211Converter.h>
#include <io/sio_Reader.h>
#include <container/sc_Record.h>


int
main( int argc, char** argv )
{

  if ( ! argv[1] ) 
  {
EOS

#

    print "cerr << \"usage: \" << argv[0] << \" ", uc($module_name) ;
    print " module \" << endl;\n";

print <<EOT;
    exit( 1 );
  }

#ifdef WIN32
  ifstream ddf( argv[1], ios::binary );
#else
  ifstream ddf( argv[1] );
#endif

  if ( ! ddf ) 
  {
    cerr << "couldn't open " << argv[1] << endl;
    exit( 2 );
  }

  sio_8211Reader  reader( ddf );
  sc_Record record;
  sio_8211ForwardIterator i( reader );

  {

EOT

    print "sb_", $module_name, " ", lc($module_name), ";\n";

print <<EOF;

    while ( i )
      {
        if ( ! i.get( record ) ) break;

        cout << "what we read in:\n\n";
        cout << record << endl;

EOF

    print lc($module_name), ".setRecord( record );\n\n";

        print "cout << \"\\nand what the ", uc($module_name), " object says it is:\\n\";\n";

        print "cout << ", lc($module_name), " << endl;\n\n";

        print "cout << \"\\nand what the record built from the ";
        print uc($module_name), " object says it is:\\n\";\n";

        print lc($module_name), ".getRecord( record );\n\n";

        print "cout << record << endl;\n\n";

        print "++i;\n}\n}\n\n";

        print "// test building a record from scratch\n";

        print "{\nsb_", $module_name, " ", lc($module_name), ";\n\n";

        $j = 0;
        foreach $i ( @facet ) {
            print "assert( ", lc($module_name), ".set";
            print $i->[1], "( ";


            if ( $i->[2] eq "A" ) {
                print "\"", $i->[0], "\"";
            }
            elsif ( $i->[2] eq "I" ) {
                print $j;
            }
            elsif ( $i->[2] eq "R" ) {
                print $j;
            }
            else {
                print "bogus value ", $i->[2], "\n";
                exit( -1 );
            }

            print " ) );\n\n";

            $j++;

        }

        print "cout << ", lc($module_name), " << endl;\n";

        print "}\n\n";

        print "exit(0);\n";
        print "}\n";
