/* Crop Extractor
 * Copyright (C) 2011 Joel Granados <joel.granados@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "crexConfig.h"
#include <Python.h>
#include <structmember.h>
#include <sys/stat.h>
#include "crex.h"

#define CREX_RETPYERR( message ) \
{ \
    PyErr_SetString ( PyExc_StandardError, message ); \
    return NULL; \
}

/*{{{ Crim Object*/
typedef struct{
  PyObject_HEAD /* ";" provided by macro*/
  CroppedImage *ci;
} Crim;

static void
Crim_dealloc ( Crim *self )
{
  delete self->ci;
  self->ob_type->tp_free((PyObject*)self);
}

/* creates (does not instantiate) new objects*/
static PyObject*
Crim_new ( PyTypeObject *type, PyObject *args, PyObject *kwds )
{
  Crim *self;
  self = (Crim *)type->tp_alloc(type, 0);
  if ( self != NULL )
    self->ci = NULL;
  return (PyObject *)self;
}

/* instantiates new objects*/
static int
Crim_init ( Crim *self, PyObject *args, PyObject *kwds )
{ /*FIXME: is not supposed to be used from outside.*/ }

static PyObject*
Crim_get_label ( Crim *self )
{
  //FIXME: check for error.
  PyObject *py_label;
  py_label = PyString_FromFormat ( "%s", self->ci->get_label().data() );
  return py_label;
}

static PyObject*
Crim_save_to ( Crim *self, PyObject *args )
{
  char *output_file;
  if ( !PyArg_ParseTuple ( args, "s", &output_file ) )
    CREX_RETPYERR ( "Invalid parameter for Crim_save_to." );

  struct stat file_stat;
  if ( stat ( output_file, &file_stat ) == 0 )
    CREX_RETPYERR ( "Output file exists.  Will not overwrite." );

  if ( self->ci->saveTo ( (string)output_file ) )
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyMemberDef Crim_members[] = { {NULL} };

static PyMethodDef Crim_methods[] = {
  {"get_label", (PyCFunction)Crim_get_label, METH_NOARGS,
    "Return the label string for this cropped image."},
  {"save_to", (PyCFunction)Crim_save_to, METH_VARARGS,
    "Save this cropped image to filename."},
  {NULL}  /* Sentinel */
};

static PyTypeObject CrimType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "pycrex.Crim",             /*tp_name*/
  sizeof(Crim),              /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)Crim_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Crim objects",            /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  Crim_methods,              /* tp_methods */
  Crim_members,              /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)Crim_init,      /* tp_init */
  0,                         /* tp_alloc */
  Crim_new,                 /* tp_new */
};
/*}}} Crim Object*/

/*{{{ Crex Module Methods*/
static PyObject*
Crex_get_version ( PyObject *self, PyObject *args )
{
  PyObject * ver_mes;
  ver_mes = PyString_FromFormat (
    "%s, Version: %d.%d.", CREX_NAME, CREX_VER_MAJOR, CREX_VER_MINOR );
  return ver_mes;
}

static PyObject*
Crex_get_cropped_images ( PyObject *self, PyObject *args )
{
  char *img_file, *ann_file;

  if ( !PyArg_ParseTuple ( args, "ss", &img_file, &ann_file ) )
    CREX_RETPYERR ( "Invalid params for Crex_get_cropped_images." );

  struct stat file_stat;
  if ( stat ( img_file, &file_stat ) != 0 )
    CREX_RETPYERR ( "Image file does not exist." );
  if ( stat ( ann_file, &file_stat ) != 0 )
    CREX_RETPYERR ( "Annotation file does not exist." );

  CroppedImageExtractor cie = CroppedImageExtractor ( img_file, ann_file );
  cie.extractCroppedImages ();
  vector<CroppedImage*> cis = cie.getCroppedImages ();

  /* list that will hold the Crim objects*/
  PyObject *crims = PyList_New ( cis.size() );

  int i=0;
  for ( vector<CroppedImage*>::iterator iter=cis.begin() ;
        iter != cis.end() ; iter++, i++ )
  {
    Crim *temp;
    temp = PyObject_NEW ( Crim, &CrimType );
    temp->ci = (CroppedImage*)(*iter); /* (*iter) is CroppedImage address */

    if ( PyList_SetItem ( crims, i, (PyObject*)temp ) == -1 )
      CREX_RETPYERR ( "Error creating crims list elem." );
  }

  return crims;
}

static PyMethodDef Crex_methods[] = {
  {"version", (PyCFunction)Crex_get_version, METH_NOARGS,
    "Return the version of the library." },
  {"get_cropped_images", (PyCFunction)Crex_get_cropped_images, METH_VARARGS,
    "Return a list of Crim objects." },
  {NULL}  /* Sentinel */
};

PyMODINIT_FUNC
initpycrex (void)
{
  PyObject* m;

  if ( PyType_Ready(&CrimType) < 0 )
    return;

  m = Py_InitModule3 ( "pycrex", Crex_methods,
    "Module that extracts annotation sub-images from an image." );

  if ( m == NULL )
    return;

  Py_INCREF ( &CrimType );
  PyModule_AddObject ( m, "Crim", (PyObject *)&CrimType );
}
/*}}} Crex Module Methods*/
