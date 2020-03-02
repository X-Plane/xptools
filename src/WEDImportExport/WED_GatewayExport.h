/* 
 * Copyright (c) 2014, Laminar Research.
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

#ifndef WED_GatewayExport_H
#define WED_GatewayExport_H

#if HAS_GATEWAY
class	IResolver;
class	WED_Document;
class WED_Thing;
class WED_Airport;

int		WED_CanExportToGateway(IResolver * resolver);
void	WED_DoExportToGateway(WED_Document * resolver);

bool GatewayExport_has_3d(WED_Airport * who);
void Enforce_MetaDataGuiLabel(WED_Airport * apt);
void EnforceRecursive_MetaDataGuiLabel(WED_Thing * thing);

const string WED_get_GW_api_url();
const string WED_get_GW_cert();

#endif

#endif
