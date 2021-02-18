# cmdlineflags

Command line flags (or options) are flags/options passed to the program during its invocation.
Please have a look at the following examples:

```
  (1) $ git merge --help
```
```
  (2) $ git merge --no-ff origin/feature/1234567
```
```
  (3) $ ip -h
```
```
  (4) $ ip route help
```

Probably the most common library to parse such command line flags is POSIX's getopt().
Using getopt() terminology an element that starts with '-' (and is not exactly '-')
or element that starts with '--' is an option element.
Thus in the above example we have:
 (1) 1 option element (help) and 1 non-option element (merge)
 (2) 1 option element (no-ff) and 2 non-option elements (merge, origin/feature/1234567)
 (3) 1 option element (h) and no non-options
 (4) 2 non-option elements

 And here is the first major difference between cmdlineflags library and getopt().
 In cmdlineflags the first non-option element (if registered) is called a module
 and it serves as a namespace identifier for the following options.
 Having said so, we may easly distinguish '--help' option in the following 2 invocations:

```
  $ git merge --help
```
```
  $ git rebase --help
```

First --help belongs to the 'merge' module whereas the second one to the 'rebase' module.
Or I'm exaggerating a bit. getopt() will handle such cases equally well.

But the second distinction from getopt() seems to be more significant.
When using cmdlineflags library all flag definitions can be scattered around the entire
source code and not just listed in one place such as main().
In practie, you (as a developer) are free to define and then use/parse any flag
in any source file of your project. This is in my opinion nice and desirable feature.
It definitely improves flexibility and makes that code reuse can be really smooth process.
You can define and use flags only in places where presence of such flags make sence.

As it usually happens, there are also drawbacks of such architechture.
Two (or more) translation units can define the same flag independently
and each of them will receive notification when such flag will be provided on the command line.
None of the translation units will know about the same flag being defined in other of them.

## How to build this library

First of all this library has only been tested with decent versions of gcc (9.3.0) and clang (10.0.0).
And because it uses compiler specific extensions it might not work with other compilers.
The second thing is, cmdlineflags uses cmake as a build system. So its integration to a project
which also uses cmake is terribly easy. And as it is with cmake this can be accomplished twofold.
You can either install cmdlineflags in your system

```
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
  $ sudo make install
  $ sudo ldconfig
```

and then in the project which is going to use cmdlineflags you add/find it by cmake command

```
  $ find_package(cmdlineflags REQUIRED)
```

Or, you can include cmdlineflags as subdirectory or submodule to your project's source tree
and then use cmake's add_subdirectory() command. For example:

```
  $ add_subdirectory(cmdlineflags)
```

Regardless which method you use to include cmdlineflags to your project, don't forget to link agains it.

```
   add_executable(your_project_name main.c other.c project.c files.c)
   target_link_libraries(your_project_name cmdlineflags)
```
