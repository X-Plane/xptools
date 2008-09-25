/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PROTO.cpp
*
******************************************************************/

#include "Proto.h"
#include "VRMLField.h"

#define FIELD_SEPARATOR_TOKENS	" \t\n"
#define PROTO_IGONRE_TOKENS			" \t"
#define PROTO_SEPARATOR_TOKENS	"{}[]\n"

////////////////////////////////////////////////
//	Parse Functions
////////////////////////////////////////////////

static int GetFieldTypeFromString(char *typeString)
{
	SFBool	field;
	field.setType(typeString);
	return field.getType();
}

static bool IsTokenChar(char c, char *tokenChars)
{
	if (tokenChars == NULL)
		return false;

	int tokenLen = strlen(tokenChars);
	for (int n=0; n<tokenLen; n++) {
		if  (c == tokenChars[n])
			return true;
	}

	return false;
}

static char *currentStringPos = NULL;

static char *GetStringToken(
char	*string,
char	*ignoreToken,
char	*separatorToken,
char	*buffer)
{
	if (string == NULL)
		string = currentStringPos;

	int stringLen = (int)strlen(string);
	int tokenLen = 0;

	int pos = 0;
	while (pos < stringLen && IsTokenChar(string[pos], ignoreToken))
		pos++;

	int startPos = pos;

	for (;pos < stringLen; pos++) {
		if (IsTokenChar(string[pos], ignoreToken) == true)
			break;
		if (IsTokenChar(string[pos], separatorToken) == true) {
			if (tokenLen == 0)
				tokenLen = 1;
			break;
		}
		tokenLen++;
	}

	if (tokenLen == 0)
		return NULL;

	strncpy(buffer, string + startPos, tokenLen);
	buffer[tokenLen] = '\0';
	currentStringPos = string + (pos + 1);

	return buffer;
}

////////////////////////////////////////////////
//	PROTO
////////////////////////////////////////////////

PROTO::PROTO(char *name, char *string, char *fieldString)
{
	setName(name);
	setString(string);
	addDefaultFields(fieldString);
}

PROTO::~PROTO(void)
{
}

void PROTO::setName(char *name)
{
	mName.setValue(name);
}

char *PROTO::getName(void)
{
	return mName.getValue();
}

void PROTO::setString(char *string)
{
	mString.setValue(string);
}

char *PROTO::getString()
{
	return mString.getValue();
}

void PROTO::addDefaultField(Field *field)
{
	mDefaultFieldVector.addElement(field);
}

void PROTO::addField(Field *field)
{
	mFieldVector.addElement(field);
}

int PROTO::getNDefaultFields()
{
	return mDefaultFieldVector.size();
}

int PROTO::getNFields()
{
	return mFieldVector.size();
}

Field *PROTO::getDefaultField(int n)
{
	return (Field *)mDefaultFieldVector.elementAt(n);
}

Field *PROTO::getField(int n)
{
	return (Field *)mFieldVector.elementAt(n);
}

void PROTO::addDefaultFields(char *fieldString)
{
	deleteDefaultFields();
	addFieldValues(fieldString, 1);
}

void PROTO::addFields(char *fieldString)
{
	deleteFields();
	addFieldValues(fieldString, 0);
}

void PROTO::deleteDefaultFields(void)
{
	mDefaultFieldVector.removeAllElements();
}

void PROTO::deleteFields(void)
{
	mFieldVector.removeAllElements();
}

