//---
// polyGLOB: 1 December 2011, gkf created.
//           me@georgeflanagin.com
//---

#include <algorithm>
#include <cfloat>
#include "chardecoder.h"
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <errno.h>
#include <iconv.h>
#include <map>
#include "polyglob.h"
#include <sstream>
#include <string>
#include <valarray>
#include <vector>

using namespace std;

#define ONE26TH (3.846)

const string polyGLOB::kMYSTERY = "??";
const string polyGLOB::kENGLISH = "en";
const string polyGLOB::kFRENCH = "fr";
const string polyGLOB::kSPANISH = "es";
const string polyGLOB::kGERMAN = "de";
const string polyGLOB::kITALIAN = "it";
const string polyGLOB::kSWEDISH = "sv";
const string polyGLOB::kPORTUGUESE = "pt";

typedef map<char, int> CharFreq_t;
typedef vector<double> LangLetterFreq_t;
typedef map<string, LangLetterFreq_t &> LangLetterFreqMap_t;

static LangLetterFreq_t vectorize(CharFreq_t & m, double d);
static double vectorDistance(const LangLetterFreq_t & v1, 
                             const LangLetterFreq_t & v2, 
                             double dExp = 2.0);

//---
// The mystery letters are here as a boundary. After all, we need
// a null-hypothesis to make sure our search is doing what it 
// ought to do. For example, if we had no mystery language, then 
// we might find that gibberish would most closely resemble 
// English when it is no such thing.
//---
static LangLetterFreq_t MysteryLetters = {
  1.0, 1.0, 1.0, 1.0, 1.0, // a - e
  1.0, 1.0, 1.0, 1.0, 1.0, // f - j
  1.0, 1.0, 1.0, 1.0, 1.0, // k - o
  1.0, 1.0, 1.0, 1.0, 1.0, // p - t
  1.0, 1.0, 1.0, 1.0, 1.0, // u - y 
  1.0                      // z
};

//---
// The letter frequencies in published materials are usually written
// as percentages. We divide by 3.846 (ONE26TH) so that each letter's
// frequency is represented as how much more or less frequent it is
// than an average letter.
//---
static LangLetterFreq_t EnglishLetters = { 
  8.167/ONE26TH, 1.492/ONE26TH, 2.782/ONE26TH, 4.253/ONE26TH, 12.70/ONE26TH,
  2.228/ONE26TH, 2.015/ONE26TH, 6.094/ONE26TH, 6.996/ONE26TH, 0.153/ONE26TH, 
  0.772/ONE26TH, 4.025/ONE26TH, 2.406/ONE26TH, 6.749/ONE26TH, 7.507/ONE26TH, 
  1.929/ONE26TH, 0.095/ONE26TH, 5.987/ONE26TH, 6.327/ONE26TH, 9.056/ONE26TH, 
  2.758/ONE26TH, 0.978/ONE26TH, 2.360/ONE26TH, 0.150/ONE26TH, 1.974/ONE26TH, 
  0.074/ONE26TH                              
};

static LangLetterFreq_t SpanishLetters = {
  12.53/ONE26TH, 1.420/ONE26TH, 4.680/ONE26TH, 5.860/ONE26TH, 13.68/ONE26TH,
  0.690/ONE26TH, 1.010/ONE26TH, 0.700/ONE26TH, 6.250/ONE26TH, 0.440/ONE26TH,
  0.010/ONE26TH, 4.970/ONE26TH, 3.150/ONE26TH, 6.710/ONE26TH, 8.680/ONE26TH,
  2.510/ONE26TH, 0.880/ONE26TH, 6.870/ONE26TH, 7.980/ONE26TH, 4.630/ONE26TH,
  3.930/ONE26TH, 0.900/ONE26TH, 0.020/ONE26TH, 0.220/ONE26TH, 0.900/ONE26TH,
  0.520/ONE26TH
};

static LangLetterFreq_t GermanLetters = {
  6.510/ONE26TH, 1.890/ONE26TH, 3.060/ONE26TH, 5.080/ONE26TH, 17.40/ONE26TH,
  1.660/ONE26TH, 3.010/ONE26TH, 4.760/ONE26TH, 7.550/ONE26TH, 0.270/ONE26TH,
  1.210/ONE26TH, 3.440/ONE26TH, 2.530/ONE26TH, 9.780/ONE26TH, 2.510/ONE26TH,
  0.790/ONE26TH, 0.020/ONE26TH, 7.000/ONE26TH, 7.270/ONE26TH, 6.150/ONE26TH, 
  4.350/ONE26TH, 0.670/ONE26TH, 1.890/ONE26TH, 0.030/ONE26TH, 0.040/ONE26TH,
  1.130/ONE26TH
};

