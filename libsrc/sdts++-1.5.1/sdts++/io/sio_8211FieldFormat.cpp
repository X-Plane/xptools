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
// sio_8211FieldFormat.cpp: implementation of the sio_8211FieldFormat class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <sdts++/io/sio_8211FieldFormat.h>

#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>
#include <functional>


#ifndef INCLUDED_SIO8211DDRFIELD_H
#include <sdts++/io/sio_8211DDRField.h>
#endif


#ifndef INCLUDED_SIO_CONVERTER_H
#include <sdts++/io/sio_Converter.h>
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif


using namespace std;


/**

  sio_8211FieldFormat's implementation structure

  */
struct
sio_8211FieldFormatImp
{
    sio_8211FieldFormat::data_struct_code data__struct_code;

    sio_8211FieldFormat::data_type_code   data__type_code;

    string tag_;

    string name_;

    char field__term;
    char unit__term;

    bool isRepeating_; // true if this is a repeating field

    sio_8211FieldFormatImp()
        :	data__struct_code( sio_8211FieldFormat::elementary ),
                data__type_code( sio_8211FieldFormat::implicit_point ),
                tag_(""),
                name_(""),
                field__term(';'),
                unit__term('&'),
                isRepeating_(false)
        {}

#ifdef NOTNEEDED

    sio_8211FieldFormatImp& operator=( sio_8211FieldFormatImp const & rhs )
        {
            if ( &rhs == this ) { return *this; }

            data__struct_code =  rhs.data__struct_code;
            data__type_code   =  rhs.data__type_code;
            tag_              =  rhs.tag_;
            name_             =  rhs.name_;
            field__term       =  rhs.field__term;
            unit__term        =  rhs.unit__term;
            isRepeating_      =  rhs.isRepeating_;

            return *this;
        }
#endif

}; // sio_8211FieldFormatImp




sio_8211FieldFormat::sio_8211FieldFormat()
    : imp_( new sio_8211FieldFormatImp )
{} // sio_8211FieldFormat ctor



sio_8211FieldFormat::sio_8211FieldFormat( sio_8211FieldFormat const & ff )
    :	imp_( new sio_8211FieldFormatImp(*ff.imp_) )
{
    this->insert( begin(), ff.begin(), ff.end() ); // copy over subfields
} // sio_8211FieldFormat copy ctor



sio_8211FieldFormat::~sio_8211FieldFormat()
{
    if ( imp_ ) { delete imp_; }
} // sio_8211FieldFormat dtor




sio_8211FieldFormat&
sio_8211FieldFormat::operator=( sio_8211FieldFormat const & rhs )
{
    if ( &rhs != this )
    {
        if ( imp_ )
        {
            *imp_ = *rhs.imp_;
        }
        else                    // We don't have an implementation,
        {                       // so create a new one with rhs values.
            imp_ = new sio_8211FieldFormatImp( *rhs.imp_ );
        }
    }

    this->insert( begin(), rhs.begin(), rhs.end() ); // copy over subfields

    return *this;
} // sio_8211FieldFormat::operator=




bool
sio_8211FieldFormat::operator<( sio_8211FieldFormat const & rhs ) const
{
    return imp_->tag_.compare( rhs.imp_->tag_ ) < 0;
} // sio_8211FieldFormat::operator=




bool
sio_8211FieldFormat::operator>( sio_8211FieldFormat const & rhs ) const
{
    return imp_->tag_.compare( rhs.imp_->tag_ ) > 0;
} // sio_8211FieldFormat::operator=



bool
sio_8211FieldFormat::operator==( sio_8211FieldFormat const & rhs ) const
{
    return imp_->tag_.compare( rhs.imp_->tag_ ) == 0;
} // sio_8211FieldFormat::operator=


bool
sio_8211FieldFormat::operator==( string const & rhs ) const
{
    return imp_->tag_.compare( rhs ) == 0;
} // sio_8211FieldFormat::operator=


bool
sio_8211FieldFormat::operator!=( string const & rhs ) const
{
    return imp_->tag_.compare( rhs ) != 0;
} // sio_8211FieldFormat::operator=



sio_8211FieldFormat::data_struct_code 
sio_8211FieldFormat::getDataStructCode( ) const
{
    return imp_->data__struct_code;
} // sio_8211FieldFormat::getDataStructCode



sio_8211FieldFormat::data_type_code 
sio_8211FieldFormat::getDataTypeCode( ) const
{
    return imp_->data__type_code;
} // sio_8211FieldFormat::getDataTypeCode



