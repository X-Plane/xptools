/* 
 * Copyright (c) 2016, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#ifndef WED_METADATAKEYS_H
#define WED_METADATAKEYS_H

#include <string>

struct MetaDataKey
{
	string display_text; //The text displayed to the user, such as "City/Locality" or "FAA Code"
	string name;    //The name used in WED, such as "city" or "faa_code"
};

//Any wed_AddMetaDataKey from wed_AddMetaDataKeyBegin to wed_AddMetaDataKeyEnd (inclusive)
typedef int KeyEnum;

//Returns the MetaDataKey struct associated with the key, asserts if passed in an invalid KeyEnum
const MetaDataKey& META_KeyInfo(KeyEnum key_enum);

//Returns the display text associated with a given (valid) key
const string& META_KeyDisplayText(KeyEnum key_enum);

//Returns the key name associated a given (valid) key
const string& META_KeyName(KeyEnum key_enum);

//Searches through the known keys for the KeyEnum matching the name given.
//Returns wed_AddMetaDataEnd if not found
KeyEnum META_KeyEnumFromName(const string& name_str);

//Searches through the known keys for the KeyEnum matching the display text given.
//Returns wed_AddMetaDataEnd if not found
KeyEnum META_KeyEnumFromDisplayText(const string& display_text_str);

#endif