static LangLetterFreq_t FrenchLetters = {
  7.636/ONE26TH, 0.901/ONE26TH, 3.260/ONE26TH, 3.669/ONE26TH, 14.72/ONE26TH, 
  1.066/ONE26TH, 0.866/ONE26TH, 0.737/ONE26TH, 7.529/ONE26TH, 0.545/ONE26TH,
  0.049/ONE26TH, 5.456/ONE26TH, 2.968/ONE26TH, 7.095/ONE26TH, 5.378/ONE26TH,
  3.021/ONE26TH, 1.362/ONE26TH, 6.553/ONE26TH, 7.948/ONE26TH, 7.244/ONE26TH,
  6.311/ONE26TH, 1.628/ONE26TH, 0.114/ONE26TH, 0.387/ONE26TH, 0.308/ONE26TH,
  0.136/ONE26TH
};

static LangLetterFreq_t PortugueseLetters = {
  14.63/ONE26TH, 1.040/ONE26TH, 3.880/ONE26TH, 4.990/ONE26TH, 12.57/ONE26TH,
  1.020/ONE26TH, 1.030/ONE26TH, 1.280/ONE26TH, 6.180/ONE26TH, 0.400/ONE26TH,
  0.020/ONE26TH, 2.780/ONE26TH, 4.740/ONE26TH, 5.050/ONE26TH, 10.73/ONE26TH,
  2.520/ONE26TH, 1.200/ONE26TH, 6.530/ONE26TH, 7.810/ONE26TH, 4.740/ONE26TH, 
  4.630/ONE26TH, 1.670/ONE26TH, 0.010/ONE26TH, 0.210/ONE26TH, 0.010/ONE26TH,
  0.470/ONE26TH
};

static LangLetterFreq_t SwedishLetters = {
  9.300/ONE26TH, 1.300/ONE26TH, 1.300/ONE26TH, 4.500/ONE26TH, 9.900/ONE26TH,
  2.000/ONE26TH, 3.300/ONE26TH, 2.100/ONE26TH, 5.100/ONE26TH, 0.700/ONE26TH,
  3.200/ONE26TH, 5.200/ONE26TH, 3.500/ONE26TH, 8.800/ONE26TH, 4.100/ONE26TH,
  1.700/ONE26TH, 0.007/ONE26TH, 8.300/ONE26TH, 6.300/ONE26TH, 8.700/ONE26TH,
  1.800/ONE26TH, 2.400/ONE26TH, 0.030/ONE26TH, 0.100/ONE26TH, 0.600/ONE26TH,
  0.020/ONE26TH
};

static LangLetterFreq_t ItalianLetters = {
  11.74/ONE26TH, 0.920/ONE26TH, 4.500/ONE26TH, 3.730/ONE26TH, 11.79/ONE26TH,
  0.950/ONE26TH, 1.640/ONE26TH, 1.540/ONE26TH, 11.28/ONE26TH, 0,
  0,             6.510/ONE26TH, 2.510/ONE26TH, 6.880/ONE26TH, 9.830/ONE26TH,
  3.050/ONE26TH, 0.510/ONE26TH, 6.370/ONE26TH, 4.980/ONE26TH, 5.620/ONE26TH,
  3.010/ONE26TH, 2.100/ONE26TH, 0,             0,             0,
  0.490/ONE26TH 
};

static LangLetterFreqMap_t languages = {
  {polyGLOB::kENGLISH, EnglishLetters},
  {polyGLOB::kSPANISH, SpanishLetters},
  {polyGLOB::kFRENCH, FrenchLetters},
  {polyGLOB::kGERMAN, GermanLetters},
  {polyGLOB::kITALIAN, ItalianLetters},
  {polyGLOB::kPORTUGUESE, PortugueseLetters},
  {polyGLOB::kSWEDISH, SwedishLetters},
  {polyGLOB::kMYSTERY, MysteryLetters}
};

//---
// We will use a function to determine if two floating point numbers
// are indistinguishable from each other. 
//---
#pragma GCC diagnostic ignored "-Wfloat-equal"
inline bool moreOrLessEqual(double d1, double d2)
{
  if (d1 == HUGE_VAL || d2 == HUGE_VAL) return d1 == d2;
  return fabs(d1 - d2) <= DBL_EPSILON * std::max(d1,d2);
}
#pragma GCC diagnostic warning "-Wfloat-equal"

//---
// A C language interface.
//---
void identifyLanguage (const char * shred, char ** p)
{
  if (shred == 0 || p == 0 || *p == 0) return;
  polyGLOB ID;
  string t = ID(shred);
  strcpy(*p, t.c_str());
} 

//---
// For rigorous analysis, get the whole answer.
//---
polyGLOBobj polyGLOB::identify(const string & s, double d)
{
  (*this)(s,d);
  return languageObj;
}

