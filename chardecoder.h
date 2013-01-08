#ifndef CHARDECODER_INCLUDED
#define CHARDECODER_INCLUDED

#include <iostream>

using namespace std;

#ifdef TEST_MODE
#define EMIT(x) cout << #x << " is \"" << (x) << "\" at line " << __LINE__ << endl
#else
#define EMIT(x)
#endif

//---
// 7 December 2011: gkf created.
//---
enum dgiEncoding_t { kASCII = 0, kUTF8 = 1 };

//---
// Achtung! Warning! Hay que cuidado! 
//   This is a virtual base class.
//---
class dgiCharDecoder_t
{
  public:
    dgiCharDecoder_t(void) : m_error(0) { }
    virtual ~dgiCharDecoder_t(void) { }
    virtual unsigned char getNextChar(istream & source) = 0; // you must derive

    operator bool (void) { return !m_error; }
    bool operator ! (void) { return m_error != 0; }
    int error (void) { int i = m_error; m_error = 0; return i; }

  protected:
    int m_error;
};


class dgiASCIIdecoder_t : public dgiCharDecoder_t
{
  public:
    dgiASCIIdecoder_t(void) : dgiCharDecoder_t() { }
    ~dgiASCIIdecoder_t(void) { }
    unsigned char getNextChar(istream & in); 
};

class dgiUTF8decoder_t : public dgiCharDecoder_t 
{
  public:
    dgiUTF8decoder_t(void) : dgiCharDecoder_t() { }
    ~dgiUTF8decoder_t(void) { }
    unsigned char getNextChar(istream & source);
};



#endif


