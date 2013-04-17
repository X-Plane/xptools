/* 
 * Copyright (c) 2013, Laminar Research.
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

#ifndef DSF2Text_H
#define DSF2Text_H

struct	DSFCallbacks_t;

// Scan a text file, shovel it into a writer.
bool Text2DSFWithWriter(const char * inFileName, DSFCallbacks_t * cbs, void * writer);

// Complete translation - text to binary.
bool Text2DSF(const char * inFileName, const char * inDSF);



struct print_funcs_s {
	int (* print_func)(void *, const char *, ...);
	void * ref;
};


// Want to write out DSF data as text?  This gives you writer callbacks
// that just print text...pass a print_funcs_s * as the ref.
void DSF2Text_CreateWriterCallbacks(DSFCallbacks_t * cbs);

// Complete tranlsation from binary to text.
bool DSF2Text(char ** inDSF, int n, const char * inFileName);


#endif /* DSF2Text_H */
