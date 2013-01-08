#include "chardecoder.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <errno.h>
#include <iconv.h>
#include <istream>

using namespace std;

#ifdef ERRORBEGIN
#undef ERRORBEGIN
#endif
#define ERRORBEGIN (-215000)
#define UNIQUE_ERROR_CODE (ERRORBEGIN-__LINE__)


extern const char chardecoder[];
const char chardecoder[] = "@@@218@" __FILE__ "///" __DATE__ "///" __TIME__ "@@@";

enum utf8byteType_t 
{
  e_singleByte = 0, 
  e_multiByteData = 1,
  e_begin2byte = 2,
  e_begin3byte = 3,
  e_begin4byte = 4,
  e_overlongEncoding = -1,
  e_begin4byteRestricted = 40,
  e_begin5byteRestricted = 50,
  e_begin6byteRestricted = 60,
  e_invalid = -2
};

union utf8conversion_t
{
  utf8conversion_t(void) { iValue64 = 0; }
  int64_t iValue64;
  int32_t iValue32;
  char bytes[sizeof(int64_t)];
};

static const unsigned char bad(255);

static int ISOLatin1[256] =
{
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x00 
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x10
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x20
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x30
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x40
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x50
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x60
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x70
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x80
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x90
  32,32,99,36,  32,36,124,42, 32,32,32,34,  32,32,32,32,   // 0xA0
  32,43,32,32,  39,117,32,32, 32,32,32,34,  32,32,32,32,   // 0xB0
  65,65,65,65,  65,65,65,67,  69,69,69,69,  73,73,73,73,   // 0xC0
  68,78,79,79,  79,79,79,128, 79,85,85,85,  85,89,32,115,  // 0xD0
  97,97,97,97,  97,97,97,99, 101,101,101,101, 105,105,105,105, // 0xE0
  111,118,111,111,  111,111,111,32,  111,117,117,117,  117,121,32,121  //0xF0
};

static int gByteTable[256] =
{
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x00 
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x10
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x20
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x30
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x40
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x50
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x60
   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   // 0x70
//----------------------------------------------------------------
   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   // 0x80
   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   // 0x90
   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   // 0xA0
   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   1, 1, 1, 1,   // 0xB0
//----------------------------------------------------------------
  -1,-1, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   // 0xC0
   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   2, 2, 2, 2,   // 0xD0
//----------------------------------------------------------------
   3, 3, 3, 3,   3, 3, 3, 3,   3, 3, 3, 3,   3, 3, 3, 3,   // 0xE0
   4, 4, 4, 4,   4,                                        // 0xF0 .. 0xF4
                   49,49,49,                               // 0xF5 .. 0xF7
                              50,50,50,50,                 // 0xF8 .. 0xFB
                                            60,60,         // 0xFC .. 0xFD
                                                  -2,-2    // 0xFE .. 0xFF
};

unsigned char dgiASCIIdecoder_t::getNextChar(istream & in)
{
  unsigned char c;
  in >> c;
  if (!in) 
  { 
    m_error = UNIQUE_ERROR_CODE; 
    return ::bad; 
  }

  return c<128 ? c : static_cast<unsigned char>(ISOLatin1[static_cast<int>(c)]);  
}

