//
// sio_8211_t.cpp
//
// Exercises some of the 8211 classes by dumping out
// detailed 8211 information for the given 8211 file.
//
// XXX This is exercises the read stuff, but what about
// XXX the write interface?
//

//#include <dmalloc.h>

#include <iostream>
#include <fstream>

#include <algorithm>


using namespace std;


#include <sdts++/io/sio_8211DirEntry.h>
#include <sdts++/io/sio_8211DDR.h>
#include <sdts++/io/sio_8211DDRLeader.h>
#include <sdts++/io/sio_8211DDRField.h>
#include <sdts++/io/sio_8211DR.h>
#include <sdts++/io/sio_8211DRLeader.h>
#include <sdts++/io/sio_8211Utils.h>



void
printDDRField(ostream& ostr, sio_8211DDRField const& field)
{
  ostr << "\tDDR Field:\n";
  ostr << "\t\tData Struct Code: " << field.getDataStructCode() << '\n';
  ostr << "\t\tData Type Code: " << field.getDataTypeCode() << '\n';
  ostr << "\t\tField Name: '" << field.getDataFieldName() << "'\n";
  ostr << "\t\tArray Descriptor: '" << field.getArrayDescriptor() << "'\n";
  ostr << "\t\tFormat Controls: '" << field.getFormatControls() << "'\n";
  ostr << '\n';

} // print DDRFied



void
printField(ostream& ostr,
           sio_8211Field const& field)

{
  ostr << "\tField Data:\n\t\tLength: " << field.getDataLength() << "\n";

  ostr << "\t\tData: ";

  for ( vector<char>::const_iterator i = field.getData().begin();
        i != field.getData().end();
        i++ )
    {
      if ( ((*i) >= 32) && ((*i) <= 126) )
        ostr << (*i);
      else
        if (*i == sio_8211UnitTerminator)
          ostr << '&';
        else if (*i == sio_8211FieldTerminator)
          ostr << ';';
        else
          ostr << '.';
    }

  ostr << "\n\n";

} // printField



void
printDDRLeader(ostream& ostr, sio_8211DDRLeader const& leader)
{
  ostr << "DDR:\n\n";
  ostr << "\tLeader:\n";
  ostr << "\t\tLength: " << leader.getRecordLength() << "\n";
  ostr << "\t\tIdentifier: '" << leader.getLeaderIdentifier() << "'\n";
  ostr << "\t\tField Control Length: " << leader.getFieldControlLength() << '\n';
  ostr << "\t\tBase Address of Field Area: " << leader.getBaseAddrOfFieldArea() << "\n";
  ostr << "\t\tSize of Field Length Field: " << leader.getSizeOfFieldLengthField() << "\n";
  ostr << "\t\tSize of Field Pos Field: " << leader.getSizeOfFieldPosField() << "\n";
  ostr << "\t\tSize of Field Tag Field: " << leader.getSizeOfFieldTagField() << "\n";

} // printDDRLeader



void
printDRLeader(ostream& ostr, sio_8211DRLeader const& leader)
{
  ostr << "DR:\n\n";
  ostr << "\tLeader:\n";
  ostr << "\t\tLength: " << leader.getRecordLength() << "\n";
  ostr << "\t\tIdentifier: '" << leader.getLeaderIdentifier() << "'\n";
  ostr << "\t\tBase Address of Field Area: " << leader.getBaseAddrOfFieldArea() << "\n";
  ostr << "\t\tSize of Field Length Field: " << leader.getSizeOfFieldLengthField() << "\n";
  ostr << "\t\tSize of Field Pos Field: " << leader.getSizeOfFieldPosField() << "\n";
  ostr << "\t\tSize of Field Tag Field: " << leader.getSizeOfFieldTagField() << "\n";

} // printDRLeaderr


void
printDirectory(ostream& ostr, sio_8211Directory const& dir)
{
  ostr << "\n\tDirectory Entries:\n\n";
  sio_8211Directory::const_iterator i;
  for (i = dir.begin(); i != dir.end(); i++)
    {
      ostr << "\t\tEntry:\n";
      ostr << "\t\t\tField Tag: '" << (*i).getTag() << "'\n";
      ostr << "\t\t\tField Length: " << (*i).getFieldLength() << "\n";
      ostr << "\t\t\tField Position: " << (*i).getPosition() << "\n";         
    }

} // printDirectory



void
printDR(ostream& ostr, sio_8211DR const& dr)
{
  sio_8211DRLeader const * leader = dynamic_cast<const sio_8211DRLeader*>(&(dr.getLeader()));

  if ( ! leader )
    {
      cerr << "invalid leader\n";
      return;
    }

  printDRLeader(ostr, *leader);

  printDirectory(ostr,dr.getDirectory());

  sio_8211FieldArea const& fieldArea = dr.getFieldArea();
  ostr << "\n   Fields:\n\n";
  sio_8211FieldArea::const_iterator j;
  for (j = fieldArea.begin(); j != fieldArea.end(); j++)
    {
      printField(ostr,*j);
    }

} // printDR



class
showDDRField
{
public:

  showDDRField( ostream& os, sio_8211DDRLeader ddrl ) 
    : _os( os ), _leader( ddrl ) {}

  void operator()( sio_8211DirEntry const & de )
    {
      if ( "000" == de.getTag().substr(0,3) ) // special field, so just print it
        {
          printField( _os, *(de.getField()) );
        }
      else                      // DDR field format field 
        {
          sio_8211DDRField ddf( _leader, *(de.getField()) );
          printDDRField( _os, ddf );
        }
    }


private:

  ostream& _os;

  sio_8211DDRLeader& _leader;

}; // showDDRField



void
printDDR(ostream& ostr, sio_8211DDR const& ddr)
{
                                // first print leader

  sio_8211Leader const& leader = ddr.getLeader();

  sio_8211DDRLeader const* ddr_leader = 
    dynamic_cast<sio_8211DDRLeader const*>(&leader);
  printDDRLeader(ostr,*ddr_leader);

                                // then dump the directory

  sio_8211Directory const& dir = ddr.getDirectory();
  printDirectory(ostr,dir);

  ostr << "\n\n\n";

                                // then dump out the DDR data area


  for_each( dir.begin(), dir.end(), showDDRField( cout, *ddr_leader )  );

} // printDDR



int
main(int argc, char** argv)
{

  if (argc != 2)
    {
      cerr << "Usage: " << argv[0] << " <8211 filename>\n";
      return 1;
    }

  // XXX need to add a check for this in the configuration script
#ifdef WIN32
  ifstream istr(argv[1],ios::binary);
#else
  ifstream istr(argv[1]);
#endif
  if (!istr)
    {
      cerr << "Error opening file '" << argv[1] << "'. Aborting.\n";
      return 2;
    }


  cout << "\n\nFilename: '" << argv[1] << "'\n\n";

  sio_8211DDR ddr;
  istr >> ddr;
  printDDR(cout,ddr);

  sio_8211DRLeader const* leader;
  sio_8211DR dr;

  do
    {
      istr >> dr;

      if ( ! istr ) break;

      leader = dynamic_cast<sio_8211DRLeader const*>(&(dr.getLeader()));

      if (leader->getLeaderIdentifier() == 'R')
        dr.reuseLeaderAndDirectory(true);

      printDR(cout,dr);
    }
  while ( istr );

  return 0;
}
