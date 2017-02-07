#include <Python.h>
#include <string>
#include "SignParser_Canonicalize.h"

extern "C" {
	PyMODINIT_FUNC inittaxi_signs(void);
	static PyObject * py_canonicalize_sign(PyObject * self, PyObject * args);
}

static PyObject * SignError;
static PyMethodDef TaxiSignMethods[] = {
		{"canonicalize", py_canonicalize_sign, METH_VARARGS, "Canonicalize a taxi sign."},
		{NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC inittaxi_signs(void)
{
	PyObject * m = Py_InitModule("taxi_signs", TaxiSignMethods);
	if(m != NULL)
	{
		SignError = PyErr_NewException("taxi_signs.error", NULL, NULL);
		Py_INCREF(SignError);
		PyModule_AddObject(m, "error", SignError);
	}
}

//int main(int argc, char *argv[])
//{
//	/* Pass argv[0] to the Python interpreter */
//	Py_SetProgramName(argv[0]);
//
//	/* Initialize the Python interpreter.  Required. */
//	Py_Initialize();
//
//	/* Add a static module */
//	inittaxi_signs();
//}

static PyObject * py_canonicalize_sign(PyObject * self, PyObject * args)
{
	const char * sign_text;
	if(PyArg_ParseTuple(args, "s", &sign_text))
	{
		auto result = canonicalize_taxi_sign(string(sign_text));
		if(result.first.empty())
		{
			PyErr_SetString(SignError, result.second.c_str());
			return NULL;
		}
		else
		{
			return Py_BuildValue("s", result.first.c_str());
		}
	}
	else
	{
		Py_RETURN_NONE;
	}
}