void PROTO::addFieldValues(
char		*fieldString,
int			bDefaultField)
{
	char	string[256];
	char	fieldTypeName[32];
	char	fieldName[256];

	char *token = strtok(fieldString, FIELD_SEPARATOR_TOKENS);

	while( token != NULL ) {

		if (bDefaultField) {
			sscanf(token, "%s", string);
			if (strcmp(string, "field") != 0 && strcmp(string, "exposedField") != 0) {
				if (strcmp(string, "eventIn") != 0 && strcmp(string, "eventOut") != 0) {
					token = strtok( NULL, FIELD_SEPARATOR_TOKENS  );
					continue;
				}
			}
			/* Get field type */
			token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
			sscanf(token, "%s", fieldTypeName);
			token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
		}

		/* Get field name */
		sscanf(token, "%s", fieldName);

		int fieldType;
		if (bDefaultField)
			fieldType = GetFieldTypeFromString(fieldTypeName);
		else
			fieldType = getFieldType(fieldName);

		Field *field = NULL;

		switch (fieldType) {
		case fieldTypeSFString:
			{
				field = new SFString();
				token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
				((SFString *)field)->setValue(token);
				break;
			}
		case fieldTypeSFFloat:
			{
				field = new SFFloat();
				token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
				float value = (float)atof(token);
				((SFFloat *)field)->setValue(value);
				break;
			}
		case fieldTypeSFInt32:
			{
				field = new SFInt32();
				token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
				int value = atoi(token);
				((SFInt32 *)field)->setValue(value);
				break;
			}
		case fieldTypeSFVec2f:
			{
				field = new SFVec2f();
				float	vec2f[2];
				for (int n=0; n<2; n++) {
					token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
					vec2f[n] = (float)atof(token);
				}
				((SFVec2f *)field)->setValue(vec2f);
				break;
			}
		case fieldTypeSFVec3f:
			{
				field = new SFVec3f();
				float	vec3f[3];
				for (int n=0; n<3; n++) {
					token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
					vec3f[n] = (float)atof(token);
				}
				((SFVec3f *)field)->setValue(vec3f);
				break;
			}
		case fieldTypeSFColor:
			{
				field = new SFColor();
				float color[3];
				for (int n=0; n<3; n++) {
					token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
					color[n] = (float)atof(token);
				}
				((SFColor *)field)->setValue(color);
				break;
			}
		case fieldTypeSFBool:
			{
				field = new SFBool();
				token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
				bool btrue = !strcmp(token, "TRUE") ? true : false;
				((SFBool *)field)->setValue(btrue);
				break;
			}
		case fieldTypeSFRotation:
			{
				field = new SFRotation();
				float rot[4];
				for (int n=0; n<4; n++) {
					token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
					rot[n] = (float)atof(token);
				}
				((SFRotation *)field)->setValue(rot);
				break;
			}
		case fieldTypeSFTime:
			{
				field = new SFTime();
				token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
				double time = atof(token);
				((SFTime *)field)->setValue(time);
				break;
			}
		}

		//assert(field);

		if (field) {
			field->setName(fieldName);
			if (bDefaultField)
				addDefaultField(field);
			else
				addField(field);
		}

		token = strtok( NULL, FIELD_SEPARATOR_TOKENS);
	}

}

bool isTokenChar(char c)
{
	if ('a' <= c && c <= 'z')
		return true;
	if ('A' <= c && c <= 'Z')
		return true;
	if ('0' <= c && c <= '9')
		return true;
	if ('_' == c)
		return true;
	return false;
}

void PROTO::getString(char *returnBuffer)
{
	char tokenBuffer[512];

	returnBuffer[0] = '\0';

	char *string = getString();
	if (!string || !strlen(string))
		return;

//	char *defaultString = strdup(string);
	char *defaultString = new char[strlen(string)+1];
	strcpy(defaultString, string);

	//char *token = strtok(defaultString, PROTO_IGONRE_TOKENS);
	char *token = GetStringToken(defaultString, PROTO_IGONRE_TOKENS, PROTO_SEPARATOR_TOKENS, tokenBuffer);

	while( token != NULL ) {
		if (!strcmp(token, "IS")) {
			//token = strtok( NULL, PROTO_IGONRE_TOKENS  );
			token = GetStringToken(NULL, PROTO_IGONRE_TOKENS, PROTO_SEPARATOR_TOKENS, tokenBuffer);
			Field *field = getField(token);
			if (field) {
				static char	fieldValue[1024*10];
				field->getValue(fieldValue);
				sprintf(&returnBuffer[strlen(returnBuffer)], "%s ", fieldValue);
			}
			else {
				for (int n=(strlen(returnBuffer)-1-1); 0 <= n; n--) {
					if (isTokenChar(returnBuffer[n]) == false)
						break;
					returnBuffer[n] = '\0';
				}
			}
		}
		else
			sprintf(&returnBuffer[strlen(returnBuffer)], "%s ", token);

		//token = strtok( NULL, PROTO_IGONRE_TOKENS  );
		token = GetStringToken(NULL, PROTO_IGONRE_TOKENS, PROTO_SEPARATOR_TOKENS, tokenBuffer);
	}

//	free(defaultString);
	delete[] defaultString;
}

Field *PROTO::getField(char *name)
{
	Field	*field;
	int		n;

	int nField = getNFields();
	for (n = 0; n<nField; n++) {
		field = getField(n);
		if (!strcmp(field->getName(), name))
			return field;
	}

	int nDefaultField = getNDefaultFields();
	for (n = 0; n<nDefaultField; n++) {
		field = getDefaultField(n);
		if (!strcmp(field->getName(), name))
			return field;
	}

	return NULL;
}

int PROTO::getFieldType(char *name)
{
	int nDefaultField = getNDefaultFields();
	for (int n = 0; n<nDefaultField; n++) {
		Field *field = getDefaultField(n);
		if (!strcmp(field->getName(), name))
			return field->getType();
	}
	return fieldTypeNone;
}
