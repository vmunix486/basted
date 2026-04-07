# BASTED

![basted-linux-readme.png](https://github.com/vmunix486/basted/blob/b628c9d6a746fe728546c8e3864ec338adfbf465/basted-linux-readme.png)

This is BASTED. BASTED is actually an acronym for the BASic Text EDitor. The minimum requirements are:

 - UNIX or UNIX-like operating system (such as Linux or NetBSD)
 - X11
 - GCC 4.5 (for now)
 - (OPTIONAL) GNU make

There are many ways to compile BASTED. The first and easiest way is with the ``make`` command. There are many ways to compile it with ``make``.

 - ``make O1``: Compiles BASTED with the ``-O1`` option.
 - ``make O2``: Compiles BASTED with the ``-O2`` option.
 - ``make O3``: Compiles BASTED with the ``-O3`` option.
 - ``make O3-flto``: Compiles BASTED with ``-flto`` and ``-O3``. Runs the fastest.
 - ``make Os``: Compiles BASTED with ``-Os``. Optimised for smallest size.
 - ``make``: Compiles all of the above.

If you do not have GNU make, you can also do a manual compilation. Just run

``cc src/basted.c -lX11 -std=c99 -o basted``

If you want to add any options, I recommend adding them between ``-lX11`` and ``-o basted``.
