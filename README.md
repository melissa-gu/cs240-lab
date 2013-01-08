polyGLOB
========

a C++ class/module to detect the likely human language in which text is written.

Currently, this module recognizes (*or if you prefer, disambiguates*) English, 
Spanish, French, German, Italian, Portuguese, Swedish. The default language is
"unknown". 

### Building the library:

1. `cd ~polyGLOB` ... whatever you name it.
1. `make`

### Using the test program example:

1. `g++ -std="c++0x" -o /usr/local/bin/polyGLOB *.cpp` ... for a *demo*, it is easiest to jam it all together.
2. `polyGLOB < some-file-of-text`


