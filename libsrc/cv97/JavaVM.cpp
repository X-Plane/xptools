/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	JavaVM.cpp
*
******************************************************************/

#ifdef SUPPORT_JSAI

#include <stdlib.h>
#include <assert.h>
#include "CJavaVM.h"

static	JavaVM	*gJavaVM	= NULL;
static	JNIEnv	*gJavaEnv	= NULL;

JavaVM *GetJavaVM(void)
{
	return gJavaVM;
}

JNIEnv *GetJniEnv(void)
{
	return gJavaEnv;
}

void SetJavaVM(JavaVM *jvm)
{
	gJavaVM = jvm;
}

void SetJniEnv(JNIEnv *jniEnv)
{
	gJavaEnv = jniEnv;
}

void CreateJavaVM(char *classpath, jint (JNICALL *printfn)(FILE *fp, const char *format, va_list args))
{
#ifdef USE_JDK12
	if (!gJavaVM && !gJavaEnv) {

		JavaVMOption options[2];
		JavaVMInitArgs vm_args;

		int	nOptions = 1;

		options[0].name = "classpath";
		if (!classpath)
			options[0].value.p = getenv("CLASSPATH");
		else
			options[0].value.p = classpath;

		if (printfn) {
			options[1].name = "vfprintf";
			options[1].value.p = printfn;
			nOptions++;
		}

		vm_args.version = JNI_VERSION_1_2;
		vm_args.options = options;
		vm_args.nOptions = nOptions;
		vm_args.result = NULL;


		JNI_CreateJavaVM(&gJavaVM, (void **)&gJavaEnv, (void *)&vm_args);
		assert(gJavaVM && gJavaEnv);
	}
#else
	if (!gJavaVM && !gJavaEnv) {
		JDK1_1InitArgs vm_args;
		JNI_GetDefaultJavaVMInitArgs(&vm_args);

		if (!classpath)
			vm_args.classpath = getenv("CLASSPATH");
		else
			vm_args.classpath = classpath;

		if (printfn)
			vm_args.vfprintf = printfn;

		JNI_CreateJavaVM(&gJavaVM, &gJavaEnv, &vm_args);

		assert(gJavaVM && gJavaEnv);
	}
#endif
}

void DeleteJavaVM(void)
{
	if (gJavaVM)
		gJavaVM->DestroyJavaVM();
}

#endif