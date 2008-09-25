/*
 * Copyright (c) 2004, Laminar Research.
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
#include "XESInit.h"
#include "EnumSystem.h"
#include "NetTables.h"
#include "DEMTables.h"
#include "ObjTables.h"

static bool HandleVocab(const vector<string>& inTokenLine, void * inRef)
{
	string	token;
	if (TokenizeLine(inTokenLine," s", &token)==2)
	{
		if (LookupToken(token.c_str()) == -1)
			NewToken(token.c_str());
	}
	return true;
}

void	XESInit(void)
{
	InitEnumSystem();
	RegisterLineHandler("VOCAB", HandleVocab, NULL);
	LoadConfigFile("vocab.txt");

	int old_mark = gTokens.size();
	LoadNetFeatureTables();
	LoadDEMTables();
	LoadObjTables();
	int new_mark = gTokens.size();

	if (old_mark != new_mark)
	{
		string vocab_path = FindConfigFile("vocab.txt");
		FILE * vocab = fopen(vocab_path.c_str(), "a");
		if (vocab)
		{
			fprintf(vocab, "# Added automatically - new tokens introduced by user." CRLF);
			for (int n = old_mark; n < new_mark; ++n)
				fprintf(vocab, "VOCAB    %s" CRLF, FetchTokenString(n));
			fclose(vocab);
		}
	}
}