#include <array>
#include <deque>
#include <vector>
#include <unordered_map>
#include <random>

#include <locale>
#include <codecvt>

#include "utils.hpp"
#include "pretty_print.hpp"


// COMPILE-TIME CONSTANTS
////////////////////////////////////////////////////////////////////////////////

// number of previous words to remember for Markovian sentence generation
constexpr const int SIZE = 2;

// minimum characters necessary for a sentence;
constexpr const int MIN_THRESHOLD = 10;

// maximum number of attempts to place a word or sentence -- O(k^2)
constexpr const int TRY_LIMIT = 5;

// maximum number of tries to place word
constexpr const int MAX_CHAR_LIMIT = 145;


// TYPE DEFINITIONS
////////////////////////////////////////////////////////////////////////////////

using WordKey = std::array <io::string, SIZE>;
using WordList = std::vector <io::string>;


// STRING CONSTANTS
////////////////////////////////////////////////////////////////////////////////

const io::char_t* BEGIN_STRING = L" [B] ";
const io::char_t* END_STRING = L" [E] ";
const io::char_t* SENTENCE_PUNCT = L".!?";
const io::char_t* SPACE = L" ";


// MARKOVIAN SENTENCE GENERATION CLASS
////////////////////////////////////////////////////////////////////////////////

class MarkovCorpus {

  // data structure to store corpus
  std::unordered_map <WordKey,
                      WordList,
                      util::key_hash <WordKey>,
                      util::key_equal <WordKey>> self;

  // internal random number generator given size
  int rng (int size) const {
    static std::mt19937 gen { std::random_device { } () };
    return std::uniform_int_distribution<> { 0, size - 1 } (gen);
  };


public:

  // always default to requiring a string for construction
  explicit MarkovCorpus (const io::string & str) {
    addFromString (str);
  }

  // adds a single sentence to the corpus for 'learning'
  void addSentence (const io::string & sentence) {
    // create istringstream from string
    io::iss iss { sentence };
    // read word-by-word into deque
    std::deque <io::string> parsed { io::isi { iss }, { } };
    // insert false start point
    std::fill_n (std::front_inserter (parsed), SIZE, BEGIN_STRING);
    // push each sliding window of SIZE into corpus
    for (uint i { 0 }; i < parsed.size() - SIZE; ++i)
      // account for dynamic creation with operator[]
      self [util::createFromContainer <WordKey> (parsed, i)].push_back (parsed [i + SIZE]);
  }

  // adds a string (may contain more than one sentence)
  void addFromString (const io::string & str) {
    // grab one sentence at a time
    for (size_t prev { 0 }, index { str.find_first_of (SENTENCE_PUNCT) };
         index != std::string::npos;
         prev = index + 1, index = str.find_first_of (SENTENCE_PUNCT, prev))
      // only care about sufficiently large sentences
      if (index - prev > MIN_THRESHOLD)
        addSentence ( { std::begin (str) + prev, std::begin (str) + index + 1 } );
  }

  // outputs a generated sentence
  io::string yieldSentence (uint max_chars = MAX_CHAR_LIMIT) const {
    std::vector <io::string> out { SIZE, BEGIN_STRING };
    uint offset { 0 }, size { 0 }, tries { 0 };
    // add words until size reached
    while (true) {
      const auto lookup = self.find (
        util::createFromContainer <WordKey> (out, offset));
      // no words available --> break
      if (lookup == self.end())
        break;
      // pull out word
      const auto & list = lookup->second;
      const auto word = list.at (rng (list.size()));
      // if number of tries exceeded or end string -> leave creation
      if (word == END_STRING)
        break;
      // if word is too big, try again
      if (word.size() + size >= max_chars && ++tries <= TRY_LIMIT)
        continue;
      // add word
      tries = 0;               //  - reset tries
      size += word.size() + 1; //  - augment size
      out.push_back (word);    //  - add to list
      ++offset;                //  - increase offset
    }
    return util::print (std::begin (out) + SIZE, std::end (out), SPACE);
  }

  // outputs one or more sentences (limit based on passed parameter)
  io::string yieldOutput (uint max_chars = MAX_CHAR_LIMIT) const {
    std::vector <io::string> out;
    uint size { 0 }, tries { 0 };
    // while we fit within the space
    while (size <= max_chars) {
      // grab sentence
      auto sentence = yieldSentence (max_chars);
      // try again if too big and try limit not reached
      if (sentence.size() + size >= max_chars) {
        if (++tries < TRY_LIMIT)
          continue;
        else
          break;
      }
      // add sentence to output
      tries = 0;                   // - reset tries
      out.push_back (sentence);    // - add sentence
      size += sentence.size() + 1; // - increment size
    }
    return util::print (std::begin (out), std::end (out), SPACE);
  }

  // prints the current corpus
  void print() const {
    for (const auto & i : self) {
      std::cout << i << std::endl;
    }
  }

};


// MAIN
////////////////////////////////////////////////////////////////////////////////

int main (int argc, char* argv[]) {
  if (argc < 2)
    return EXIT_FAILURE;

  // set global locale to UTF-8
  std::locale::global ({ std::locale { "" }, new std::codecvt_utf8 <io::char_t> });
  std::wcout.imbue (std::locale { });
  std::wcerr.imbue (std::locale { });
  std::wcin.imbue (std::locale { });

  // read contents of file and yield an output
  std::wcout << MarkovCorpus { (io::oss {} << io::ifs { argv [1] }.rdbuf()).str() }.yieldOutput() << std::endl;
  return EXIT_SUCCESS;
}
