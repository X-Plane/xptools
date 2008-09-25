/******************************************************************
*
*	VRML library for C++
*
*	Copyright (C) Satoshi Konno 1996-1997
*
*	File:	CJavaVM.h
*
******************************************************************/

#ifndef _CJAVAVM_H_
#define _CJAVAVM_H_

#ifdef SUPPORT_JSAI

#include <stdio.h>
#include <stdarg.h>
#include <jni.h>

JavaVM	*GetJavaVM(void);
JNIEnv	*GetJniEnv(void);
void	SetJavaVM(JavaVM *jvm);
void	SetJniEnv(JNIEnv *jniEnv);
void	CreateJavaVM(char *classpath = NULL, jint (JNICALL *printfn)(FILE *fp, const char *format, va_list args) = NULL);
void	DeleteJavaVM(void);

class CJavaVM {

public:
	JavaVM *getJavaVM(void) {
		return GetJavaVM();
	}

	JNIEnv *getJniEnv(void) {
		return GetJniEnv();
	}
};

#endif

#endif

