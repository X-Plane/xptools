/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef WED_ConvertCommands_H
#define WED_ConvertCommands_H

class	IResolver;
class	ISelection;
class	WED_LibraryMgr;
class	WED_Archive;
class	WED_Thing;

// int		WED_DoConvertToJW(WED_Airport* apt, int statistics[4] = nullptr);

// Tests if 'thing' is of a specific type.
typedef bool (*IsTypeFunc)(WED_Thing* thing);

// Factory function for a certain subclass of WED_Thing.
typedef WED_Thing* (*CreateThingFunc)(WED_Archive* parent);

// Checks whether the selected objects can be converted to the type checked for by 'isDstType'.
int		WED_CanConvertTo(IResolver * resolver, const char* DstType);

// Converts the selected objects to the type produced by 'create'.
void	WED_DoConvertTo(IResolver * resolver, CreateThingFunc create);
bool	WED_ConvertTo(WED_LibraryMgr* lmgr, ISelection* sel, CreateThingFunc create);

void	WED_DoConvertToForest(IResolver* resolver);

#endif /* WED_ConvertCommands_H */
