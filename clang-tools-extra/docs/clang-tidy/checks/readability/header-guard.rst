.. title:: clang-tidy - readability-header-guard

readability-header-guard
========================

Finds and fixes header guards that do not adhere to a specified style.

Options
-------

.. option:: HeaderFileExtensions

   A comma-separated list of filename extensions of header files (the filename
   extensions should not include "." prefix). Default is "h,hh,hpp,hxx".
   For header files without an extension, use an empty string (if there are no
   other desired extensions) or leave an empty element in the list. E.g.,
   "h,hh,hpp,hxx," (note the trailing comma).

.. option:: Style

   The name of a header guard style to select. The default is "llvm". Available
   options are:

   ``llvm``

     Use the LLVM header guard style.
