#include <string>
#include <iostream>
#include <cstdlib>
#include "polyglob.h"

using namespace std;

//---
// This test program is to be run:
//
//  prog < file
//
// It will terminate when the file is read, and will report the language.
//---

int main(int argc, char **argv)
{
  string s;
  polyGLOB p;
  polyGLOBobj po;
 
  // Read input into a file.
  while (!cin.eof()) s += cin.get();
  
  // Just the language via the functor:
  cout << p(s) << endl;
  
  // More info:
  po = p.identify(s);
  
  // What you have seen before:
  cout << po.language << endl;

  // How far to the nearest other language?
  cout << po.distance << endl;

  // How certain?
  cout << po.confidence << endl;

  return EXIT_SUCCESS;
}

