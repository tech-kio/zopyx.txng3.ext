/*
 TextIndexNG V 3
 The next generation TextIndex for Zope

 This software is governed by a license. See
 LICENSE.txt for the terms of this license.

 The stemmer code ("Snowball") is (C) 2001, Dr. Martin Porter and published
 under the license.
*/

#include "Python.h"
#include <stdlib.h>
#include "libstemmer.h"
#include "modules.h"

#if PY_MAJOR_VERSION >= 3
#define PY3K
#endif

typedef struct
{
  PyObject_HEAD struct sb_stemmer *stemmer;
}
Stemmer;


static void
Stemmer_dealloc (Stemmer * self)
{
  sb_stemmer_delete (self->stemmer);
//  PyMem_DEL (self);
  PyObject_DEL(self);
}


static PyObject *Stemmer_availableStemmers(Stemmer *self,PyObject*args)
{
    PyObject *list, *lang;
    int i = 0;

    list = PyList_New(0);

    /* int i = 0; */
    while (1) {
      const char * s = algorithm_names[i];
      if (s) {
          lang = PyBytes_FromString(s);
          PyList_Append(list, lang);
          Py_DECREF(lang);
          i++;
      }
      else
        break;
    }
    PyList_Sort(list);

    return list;
}


static PyObject *
Stemmer_stem (Stemmer * self, PyObject * args)
{
  PyObject *obj, *list, *item, *encoded, *u_stemmed;
  const sb_symbol *stemmed;
  char *word;
  Py_ssize_t i;
  Py_ssize_t len;

  if (self == NULL)
	  {
		PyErr_SetString (PyExc_TypeError,
						 "can not call stem() on unbound method");
		return NULL;
	  }

  if (!(PyArg_ParseTuple (args, "O", &obj)))
	return NULL;

  if (!PySequence_Check (obj))
	  {
		PyErr_SetString (PyExc_TypeError,
						 "stem() only accepts a sequence of unicode strings");
		return NULL;
	  }

  if (!(obj = PySequence_Fast (obj, "argument must be a sequence")))
   {
     return NULL;
   }

  len = PySequence_Length (obj);
  list = PyList_New (len);

  for (i = 0; i < len; i++)
	  {

		item = PySequence_Fast_GET_ITEM (obj, i);

		if (PyUnicode_Check (item))
			{

			  // Unicode -> UTF-8 (Python string)
			  encoded =
				PyUnicode_AsEncodedString (item, "UTF-8", "ignore");

			  // -> byte string
			  word = PyBytes_AS_STRING (encoded);

			  // now stem
			  stemmed =
				sb_stemmer_stem (self->stemmer, word, strlen (word));

			  Py_DECREF (encoded);

			  // convert back to unicode
			  u_stemmed =
				PyUnicode_DecodeUTF8 (stemmed,
									  sb_stemmer_length (self->stemmer),
									  "ignore");

			  PyList_SET_ITEM(list, i, u_stemmed); // steals the reference
			}
		else
			{
			  PyErr_SetString (PyExc_TypeError,
							   "Unsupported datatype found in list (only unicode string allowed)");
			  Py_DECREF(list);
			  Py_DECREF(obj);
			  return NULL;
			}
	  }

  Py_DECREF (obj);
  return list;

}


static struct PyMethodDef Stemmer_methods[] = {
  {"stem", (PyCFunction) Stemmer_stem, METH_VARARGS,
   "stem([word1, word2,..]) -- perform stemming against a sequence of *unicode strings*",
   },
  {NULL, NULL}					/* sentinel */
};


static char StemmerType__doc__[] = "Stemmer object";

static PyTypeObject StemmerType = {
#ifndef PY3K
  PyObject_HEAD_INIT (NULL) 0,	/*ob_size */
  "Stemmer",					/*tp_name */
#else
  PyObject_HEAD_INIT (NULL)
  "Stemmer",					/*tp_name */
#endif
  sizeof (Stemmer),				/*tp_basicsize */
  0,							/*tp_itemsize */
  /* methods */
  (destructor) Stemmer_dealloc,	/*tp_dealloc */
  (printfunc) 0,				/*tp_print */
  (getattrfunc) 0,	/*tp_getattr */
  (setattrfunc) 0,				/*tp_setattr */
  0,					/*tp_compare */
  (reprfunc) 0,					/*tp_repr */
  0,							/*tp_as_number */
  0,							/*tp_as_sequence */
  0,							/*tp_as_mapping */
  (hashfunc) 0,					/*tp_hash */
  (ternaryfunc) 0,				/*tp_call */
  (reprfunc) 0,					/*tp_str */

  /* Space for future expansion */
  0L, 0L, 0L, 0L,
  StemmerType__doc__,			/* Documentation string */
  /* tp_traverse       */ (traverseproc)0,
  /* tp_clear          */ (inquiry)0,
  /* tp_richcompare    */ (richcmpfunc)0,
  /* tp_weaklistoffset */ (long)0,
  (getiterfunc)0,		/*tp_iter*/
  /* tp_iternext       */ (iternextfunc)0,
  /* tp_methods        */ Stemmer_methods,
};



static PyObject *
newStemmer (PyObject * modinfo, PyObject * args)
{
  Stemmer *self = NULL;
  char *language;

  if (!(PyArg_ParseTuple (args, "s", &language)))
	return NULL;

  if (!(self = PyObject_NEW (Stemmer, &StemmerType)))
	return NULL;

  self->stemmer = sb_stemmer_new (language, "UTF_8");
  if (self->stemmer == 0)
	  {
        PyErr_Format(PyExc_ValueError,
                     "No stemmer available for language '%s'", language);
		Py_DECREF (self);
		return NULL;
	  }

  return (PyObject *) self;
}


static struct PyMethodDef stemmer_module_methods[] = {
  { "availableStemmers", (PyCFunction) Stemmer_availableStemmers, METH_VARARGS,
      "availableStemmers() -- Return a list of all available stemmers"
  },
  {"Stemmer", (PyCFunction) newStemmer,
   METH_VARARGS,
   "Stemmer(language) " "-- Return a new language specific stemmer"},

  {NULL, NULL}
};


#ifdef PY3K
static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "stemmer",                     /* m_name */
        "TextIndexNG stemmer module",  /* m_doc */
        -1,                             /* m_size */
        stemmer_module_methods,        /* m_methods */
        NULL,                           /* m_reload */
        NULL,                           /* m_traverse */
        NULL,                           /* m_clear */
        NULL,                           /* m_free */
    };
#endif

static PyObject*
module_init (void)
{
  PyObject *m;

  if (PyType_Ready(&StemmerType) < 0) {
	  return NULL;
  }

  /* Create the module and add the functions */
#ifdef PY3K
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule3 ("stemmer", stemmer_module_methods,
					  "TextIndexNG stemmer module");
#endif
  return m;
}

#ifdef PY3K
PyMODINIT_FUNC PyInit_stemmer(void)
{
    return module_init();
}
#else
PyMODINIT_FUNC initstemmer(void)
{
    module_init();
}
#endif