string const& 
sio_8211FieldFormat::getTag( ) const
{
    return imp_->tag_;
} // sio_8211FieldFormat::getTag



string const& 
sio_8211FieldFormat::getName( ) const
{
    return imp_->name_;
} // sio_8211FieldFormat::getName()



char 
sio_8211FieldFormat::getFieldTerm( ) const
{
    return imp_->field__term;
} // sio_8211FieldFormat::getFieldTerm



char 
sio_8211FieldFormat::getUnitTerm( ) const
{
    return imp_->unit__term;
} // sio_8211FieldFormat::getUnitTerm


bool 
sio_8211FieldFormat::isRepeating( ) const
{
    return imp_->isRepeating_;
} // sio_8211FieldFormat::isRepeating




void 
sio_8211FieldFormat::setDataStructCode( data_struct_code dsc )
{
    imp_->data__struct_code = dsc;
} // sio_8211FieldFormat::setDataStructCode



void 
sio_8211FieldFormat::setDataTypeCode( data_type_code dtc )
{
    imp_->data__type_code = dtc;
} // sio_8211FieldFormat::setDataTypeCode



void 
sio_8211FieldFormat::setTag( string const & tag )
{
    imp_->tag_ = tag;
} // sio_8211FieldFormat::setTag



void 
sio_8211FieldFormat::setName( string const & name )
{
    imp_->name_ = name;
} // sio_8211FieldFormat::setName()



void 
sio_8211FieldFormat::setFieldTerm( char ft )
{
    imp_->field__term = ft;
} // sio_8211FieldFormat::setFieldTerm



void 
sio_8211FieldFormat::setUnitTerm( char ut )
{
    imp_->unit__term = ut;
} // sio_8211FieldFormat::setUnitTerm




void 
sio_8211FieldFormat::setIsRepeating( bool repeating )
{
    imp_->isRepeating_ = repeating;
} // sio_8211FieldFormat::setIsRepeating




// buffer containing current subfield format string for the parser
const char * sio_8211_subfield_format_buffer = "";


// container of hints for resolving binary subfield types in the format parser
sio_8211_converter_dictionary const* sio_8211_binary_converter_hints;


// cursor into a subfield format to be set by the format string by the parser
sio_8211FieldFormat::iterator current_sio_8211Subfield;


// three guesses what this is ... and the first two don't count
extern int sio_8211_yyparse();




struct yy_buffer_state;		// opaque type used by flex

// used by flex to get its input from a string and not from a stream
yy_buffer_state* sio_8211_yy_scan_bytes( const char*, int s );

// frees up resources used by a yy_buffer_state
void sio_8211_yy_delete_buffer( yy_buffer_state* bs );



//
// Simple debugging function that can be called from sio_8211MakeFieldFormat()
//
static
void
_dumpConverterMap( sio_8211_converter_dictionary const* c_map )
{
    if ( ! c_map ) 
    {
        cerr << __FILE__ << " no map\n";
        return;
    }

    cerr << "converter map:\n";

    for ( sio_8211_converter_dictionary::const_iterator i = c_map->begin();
          i != c_map->end();
          i++ )
    {
        cerr << "\t" << (*i).first.c_str() << "\t" << (*i).second << "\n";
    }
} // dumpConverterMap_



bool 
sio_8211MakeFieldFormat( sio_8211FieldFormat &		ff,
			 sio_8211DDRField const &	ddr_field,
			 string const &			field_tag,
			 sio_8211_converter_dictionary const* binary_converter_hints )
 
