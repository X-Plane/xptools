#include <Python.h>
#include <string>
#include "SignParser_Canonicalize.h"

static constexpr const char * s_module_name = "xplane_taxi_sign";
static PyObject * py_canonicalize_sign(PyObject * self, PyObject * args);

extern "C" {
	static PyMethodDef taxi_sign_methods[] = {
		{"canonicalize", py_canonicalize_sign, METH_VARARGS, "Canonicalize a taxi sign."},
		{NULL, NULL, 0, NULL}        /* Sentinel */
	};

	static struct PyModuleDef taxi_sign_module = {
		PyModuleDef_HEAD_INIT,
		s_module_name,
		NULL, /* module documentation, may be NULL */
		-1,   /* size of per-interpreter state of the module,
				 or -1 if the module keeps state in global variables. */
		taxi_sign_methods
	};
}

static PyObject * SignError;

PyMODINIT_FUNC PyInit_xplane_taxi_sign(void)
{
	return PyModule_Create(&taxi_sign_module);
}

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

int main(int argc, char *argv[])
{
	wchar_t * program = Py_DecodeLocale(argv[0], NULL);
	if(!program)
	{
		fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
		exit(1);
	}
	
	/* Add a built-in module, before Py_Initialize */
	PyImport_AppendInittab(s_module_name, PyInit_xplane_taxi_sign);

	/* Pass argv[0] to the Python interpreter */
	Py_SetProgramName(program);
	
	/* Initialize the Python interpreter.  Required. */
	Py_Initialize();
	
	/* Optionally import the module; alternatively,
	 import can be deferred until the embedded script
	 imports it. */
	PyImport_ImportModule(s_module_name);
	
	PyMem_RawFree(program);
	return 0;
}

