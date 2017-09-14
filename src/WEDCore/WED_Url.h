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

#ifndef WED_Url_H
#define WED_Url_H

// These are redirect addresses for WED documentation - the URL just has a 302
// pointing to the real online resources.  This gives us flexibility to
// reorg the doc site in the future without borking WED.
#define WED_URL_MANUAL	"http://lookup.x-plane.com/_lookup_10_/wed_14_manual.html"
#define WED_URL_HELP_SCENERY "http://lookup.x-plane.com/_lookup_10_/wed_14_scenery.html"
#define WED_URL_UPLOAD_OK "http://lookup.x-plane.com/_lookup_10_/wed_14_upload_ok.html"

// This is the canonical service base URL for the gateway.
#define WED_URL_GATEWAY_API "https://gatewayapi.x-plane.com:3001/apiv1/"
#define WED_URL_AIRPORT_METADATA_CSV "https://gatewayapi.x-plane.com:3001/airport_metadata.csv"

#define WED_URL_CIFP_RUNWAYS "https://gatewayapi.x-plane.com:3001/runway_coordinates.txt"

#endif /* WED_Url_H */
