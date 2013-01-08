//---
// polyGLOB: 1 December 2011, gkf created.
//           me@georgeflanagin.com
//---

#ifndef POLYGLOB_INCLUDED
#define POLYGLOB_INCLUDED

#include <string>
#include <vector>
#include <map>

using namespace std;

// externally callable, non-C++ interface.
extern "C" { 
  void identifyLanguage (const char *shred, char ** pLangID);
}

// a little package containing the answers.
struct polyGLOBobj
{
  double confidence;
  double distance;
  string language;
  vector<double> charDistances;
  map<string, double> languageDistances;
};

// Our polyGLOBber
class polyGLOB
{
  public:
    polyGLOB(void) : m_error(0) { } 
    ~polyGLOB(void) { }

    // for general use, the functor will do.
    string operator()(const string & sampleText, double dExp=2.0);

    // to get more details ..
    polyGLOBobj identify(const string & sampleText, double dExp=2.0);

    operator bool (void) { return !m_error; }
    bool operator ! (void) { return m_error != 0; }
    int error (void) { int i = m_error; m_error = 0; return i; }
    string errorMessage (void) { return m_errorMessage; }

    static const string kMYSTERY;
    static const string kENGLISH;
    static const string kFRENCH;
    static const string kSPANISH;
    static const string kGERMAN;
    static const string kPORTUGUESE;
    static const string kSWEDISH;
    static const string kITALIAN;

  protected:
    int m_error;
    string m_errorMessage;
    polyGLOBobj languageObj;
};

#endif