{
    // dumpConverterMap_( binary_converter_hints );


    // set field specific stuff first

    ff.setTag( field_tag );
    ff.setName( ddr_field.getDataFieldName() );

    switch ( ddr_field.getDataStructCode() )
    {
    case '0' : ff.setDataStructCode( sio_8211FieldFormat::elementary ); break;
    case '1' : ff.setDataStructCode( sio_8211FieldFormat::vector ); break;
    case '2' : ff.setDataStructCode( sio_8211FieldFormat::array ); break;
    case '3' : ff.setDataStructCode( sio_8211FieldFormat::concatenated ); break;
    default : return false;
    }


    switch ( ddr_field.getDataTypeCode() )
    {
    case '0' : ff.setDataTypeCode( sio_8211FieldFormat::char_string); break;
    case '1' : ff.setDataTypeCode( sio_8211FieldFormat::implicit_point ); break;
    case '2' : ff.setDataTypeCode( sio_8211FieldFormat::explicit_point ); break;
    case '3' : ff.setDataTypeCode( sio_8211FieldFormat::explicit_point_scaled); break;
    case '4' : ff.setDataTypeCode( sio_8211FieldFormat::char_bit_string ); break;
    case '5' : ff.setDataTypeCode( sio_8211FieldFormat::bit_string ); break;
    case '6' : ff.setDataTypeCode( sio_8211FieldFormat::mixed_data_type ); break;
    default : return false;
    }

                                // XXX Again, do we really need to
                                // XXX keep this information around in the
                                // XXX format?
                                // XXX These aren't (yet) proviced.
                                // XXX In fact, they may never be.

	// xxx ff.setFieldTerm( ddr_field.getFieldControls().[4] );
	// xxx ff.setUnitTerm( ddr_field.getFieldControls().[5] );


                                // for each subfield label, create a
                                // subfield format

    string const & array_descriptor = ddr_field.getArrayDescriptor();

    string temp_label("");      // label string re-used for each subfield

    int i = 0;

                                // if we're dealing with the array, then
                                // there should__ be an '*' as the first
                                // character of the subfield labels; if so,
                                // skip that character

    if ( sio_8211FieldFormat::array == ff.getDataStructCode() &&
         '*' == array_descriptor[0] )
    {
        i++;
    }

    for (;;)
    {
                                // grab the next '!' delimited subfield label

        while( i < array_descriptor.length() && 
               array_descriptor[i] != '!' ) 
        {
            temp_label += array_descriptor[i++];
        }

                                // if the label is empty, then either
                                // there was no
                                // label or we're out of labels; regardless
                                // we're done so bail

        if ( 0 == temp_label.length() ) break;

        ff.push_back( sio_8211SubfieldFormat() ); // add a new subfield format 
                                                  // and set
        ff.back().setLabel( temp_label );         // its label

        i++;                    // move over the '!' delimiter or one beyond the
                                // subfield label string

        temp_label = "";        // reset the label for the next subfield
                                // format in line
    };

                                // now parse the format string and set the type
                                // for each subfield format that was created

    // Set an iterator to the first subfield format, then have a parser grind 
    // its way through the format string, setting each subfield format to 
    // reasonable values along the way.

                                // set the string buffer pointer to the start of
                                // the field format controls string

    sio_8211_subfield_format_buffer = ddr_field.getFormatControls().c_str();

                                // set the global hints for the parser
	
    sio_8211_binary_converter_hints = binary_converter_hints;

                                // set the global subfield format
                                // cursor to the first
                                // subfield

    current_sio_8211Subfield = ff.begin();

    yy_buffer_state* bf;
    bf = sio_8211_yy_scan_bytes( sio_8211_subfield_format_buffer, 
                                 ddr_field.getFormatControls().length() );

    sio_8211_yyparse();

    sio_8211_yy_delete_buffer( bf );


    return true;

} // sio_8211MakeFieldFormat



ostream&
operator<<( ostream& os, sio_8211FieldFormat const& ff )
{
    os << ff.getTag() << " : " << ff.getName() 
       << (  ( ff.isRepeating() ) ? "\t(repeating)" : "" )  << "\n\t";

    switch ( ff.getDataStructCode() )
    {
    case sio_8211FieldFormat::elementary :
        os << "is elementary";
        break;

    case sio_8211FieldFormat::vector :
        os << "is vector";
        break;

    case sio_8211FieldFormat::array :
        os << "is array";
        break;

    case sio_8211FieldFormat::concatenated :
        os << "is concatenated";
        break;

    default :
        os << "is unknown";
        break;

    }

    os << "\n\t";

    switch ( ff.getDataTypeCode() )
    {
    case sio_8211FieldFormat::elementary :
        os << "is elementary";
        break;

    case sio_8211FieldFormat::implicit_point :
        os << "is implicit point";
        break;

    case sio_8211FieldFormat::explicit_point :
        os << "is explicit point";
        break;

    case sio_8211FieldFormat::explicit_point_scaled :
        os << "is explicit point, scaled";
        break;

    case sio_8211FieldFormat::char_bit_string :
        os << "is character bit string";
        break;

    case sio_8211FieldFormat::bit_string :
        os << "is bit string";
        break;

    case sio_8211FieldFormat::mixed_data_type :
        os << "is mixed data type";
        break;

    default :
        os << "is unknown";
        break;

    }

    os << "\n\t";

    ostream_iterator<sio_8211SubfieldFormat> os_itr( os, "\n\t" );

    copy( ff.begin(), ff.end(), os_itr );

    return os;

} // operator<<
