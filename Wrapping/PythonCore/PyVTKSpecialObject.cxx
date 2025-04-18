// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/*-----------------------------------------------------------------------
  The PyVTKSpecialObject was created in Feb 2001 by David Gobbi.
  It was substantially updated in April 2010 by David Gobbi.

  A PyVTKSpecialObject is a python object that represents an object
  that belongs to one of the special classes in VTK, that is, classes
  that are not derived from vtkObjectBase.  Unlike vtkObjects, these
  special objects are not reference counted: a PyVTKSpecialObject
  always contains its own copy of the C++ object.

  The PyVTKSpecialType is a simple structure that contains information
  about the PyVTKSpecialObject type that cannot be stored in python's
  PyTypeObject struct.  Each PyVTKSpecialObject contains a pointer to
  its PyVTKSpecialType. The PyVTKSpecialTypes are also stored in a map
  in vtkPythonUtil.cxx, so that they can be lookup up by name.
-----------------------------------------------------------------------*/

#include "PyVTKSpecialObject.h"
#include "PyVTKMethodDescriptor.h"
#include "vtkABINamespace.h"
#include "vtkPythonUtil.h"

#include <sstream>

// Silence warning like
// "dereferencing type-punned pointer will break strict-aliasing rules"
// it happens because this kind of expression: (long *)&ptr
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

VTK_ABI_NAMESPACE_BEGIN
//------------------------------------------------------------------------------
PyVTKSpecialType::PyVTKSpecialType(
  PyTypeObject* typeobj, PyMethodDef* cmethods, PyMethodDef* ccons, vtkcopyfunc copyfunc)
{
  this->py_type = typeobj;
  this->vtk_methods = cmethods;
  this->vtk_constructors = ccons;
  this->vtk_copy = copyfunc;
}
VTK_ABI_NAMESPACE_END

//------------------------------------------------------------------------------
// Object protocol

//------------------------------------------------------------------------------
PyObject* PyVTKSpecialObject_Repr(PyObject* self)
{
  PyVTKSpecialObject* obj = (PyVTKSpecialObject*)self;
  PyTypeObject* type = Py_TYPE(self);
  const char* name = vtkPythonUtil::GetTypeName(type);

#if PY_VERSION_HEX >= 0x030A0000
  while (PyType_GetSlot(type, Py_tp_base) && !PyType_GetSlot(type, Py_tp_str))
  {
    type = (PyTypeObject*)PyType_GetSlot(type, Py_tp_base);
  }
#else
  while (type->tp_base && !type->tp_str)
  {
    type = type->tp_base;
  }
#endif

  // use str() if available
  PyObject* s = nullptr;
#if PY_VERSION_HEX >= 0x030A0000
  reprfunc type_str = (reprfunc)PyType_GetSlot(type, Py_tp_str);
  reprfunc base_type_str = (reprfunc)PyType_GetSlot(&PyBaseObject_Type, Py_tp_str);
  if (type_str && type_str != base_type_str)
#else
  if (type->tp_str && type->tp_str != (&PyBaseObject_Type)->tp_str)
#endif
  {
    PyObject* t =
#if PY_VERSION_HEX >= 0x030A0000
      type_str(self)
#else
      type->tp_str(self)
#endif
      ;
    if (t == nullptr)
    {
      Py_XDECREF(s);
      s = nullptr;
    }
    else
    {
      s = PyUnicode_FromFormat("%s(%S)", name, t);
    }
  }
  // otherwise just print address of object
  else if (obj->vtk_ptr)
  {
    s = PyUnicode_FromFormat("<%s(%p) at %p>", name, obj->vtk_ptr, static_cast<void*>(obj));
  }

  return s;
}

