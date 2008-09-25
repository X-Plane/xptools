/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	UrlFile.h
*
******************************************************************/

#ifndef _CV97_URLFILE_H_
#define _CV97_URLFILE_H_

#include "JavaVM.h"
#include "String.h"

#ifdef SUPPORT_URL

class UrlFile : public JavaVM {

	static jclass		mUrlGetStreamClassID;
	static jmethodID	mUrlGetStreamInitMethodID;
	static jmethodID	mUrlGetStreamGetStreamMethodID;
	static jobject		mUrlGetStreamObject;

	String				*mUrl;
	String				*mUrlString;

public:

	UrlFile();
	~UrlFile();
	void	initialize();
	void	setUrl(char *urlString);
	char	*getUrl();
	bool	getStream(char *urlString);
	char	*getOutputFilename();
	bool	deleteOutputFilename();
};

#endif

#endif