unsigned char dgiUTF8decoder_t::getNextChar(istream & in)
{
  // statics up front.
  static const iconv_t toUNICODE = iconv_open("UNICODE", "UTF-8");

  if (toUNICODE == reinterpret_cast<iconv_t>(-1)) 
  { 
    m_error = UNIQUE_ERROR_CODE; 
    return ::bad; 
  }

  unsigned char uc;
  char c;
  int iResult;

  bool firstChar(true);
  utf8byteType_t t; 
  utf8conversion_t inbuf, outbuf;
  char * pIn(reinterpret_cast<char *>(&inbuf));
  char * pOut(reinterpret_cast<char *>(&outbuf));
  size_t uNumRead(0);
  size_t outbytesleft(sizeof(utf8conversion_t));

  int bytesToGo(0);
  
  // see if we can get to the bottom of this thing.
  while ((bytesToGo || firstChar) && 
          in.good() && 
          !in.eof() && 
          uNumRead < sizeof(utf8conversion_t))
  {
    in.read(&c, sizeof(c));
    uc = static_cast<unsigned char>(c);
    // EMIT( static_cast<int>(uc) );
    uNumRead++;
    t = static_cast<utf8byteType_t>(gByteTable[static_cast<int>(uc)]);
    // EMIT(t);
    switch (t)
    {
      case  e_singleByte:
        if (!firstChar) m_error = UNIQUE_ERROR_CODE;
        return firstChar ? uc : ::bad;
        break;

      case  e_multiByteData:
        if (firstChar) return ::bad;
        inbuf.bytes[uNumRead-1] = c;
        bytesToGo--;
        break;

      case  e_begin2byte:
        if (!firstChar) return ::bad;
        firstChar = false;
        bytesToGo = 1;
        inbuf.bytes[uNumRead-1] = c;
        break;

      case  e_begin3byte:
        if (!firstChar) return ::bad;
        firstChar = false;
        bytesToGo = 2;
        inbuf.bytes[uNumRead-1] = c;
        break;

      case  e_begin4byte:
      case  e_begin4byteRestricted:
        if (!firstChar) return ::bad;
        firstChar = false;
        bytesToGo = 3;
        inbuf.bytes[uNumRead-1] = c;
        break;

      case  e_begin5byteRestricted:
        if (!firstChar) return ::bad;
        firstChar = false;
        bytesToGo = 4;
        inbuf.bytes[uNumRead-1] = c;
        break;

      case  e_begin6byteRestricted:
        if (!firstChar) return ::bad;
        firstChar = false;
        bytesToGo = 5;
        inbuf.bytes[uNumRead-1] = c;
        break;

      case  e_overlongEncoding:
        m_error = UNIQUE_ERROR_CODE;
        break;

      case  e_invalid:
        m_error = UNIQUE_ERROR_CODE;
        break;

      default:
        m_error = UNIQUE_ERROR_CODE;
        break;
    }
  }

  // So, now we are here, but how did we arrive? Candidate answers are:
  // input went bad.
  if (in.bad()) m_error = UNIQUE_ERROR_CODE;
  // we were in the middle of something, and got EOF.
  if (in.eof() && bytesToGo) m_error = UNIQUE_ERROR_CODE;
  // we have read more multibyte data than is possible.
  if (uNumRead == sizeof(utf8conversion_t)) m_error = UNIQUE_ERROR_CODE;
  if (m_error) return ::bad;
  if (in.eof()) return 0;

  // It appears everything went well. Attempt the conversion.
  iResult = iconv(toUNICODE, &pIn, &uNumRead, &pOut, &outbytesleft); 
  EMIT(iResult);
  // if for some reason this function doesn't seem to be working correctly,
  // remove the first static cast in the next line, and accept the compiler
  // warning.
  if (static_cast<size_t>(iResult) == static_cast<size_t>(-1)) 
  {
    switch (errno)
    {
      case EINVAL:
        m_error = UNIQUE_ERROR_CODE;
        break;
      case EILSEQ:
        m_error = UNIQUE_ERROR_CODE;
        break;
      case E2BIG:
        m_error = UNIQUE_ERROR_CODE;
        break;
      default:
        m_error = UNIQUE_ERROR_CODE;
        break;
    }
  }

  // And so, the conversion went well.
  switch (outbuf.iValue32)
  {
    // left quotes
    case 8249:
    case 8220:
    case 8222:
    case 8218:
      uc = 0x22;
      break;

    // unambiguous right quotes
    case 8250:
    case 8221:
      uc = 0x22;
      break;

    // left single quotes
    case 8216:
      uc = 0x27;
      break;

    // right single quotes
    case 8217:
      uc = 0x27;
      break;  
   
    default:
      uc = outbuf.iValue32 < 256 ? 
           static_cast<unsigned char>(ISOLatin1[outbuf.iValue32]) :
           static_cast<unsigned char>(32);
      break;
  }
  return uc;
}