//---
// Here is our functor function that returns nothing except the 
// ISO language code.
//---
string polyGLOB::operator() (const string & s, double dExp)
{
  if (dExp < 1.0) dExp = 2.0;
  if (dExp > 4.0) dExp = 2.0;

  // zap anything that we had from a previous analysis.
  languageObj.charDistances.clear();
  languageObj.languageDistances.clear();
  // make mystery the default language.
  string thisLanguage = kMYSTERY;
  if (s.size() < 100) return thisLanguage;

  // Need to make sure the map has all letters represented, even
  // if the counts of some of them are zero.
  CharFreq_t freq;
  for (char c = 'a'; c <= 'z'; freq[c++]=0);

  // The secret decoder ring!
  dgiASCIIdecoder_t decoder;

  stringstream ss;
  ss.str(s);
  size_t iTotal{0};


  // Put the minDist way on out there .... we will find something
  // closer than infinitely far away.
  double dist{0}, minDist{HUGE_VAL};

  //---
  // Translation: for each char in the stream, convert from UTF8 to
  // ASCII, convert to lower case and count it.
  //---
  do freq[::tolower(decoder.getNextChar(ss))]++; while (!ss.eof()); 

  // Find out how many "letters" there were in the text. The
  // answer is iTotal.
  auto pf = freq.lower_bound('a');
  do iTotal += pf->second; while (++pf != freq.upper_bound('z'));

  // It is a little more flexible to have all these things be
  // vector<double>, so we rip the values out of the map.
  auto v = vectorize(freq, iTotal); 

  LangLetterFreq_t::iterator theseFrequencies;


  // Check each language.
  auto p = languages.begin();
  do
  {
    dist = vectorDistance(v, p->second, dExp); 
    languageObj.languageDistances[p->first] = dist;

    // is this better than our previous best fit?
    if (dist < minDist) 
    {
      languageObj.distance = dist;
      minDist = dist;
      languageObj.confidence = fabs(languageObj.distance - minDist);
      thisLanguage = p->first;
      theseFrequencies = p->second.begin();
    }
  }
  while (++p != languages.end());

  // and the answer is, "thisLanguage"
  languageObj.language = thisLanguage;

  // preserve the nature of the fit for later use.
  auto q = v.begin();
  do 
    languageObj.charDistances.push_back(*q - *theseFrequencies); 
  while (++theseFrequencies, ++q != v.end());

  //---
  // Confidence ... There are two factors:
  //   [1] How much text did we have to work with?
  //   [2] How much better was our best fit than the second best?
  //---

  // Was the sample bigger than 500 significant letters? (Note that
  // we are using integer division with a floating point result.)
  double shred_size_factor = (iTotal / 500 ? 1.0 : iTotal / 500.0);

  // Find the second best fit.
  vector<double> vdist;
  auto pld = languageObj.languageDistances.begin();
  do vdist.push_back(pld->second); while (++pld != languageObj.languageDistances.end());
  sort(vdist.begin(), vdist.end()); 

  // Is it more than 25% farther away? 
  double second_choice_factor = (vdist[1]/vdist[0] > 1.25 ? 
                                 1.0 : // Yes. Well that's good enough.
                                 (vdist[1]/vdist[0] - 1.0) / 0.25 ); // No.

  // shred_size_factor is on the half open interval [0 .. 1). Taking its
  // squareroot tends to push the number closer to one, just like all that
  // grading on a curve you suffered through in university.
  //
  // second_choice_factor is also a number on the same half open interval,
  // but we will treat it linearly.
  languageObj.confidence = sqrt(shred_size_factor) * second_choice_factor;

  return thisLanguage;  
}

//---
// vectorDistance() calculates the R(n) Cartesian distance between
// the vectors using any exponent you like. Standard math
// would have us using "2", but as Bill James has taught us,
// there are worlds in which the distance metric is not 2.
//---
static double vectorDistance(const LangLetterFreq_t & v1, 
                             const LangLetterFreq_t & v2, 
                             double dExp)
{
  // This check is just in case you want to use this function elsewhere.
  if (v1.size() != v2.size()) return HUGE_VAL;
  // as is this little correction.
  dExp = fabs(dExp);

  // special cases ...
  auto square = moreOrLessEqual(dExp, 2.0); 
  auto unity = moreOrLessEqual(dExp, 1.0);

  // let's do it.
  double dist{0.0};
  auto p = v1.begin();
  auto q = v2.begin();
  do
    if (square)
      dist += ((*p - *q) * (*p - *q));
    else if (unity)
      dist += (p > q ? *p - *q : *q - *p);
    else
      dist += pow((fabs(*p - *q)), dExp);
  while (++q, ++p != v1.end());
  return square ? // the usual, if not default case
           sqrt(dist) : 
           (unity ? 
             dist : // no need to do anything at all.
             pow(dist, 1.0/dExp));
}

LangLetterFreq_t vectorize(CharFreq_t & m, double d)
{
  LangLetterFreq_t v;

  // We are only interested in the letters.
  auto p = m.lower_bound('a');
  auto q = m.upper_bound('z');
  do
    v.push_back((p->second/d)*(100.0/ONE26TH));
  while (++p != q);
  return v;
}

