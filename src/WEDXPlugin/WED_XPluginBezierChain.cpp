/*
 * Copyright (c) 2012, mroe.
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
#include "WED_XPluginBezierChain.h"
#include "WED_XPluginBezierNode.h"

WED_XPluginBezierChain::WED_XPluginBezierChain()
{
    //ctor
}

WED_XPluginBezierChain::~WED_XPluginBezierChain()
{
    vector<WED_XPluginBezierNode *>::iterator it ;
    for(it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        if(*it) (*it)->SetParent(NULL);
    }
}

void  WED_XPluginBezierChain::AddNode(WED_XPluginBezierNode * inNode)
{
    unsigned int idx = inNode->GetIdx();
    bool IsExists = false;

    vector<WED_XPluginBezierNode *>::iterator it;

    for(it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        if(idx == (*it)->GetIdx())
        {
            it = mNodes.insert(it,inNode);
            IsExists = true;
            break;
        }
        else if(idx < (*it)->GetIdx())
        {
            mNodes.insert(it,inNode);
            break;
        }
    }

    if(IsExists)
    {
        for(++it; it != mNodes.end(); ++it)
        {
            (*it)->SetIdx((*it)->GetIdx()+1);
        }
    }
    else
    {
        if(it == mNodes.end()) mNodes.push_back(inNode);
    }
}

void  WED_XPluginBezierChain::DelNode(WED_XPluginBezierNode * inNode)
{
    vector<WED_XPluginBezierNode *>::iterator it ;
    for(it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        if(*it == inNode)
        {
            mNodes.erase(it);
            for(; it != mNodes.end(); ++it)
            {
                (*it)->SetIdx((*it)->GetIdx()-1);
            }
            break;
        }
    }
}
