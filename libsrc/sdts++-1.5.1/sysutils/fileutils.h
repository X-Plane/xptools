//
// $Id: fileutils.h,v 1.2 2001/03/25 21:44:05 mcoletti Exp $
//
//

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>

namespace fileutils
{
   std::string dirname( std::string const & path );

   std::string basename( std::string const & path );

} // namespace fileutils


#endif