//------------------------------------------------------------------------------
PyObject* PyVTKSpecialObject_SequenceString(PyObject* self)
{
  Py_ssize_t n, i;
  PyObject* s = nullptr;
  PyObject *t, *o, *comma;
  const char* bracket = "[...]";

#if PY_VERSION_HEX >= 0x030A0000
  PyTypeObject* type = Py_TYPE(self);
  ssizeargfunc sq_item = (ssizeargfunc)PyType_GetSlot(type, Py_sq_item);
  ssizeobjargproc sq_ass_item = (ssizeobjargproc)PyType_GetSlot(type, Py_sq_ass_item);
  if (sq_item != nullptr && sq_ass_item == nullptr)
#else
  if (Py_TYPE(self)->tp_as_sequence && Py_TYPE(self)->tp_as_sequence->sq_item != nullptr &&
    Py_TYPE(self)->tp_as_sequence->sq_ass_item == nullptr)
#endif
  {
    bracket = "(...)";
  }

  i = Py_ReprEnter(self);
  if (i < 0)
  {
    return nullptr;
  }
  else if (i > 0)
  {
    return PyUnicode_FromString(bracket);
  }

  n = PySequence_Size(self);
  if (n >= 0)
  {
    comma = PyUnicode_FromString(", ");
    s = PyUnicode_FromStringAndSize(bracket, 1);

    for (i = 0; i < n && s != nullptr; i++)
    {
      if (i > 0)
      {
        PyObject* tmp = PyUnicode_Concat(s, comma);
        Py_DECREF(s);
        s = tmp;
      }
      o = PySequence_GetItem(self, i);
      t = nullptr;
      if (o)
      {
        t = PyObject_Repr(o);
        Py_DECREF(o);
      }
      if (t)
      {
        PyObject* tmp = PyUnicode_Concat(s, t);
        Py_DECREF(s);
        Py_DECREF(t);
        s = tmp;
      }
      else
      {
        Py_DECREF(s);
        s = nullptr;
      }
      n = PySequence_Size(self);
    }

    if (s)
    {
      PyObject* tmp1 = PyUnicode_FromStringAndSize(&bracket[4], 1);
      PyObject* tmp2 = PyUnicode_Concat(s, tmp1);
      Py_DECREF(s);
      Py_DECREF(tmp1);
      s = tmp2;
    }

    Py_DECREF(comma);
  }

  Py_ReprLeave(self);

  return s;
}

//------------------------------------------------------------------------------
// C API

//------------------------------------------------------------------------------
// Create a new python object from the pointer to a C++ object
PyObject* PyVTKSpecialObject_New(const char* classname, void* ptr)
{
  // would be nice if "info" could be passed instead if "classname",
  // but this way of doing things is more dynamic if less efficient
  PyVTKSpecialType* info = vtkPythonUtil::FindSpecialType(classname);

  PyVTKSpecialObject* self = PyObject_New(PyVTKSpecialObject, info->py_type);

  self->vtk_info = info;
  self->vtk_ptr = ptr;
  self->vtk_hash = -1;

  return (PyObject*)self;
}

//------------------------------------------------------------------------------
// Create a new python object via the copy constructor of the C++ object
PyObject* PyVTKSpecialObject_CopyNew(const char* classname, const void* ptr)
{
  PyVTKSpecialType* info = vtkPythonUtil::FindSpecialType(classname);

  if (info == nullptr)
  {
    return PyErr_Format(PyExc_ValueError, "cannot create object of unknown type \"%s\"", classname);
  }
  else if (info->vtk_copy == nullptr)
  {
    return PyErr_Format(
      PyExc_ValueError, "no copy constructor for object of type \"%s\"", classname);
  }

  PyVTKSpecialObject* self = PyObject_New(PyVTKSpecialObject, info->py_type);

  self->vtk_info = info;
  self->vtk_ptr = info->vtk_copy(ptr);
  self->vtk_hash = -1;

  return (PyObject*)self;
}

//------------------------------------------------------------------------------
// Add a special type, add methods and members to its type object.
// A return value of nullptr signifies that it was already added.
PyTypeObject* PyVTKSpecialType_Add(PyTypeObject* pytype, PyMethodDef* methods, PyGetSetDef* getsets,
  PyMethodDef* constructors, vtkcopyfunc copyfunc)
{
  // Check whether the type is already in the map (use classname as key),
  // and return it if so.  If not, then add it to the map.
  pytype = vtkPythonUtil::AddSpecialTypeToMap(pytype, methods, constructors, copyfunc);

  // If type object already has a dict, we're done
  if (pytype->tp_dict)
  {
    return pytype;
  }

  // Create the dict
  pytype->tp_dict = PyDict_New();

  // Add all of the methods
  for (PyMethodDef* meth = methods; meth && meth->ml_name; meth++)
  {
    PyObject* func = PyVTKMethodDescriptor_New(pytype, meth);
    PyDict_SetItemString(pytype->tp_dict, meth->ml_name, func);
    Py_DECREF(func);
  }

  // Add all of the getsets
  for (PyGetSetDef* getset = getsets; getset && getset->name; getset++)
  {
    PyObject* descr = PyDescr_NewGetSet(pytype, getset);
    PyDict_SetItemString(pytype->tp_dict, getset->name, descr);
    Py_DECREF(descr);
  }

  return pytype;
}
