#include "test.h"

using namespace Magick;


PIXM_PyImage pixmagick_test(PyObject *self, PyObject *args) {
    PyObject *pyArg1;
    char *path;
    path = PyBytes_AsString(PyUnicode_AsMBCSString(pyArg1));
    Image *image = new Image(path);
    return image->image();  /* real content are still kept in memory, not a real leak, but will be deallocated by pmgToPix function */
}
