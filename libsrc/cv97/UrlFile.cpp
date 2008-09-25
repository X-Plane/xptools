/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	UrlFile.cpp
*
******************************************************************/

#include <assert.h>
#include <string.h>
#ifdef WIN32
#include "windows.h"
#endif
#include "UrlFile.h"

#ifdef SUPPORT_URL

jclass		UrlFile::mUrlGetStreamClassID = NULL;
jmethodID	UrlFile::mUrlGetStreamInitMethodID = NULL;
jmethodID	UrlFile::mUrlGetStreamGetStreamMethodID = NULL;
jobject		UrlFile::mUrlGetStreamObject = NULL;

////////////////////////////////////////////////////////////
//	UrlFile::UrlFile
////////////////////////////////////////////////////////////

UrlFile::UrlFile()
{
	mUrl		= new JString();
	mUrlString	= new JString();

	initialize();
}

////////////////////////////////////////////////////////////
//	UrlFile::UrlFile
////////////////////////////////////////////////////////////

UrlFile::~UrlFile()
{
	delete	mUrl;
	delete	mUrlString;
}

////////////////////////////////////////////////////////////
//	UrlFile::initialize
////////////////////////////////////////////////////////////

#define URLGETSTREAM_FILENAME	"UrlGetStream"

void UrlFile::initialize()
{
	if (mUrlGetStreamObject == NULL) {

		JNIEnv	*jniEnv	= getJniEnv();

		if (jniEnv == NULL)
			return;

		mUrlGetStreamClassID = jniEnv->FindClass(URLGETSTREAM_FILENAME);
		assert(mUrlGetStreamClassID);

		mUrlGetStreamInitMethodID		= jniEnv->GetMethodID(mUrlGetStreamClassID, "<init>",		"()V");
		mUrlGetStreamGetStreamMethodID	= jniEnv->GetMethodID(mUrlGetStreamClassID, "getStream",	"(Ljava/lang/String;)Z");

		assert(mUrlGetStreamInitMethodID && mUrlGetStreamGetStreamMethodID);

		mUrlGetStreamObject = jniEnv->NewObject(mUrlGetStreamClassID, mUrlGetStreamInitMethodID);

		assert(mUrlGetStreamObject);
	}
}

////////////////////////////////////////////////////////////
//	UrlFile::getStream
////////////////////////////////////////////////////////////

void UrlFile::setUrl(char *urlString)
{
	int indexPeriod, indexSlash;

	if (urlString == NULL || strlen(urlString) == 0) {
		mUrl->setValue(urlString);
		return;
	}

	for (indexPeriod=strlen(urlString)-1; 0<=indexPeriod; indexPeriod--) {
		if (urlString[indexPeriod] == '.')
			break;
	}

	for (indexSlash=strlen(urlString)-1; 0<=indexSlash; indexSlash--) {
		if (urlString[indexSlash] == '/')
			break;
	}

	char *string = new char[strlen(urlString)+1+1];
	strcpy(string, urlString);

	if (indexSlash < indexPeriod) {
		string[indexSlash+1] = '\0';
	}

	if (string[strlen(string)-1] != '/') {
		string[strlen(string)] = '/';
		string[strlen(string)+1] = '\0';
	}

	mUrl->setValue(string);

	delete []string;
}

////////////////////////////////////////////////////////////
//	UrlFile::getStream
////////////////////////////////////////////////////////////

char *UrlFile::getUrl()
{
	return mUrl->getValue();
}


////////////////////////////////////////////////////////////
//	UrlFile::getStream
////////////////////////////////////////////////////////////

bool UrlFile::getStream(char *urlString)
{
	JNIEnv		*jniEnv	= getJniEnv();
	jboolean	result = 0;

	initialize();

	if (urlString && strlen(urlString)) {

		jstring	value = NULL, urlValue = NULL;

		value = jniEnv->NewStringUTF(urlString);
		result = jniEnv->CallBooleanMethod(mUrlGetStreamObject, mUrlGetStreamGetStreamMethodID, value);

		if (!result) {

			char *url = getUrl();

			if (url && strlen(url)) {

				char *newUrlString = new char [strlen(url) + strlen(urlString) + 1];
				strcpy(newUrlString, url);
				strcat(newUrlString, urlString);

				jstring	urlValue = NULL;

				urlValue = jniEnv->NewStringUTF(newUrlString);
				result = jniEnv->CallBooleanMethod(mUrlGetStreamObject, mUrlGetStreamGetStreamMethodID, urlValue);

				if (urlValue)
					jniEnv->DeleteLocalRef(urlValue);

				delete []newUrlString;
			}
		}

		if (value)
			jniEnv->DeleteLocalRef(value);
	}

	if (result)
		mUrlString->setValue(urlString);

	return (result ? true : false);
}

////////////////////////////////////////////////////////////
//	UrlFile::getOutputFilename
////////////////////////////////////////////////////////////

char *UrlFile::getOutputFilename()
{
	char *urlString = mUrlString->getValue();

	if (urlString == NULL)
		return NULL;

	for (int n=strlen(urlString)-1; 0<=n; n--) {
		if (urlString[n] == '/')
			break;
	}

	if (n < 0)
		return urlString;

	if (((int)strlen(urlString)-1) < n+1 )
		return NULL;

	return &urlString[n+1];
}

////////////////////////////////////////////////////////////
//	UrlFile::getOutputFilename
////////////////////////////////////////////////////////////

bool UrlFile::deleteOutputFilename()
{
	char *filename = getOutputFilename();
	if (!filename)
		return false;

	bool result = false;
#ifdef WIN32
	result = DeleteFile(filename) ? true : false;
#endif

	mUrlString->setValue(NULL);

	return result;
}

#endif
