////////////////////////////////////////////////////////////////////////////////
///
/// @file namiML.cpp
/// @brief NAtive Mechanic Intelligent Machine Learning Framework Source File
/// @details namiML framework supports multiple machine learning algorithms. @n
///          Some algorithms require different forms of inputs or training @n
///          data. Be careful not to adapt wrong type of data before executing @n
///          the program. The list of machine learning algorithms is in a header @n
///          file.
///
/// @author Suwon Oh <suwon@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2014/12/18 Suwon Oh created @n
/// 2014/12/18 Suwon Oh implemented prototype @n
/// 2014/12/18 Suwon Oh adapted to Doxygen @n
/// 2014/12/18 Suwon Oh added to concept learning framework @n
/// 2014/12/19 Suwon Oh finished first version @n
/// 2015/01/17 Suwon Oh option update @n
/// 2015/01/17 Suwon Oh added generating input & train functions for CE@n
/// 2015/01/23 Suwon Oh added decision tree learning @n
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///

#include <iomanip>      ///< for managing console output
#include <cstdlib>      ///< for exit() & rand() & srand()
#include <cstring>      ///< string compare
#include <ctime>        ///< for time()
#include "namiML.h"

#define VERSION "v0.5"

using namespace std;  ///< already declared by header file

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief print terminate log
void namiTerm(void) {
  cout <<    "///"
  << endl << "///" << "                               Good Bye ~"
  << endl << "///"
  << endl << "////////////////////////////////////////////////////////////////////////////////"
  << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief print help message
void printHelp(void) {
  cout <<    "///" << "  Usage: namiML <-g / -p> -m [MODE] -i [INPUT] -t [T-DATA] (-o [OUTPUT]) (-x [ANSWER])"
  << endl << "///"
  << endl << "///" << "  < Example > :"
  << endl << "///" << "              namiML -g -m concept"
  << endl << "///" << "              namiML -g -m decision -i input.txt -t train.txt -o answer.txt"
  << endl << "///" << "              namiML -p -m concept -i input.txt -t train.txt -o output.txt -x answer.txt"
  << endl << "///" << "              namiML -p -m decision -i data/IN.txt -t data/TRAIN.txt"
  << endl << "///"
  << endl << "///" << "  OPTION LIST ; Each option must be only one or not"
  << endl << "///"
  << endl << "///" << "    -g       generate a training data file and a new instance file (output means answer)"
  << endl << "///" << "    -p       predict output about given input data and training data"
  << endl << "///" << "    -h       print this help message (no other options)"
  << endl << "///" << "    -m       choose machine learning mode"
  << endl << "///" << "    -i       choose a new instance file (-g option default : ./input.txt)"
  << endl << "///" << "    -t       choose a training data file (-g option default : ./train.txt)"
  << endl << "///" << "    -o       determine an output file name (-p option default : ./output.txt, -g : ./answer.txt)"
  << endl << "///" << "    -x       perfomance test for generated input & train set (only with predict)"
  << endl << "///"
  << endl << "///" << "  [MODE]     mode type"
  << endl << "///" << "  [INPUT]    input file name"
  << endl << "///" << "  [T-DATA]   training data file name"
  << endl << "///" << "  [OUTPUT]   output file name (-g default : ./output.txt, -p default : ./answer.txt)"
  << endl << "///" << "  [ANSWER]   answer file name for generated examples"
  << endl << "///"
  << endl << "///" << "  MODE LIST"
  << endl << "///"
  << endl << "///" << "    " << setw(10) << left << "concept" << "  concept learning algorithm"
  << endl << "///" << "    " << setw(10) << left << "decision" << "  decision tree algorithm"
  << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief print error message
///
/// @param message error message
void printError(const char* message) {
  cout <<    "///" << "  ERROR : " << message
  << endl << "///"
  << endl << "///" << "  please see namiML help message like below ..."
  << endl << "///" << "      namiML -h"
  << endl;
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief global valid option information
///
static char gl_valid_option_set[] = { 'g', 'p', 'm', 'i', 't', 'o', 'h', 'x' };
static int gl_num_valid_option = 8;

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief find given option from arguments vector
/// 
/// @param option option type character
/// @param argc the number of total given arguments
/// @param argv arguments vector
/// @retval index option index from arguments vector
/// @retval 0 error return, not index 0 (pointing executing program itself)
int findOption(char option, int argc, char* argv[]) {
  int find = 0; // will be replaced with correct one

  if (argc <= 1)
    return 0;  // means error return

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == option) {
        if (find == 0) { // first find
          find = i;
        } else { // duplicated
          return 0; // means error return
        }
      }
    }
  }

  return find; // if 0, there is no match
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief check if this option is valid and handle printError too
/// 
/// @param argc the number of total given arguments
/// @param argv arguments vector
/// @retval true if all option set is valid
/// @retval false if option set has error
bool isOptionValid(int argc, char* argv[]) {
  if (argc <= 1) { // means no options
    printError("NO OPTIONS");
    return false;
  }
  
  // Option syntax check
  for (int i = 1; i < argc; i++) {
    bool typeGood = false;
    if (argv[i][0] == '-') {
      for (int j = 0; j < gl_num_valid_option; j++) {
        if (argv[i][1] == gl_valid_option_set[j]) {
          typeGood = true;
          break;
        }
      }

      if (!typeGood) {
        printError("INVALID OPTION TYPE");
        return false;
      }

      // single option handling ('g', 'p', 'h')
      if (argv[i][1] == 'h' && argc > 2) {
        printError("HELP OPTION COME ALONE");
        return false;
      }
      if (argv[i][1] == 'g' || argv[i][1] == 'p')
        continue;
      
      // couple option handling ('m', 'i', 't', 'o')
      if (argv[i][1] == 'm' || argv[i][1] == 'i' || \
          argv[i][1] == 't' || argv[i][1] == 'o' || \
          argv[i][1] == 'x') {
        // check if there is second option
        if (i >= (argc - 1)) { // check if this option is the last one
          printError("THERE IS NO SECOND OPTION");
          return false;
        }
        i++; // skip the second option
        continue;
      }
    }
    else { // invalid option
      printError("INVALID OPTION");
      return false;
    }
  }
  
  // Option semantic check
  if (findOption('h', argc, argv)) // help logical check
    return true;

  if (findOption('p', argc, argv)) { // prediction logical check
    if (findOption('g', argc, argv)) {
      printError("GENERATION AND PREDICTION ARE NOT PROCESSING SIMULTANEOUSLY");
      return false;
    }

    if (findOption('m', argc, argv) && findOption('i', argc, argv) && \
        findOption('t', argc, argv))
      return true;

    printError("PREDICTION OPTION ERROR");
    return false;
  }
  
  if (findOption('g', argc, argv)) { // generation logical check
    if (findOption('x', argc, argv)) {
      printError("GENERATION CAN't EXECUTE A PERFORMANCE TEST (IT'S ONLY FOR PREDICTION)");
      return false;
    }
    
    if (findOption('m', argc, argv))
      return true;

    printError("GENERATION OPTION ERROR");
    return false;
  }

  printError("INVALID OPTIONS");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief ignore blank character from input stream
/// 
void ignoreBlank(istream *i)
{
  while(i->peek() == '\n' || i->peek() == '\t' || i->peek() == ' ')
    i->get();
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief check if next istream token is white space
/// 
bool isWhite(istream *i)
{
  return (i->peek() == '\n' || i->peek() == '\t' || i->peek() == ' ');
}

////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief get integer number from istream
/// @details next token must be white token or given token
///
int istream_to_int(istream *i)
{
  int ret = 0;
  while (!isWhite(i)) {
    ret *= 10;
    int tmp = i->get();
    if (tmp < 48 || tmp > 57)
      exit(-1);
    ret += tmp - 48; // 48 - ASCII code '0' value
  }
  return ret;
}

int istream_to_int_by_token(istream *i, char token)
{
  int ret = 0;
  while (i->peek() != token) {
    ret *= 10;
    int tmp = i->get();
    if (tmp < 48 || tmp > 57)
      exit(-1);
    ret += tmp - 48; // 48 - ASCII code '0' value
  }
  return ret;
}

int main(int argc, char* argv[])  ///< taking options
{
  cout <<    "////////////////////////////////////////////////////////////////////////////////"
  << endl << "///"
  << endl << "///" << "           NAtive Mechanic Intelligent Machine Learning Framework"
  << endl << "///" << "                               NamiML " << VERSION
  << endl << "///" << endl;
    
  if (!isOptionValid(argc, argv)) {
    namiTerm();
    exit(1);
  }
  // there is no option error at all

  // first, find help option if any
  //    if is, print help message and terminate
  //    otherwise, do next procedure
  if (findOption('h', argc, argv)) { // if any help option
    printHelp();
    namiTerm();
    return 0;
  }

  if (findOption('p', argc, argv)) { // do prediction
    ofstream*     output;         ///< output file stream
    ifstream*     input;          ///< new instance file stream
    ifstream*     training;       ///< training data file stream
    ifstream*     answer;         ///< answer data file stream
    ML_Machine*   machine;        ///< machine learning engine
    bool          isGen = false;  ///< true if input is gen by generator

    // input file stream open
    input = new ifstream(argv[findOption('i', argc, argv) + 1]);
    if (!input->is_open()) {
      printError("INPUT FILE OPEN ERROR");
      namiTerm();
      exit(1);
    }

    // training file stream open
    training = new ifstream(argv[findOption('t', argc, argv) + 1]);
    if (!training->is_open()) {
      input->close();
      printError("TRAINING FILE OPEN ERROR");
      namiTerm();
      exit(1);
    }

    // output file stream open
    if (int oIndex = findOption('o', argc, argv)) //optional
      output = new ofstream(argv[oIndex+1]);
    else
      output = new ofstream("output.txt");
    if (!output->is_open()) { 
      input->close();
      training->close();
      printError("OUTPUT FILE OPEN ERROR");
      namiTerm();
      exit(1);
    }
    
    // answer file stream open
    if (findOption('x', argc, argv))
      answer = new ifstream(argv[findOption('x', argc, argv) + 1]);
    else
      answer = NULL;

    // select machine algorithm
    const char* mode = argv[findOption('m', argc, argv) + 1];
    if (strcmp(mode, "concept") == 0) {
      machine = new CE_Machine(input, training, answer, output);
    } else if (strcmp(mode, "decision") == 0) {
      machine = new ID3_Machine(input, training, answer, output);
    } else {
      printError("NO SUCH MODE");
      namiTerm();
      exit(1);
    }

    // do training
    if (!machine->train()) {
      printError("TRAIN ERROR");
      namiTerm();
      exit(1);
    }
    
    // do predict
    if (!machine->predict()) {
      printError("PREDICT ERROR");
      namiTerm();
      exit(1);
    }

    // close all streams
    input->close();
    training->close();
    answer->close();
    output->close();
  } else { // do generating input and training data
    ofstream*      input;     ///< new instance file stream
    ofstream*      training;  ///< training data file stream
    ofstream*      answer;    ///< answer data file stream
    ML_Machine*    machine;   ///< machine learning engine
    
    // input file stream open
    if (int iIndex = findOption('i', argc, argv)) //optional
      input = new ofstream(argv[iIndex+1]);
    else
      input = new ofstream("input.txt");
    if (!input->is_open()) {
      printError("INPUT FILE OPEN ERROR");
      namiTerm();
      exit(1);
    }

    // training file stream open
    if (int tIndex = findOption('t', argc, argv)) //optional
      training = new ofstream(argv[tIndex+1]);
    else
      training = new ofstream("train.txt");
    if (!training->is_open()) {
      input->close();
      printError("TRAINING FILE OPEN ERROR");
      namiTerm();
      exit(1);
    }
    
    // answer file stream open
    if (int aIndex = findOption('o', argc, argv)) //optional
      answer = new ofstream(argv[aIndex+1]);
    else
      answer = new ofstream("answer.txt");
    if (!answer->is_open()) {
      input->close();
      training->close();
      printError("ANSWER FILE OPEN ERROR");
      namiTerm();
      exit(1);
    }

    // select machine algorithm
    const char* mode = argv[findOption('m', argc, argv) + 1];
    if (strcmp(mode, "concept") == 0) {
      machine = new CE_Machine(input, training, answer, NULL);
    } else if (strcmp(mode, "decision") == 0) {
      machine = new ID3_Machine(input, training, answer, NULL);
    } else {
      printError("NO SUCH MODE");
      namiTerm();
      exit(1);
    }

    // do generating
    if (!machine->generate()) {
      printError("GENERATION ERROR");
      namiTerm();
      exit(1);
    }

    // close all streams
    input->close();
    training->close();
    answer->close();
  }
  
  namiTerm();
  return 0;  
}

////////////////////////////////////////////////////////////////////////////////
///
///  ML_Machine
///

ML_Machine::ML_Machine(ios *input, ios *training, ios *answer, ostream *output)
  : input(input), training(training), answer(answer), output(output) {}

ML_Machine::~ML_Machine() {}

////////////////////////////////////////////////////////////////////////////////
///
///  CE_Machine
///

CE_Machine::CE_Machine(ios *input, ios *training, ios *answer, ostream *output)
  : ML_Machine(input, training, answer, output) {
  ce = NULL;
}

CE_Machine::~CE_Machine() {
  if (ce)
    delete(ce);
}

bool CE_Machine::train()
{
  cout <<    "///" << "  Candidate Elimination Algorithm is training..."
  << endl;

  int attrNum = 0; // assume that attribute number don't exceed int range
  // get the number of attributes
  // self-implemented char - to - int mechanism

  // training should reference istream class
  if (istream *train = dynamic_cast <istream*> (training)) {
    while (train->peek() != '\n') {
      attrNum *= 10;
      int testVal = train->get();
      if (testVal < 48 || testVal > 57)
        return false;
      attrNum += testVal - 48; // 48 - ASCII code '0' value
    }
    ignoreBlank(train);

    // update attribute number
    size = attrNum;

    // create ce engine
    ce = new CE(size);  

    do {
      bool* td = new bool[size + 1];  // including target attribute
      for (int i = 0; i < attrNum + 1; i++) {
        if (train->eof() || train->bad()) // middle check
          return false;

        char c = train->get();
        if (c != 't' && c != 'f') // only 't' or 'f' can read
          return false;

        td[i] = (c == 't') ? true : false;
        ignoreBlank(train);
      }
      if (!ce->updateVS(td)) // update version space
        return false;
    } while (!train->eof() && !train->bad());

    cout <<    "///" << "                               -- finish"
      << endl << "///" << endl;

    return true;
  } else { // reference error
    return false;
  }

  return false; // code error
}

bool CE_Machine::predict(void)
{
  /* these three counters are for perfomance test with generated inputs */
  int total = 0;      ///< the number of total inputs
  int dontknow = 0;   ///< the number of inputs predicted dontknow(?)
  int wrong = 0;      ///< the number of inputs predicted wrong

  cout <<    "///" << "  Candidate Elimination Algorithm is predicting..."
  << endl;
  
  if (istream *inp = dynamic_cast <istream*> (input)) {
    ignoreBlank(inp); // prevent blank error
    
    istream *ans = NULL;
    Hypothesis *ans_h = NULL;
    if (answer) { // do performance test
      if (ans = dynamic_cast <istream*> (answer)) {
        AttrVal *tmp_a = new AttrVal[size];
        for (int i = 0; i < size; i++) {
          char c = ans->get();
          switch(c) {
            case 't':
              tmp_a[i] = vTrue; break;
            case 'f':
              tmp_a[i] = vFalse; break;
            case '?':
              tmp_a[i] = vAllAccept; break;
            default:
              return false; break; // answer file error
          }
          ignoreBlank(ans);
        }
        ans_h = new Hypothesis(size, tmp_a);
      } else {
        return false; // istream error
      }
    }

    do {
      bool* in = new bool[size];
      for (int i = 0; i < size; i++) {
        if (inp->eof() || inp->bad()) // middle check
          return false;

        char c = inp->get();
        if (c != 't' && c != 'f') // only 't' or 'f' can read
          return false;

        in[i] = (c == 't') ? true : false;
        ignoreBlank(inp);

        // update output too
        (*output) << c << " ";
      }
      char out;
      switch(ce->predict(in)) {
        case r_true: 
          out = 't';
          if (answer)
            if (!ans_h->isCover(in)) // should be covered
              wrong++;
          break;
        case r_false: 
          out = 'f'; 
          if (answer)
            if (ans_h->isCover(in)) // should be uncovered
              wrong++;
          break;
        case r_dontknow: 
          out = '?';
          if (answer)
            dontknow++; // increase dontknow counter
          break;
        default: 
          out = 'x'; 
          break; // error
      }
      (*output) << "=> " << out << endl;
      
      if (answer)
        total++; // increase total counter
    } while (!inp->eof() && !inp->bad());

    cout <<    "///" << "                               -- finish"
    << endl << "///" << endl;

    if (answer) {
      cout <<    "///" << "  Perfomance Test Result :"
      << endl << "///" << "    - total :        " << total
      << endl << "///" << "    - dontknow :     " << dontknow
      << endl << "///" << "    - wrong :        " << wrong
      << endl << "///" << "    - Success Rate : "
      << (total - dontknow - wrong) * 100 / total << " %"
      << endl << "///" << endl;
 
    }

    return true;
  } else { // reference error
    return false;
  }

  return false; // code error
}

#define MAX_CE_ATTR    20      ///< the maximum number of possible attributes
#define MIN_CE_ATTR    5       ///< the minimum number of possible attributes
#define MAX_CE_INPUT   100     ///< the maximum number of input data
#define MIN_CE_INPUT   5       ///< the minimum number of input data

////////////////////////////////////////////////////////////////////////////////
///  @brief calculate n power of two
///
static inline int powerOfTwo(int n) {
  int ret = 1;

  for (int i = 0; i < n; i++)
    ret *= 2;

  return ret;
}

Result* CE_Machine::createTarget() {
  Result *ret = new Result[size];

  while (true) {
    bool allDontknow = true;
    for (int i = 0; i < size; i++) {
      int tmp = rand() % 3;
      ret[i] = (tmp == 2) ? r_dontknow : (tmp == 1) ? r_true : r_false;
      if (ret[i] != r_dontknow)
        allDontknow = false;
    }

    if (!allDontknow)
      break;
  }

  return ret;
}

bool CE_Machine::makeOneEntry(Result target, bool t) {
  bool ret;

  switch(target) {
    case r_false:
      if (t)
        ret = false;
      else
        ret = true;
      break;
    case r_true:
      if (t)
        ret = true;
      else
        ret = false;
      break;
    default: // r_dontknow
      ret = (rand() % 2 == 0) ? false : true;
      break;
  }

  return ret;
}

bool* CE_Machine::createInstance(Result* target, bool t) {
  bool *ret = new bool[size + 1]; // the last entry is target attribute

  bool *entryBool = new bool[size];
  if (!t) {
    while(true) {
      bool falseExist = false;
      for (int i = 0; i < size; i++) {
        entryBool[i] = (rand() % 2 == 0) ? false : true;
        if (target[i] != 2 && entryBool[i] == false)
          falseExist = true;
      }

      if (falseExist)
        break;
    }
  }
  
  for (int i = 0; i < size; i++) {
    if (t)
      ret[i] = makeOneEntry(target[i], true);
    else
      ret[i] = makeOneEntry(target[i], entryBool[i]);
  }
  ret[size] = t;
  
  return ret;
}


bool CE_Machine::generate()
{
  cout <<    "///" << "  Candidate Elimination Algorithm is generating \
input & training data..." << endl;
  cout <<    "///" << endl;
  cout <<    "///" << "  [Brief]" << endl;
  cout <<    "///" << "  - Training Data File Format:" << endl;
  cout <<    "///" << "   top of line         : the number of attributes \
except for target attr" << endl;
  cout <<    "///" << "   the following lines : sets of boolean value \
represented training data" << endl;
  cout <<    "///" << endl;
  cout <<    "///" << "  - Input Data File Format:" << endl;
  cout <<    "///" << "   entire lines : sets of boolean value represented \
unknown input data" << endl;
  cout <<    "///" << endl;
  
  srand(time(0)); // time seeding to random
  ostream *inp = dynamic_cast <ostream*> (input);
  ostream *train = dynamic_cast <ostream*> (training);
  ostream *ans = dynamic_cast <ostream*> (answer);
  if (!inp || !train || !ans)
    return false;
    
  // step 1. define target information.
  
  // the number of attributes ( MIN_CE_ATTR ~ MAX_CE_ATTR )
  int numAttr = MIN_CE_ATTR + (rand() % (MAX_CE_ATTR - MIN_CE_ATTR + 1));
  // the number of training data ( 2 ^ (0.5 * numAttr) ~ 2 ^ (0.75 * numAttr) )
  int numTrain = powerOfTwo((numAttr / 2) + (rand() % (numAttr / 4)));
  // the number of input data ( MIN_CE_INPUT ~ MAX_CE_INPUT )
  int numInput = MIN_CE_INPUT + (rand() % (MAX_CE_INPUT - MIN_CE_INPUT + 1));
  size = numAttr;

  // step 2. create a target concept.
  cout <<    "///" << "  [Details]" << endl;
  cout <<    "///" << "  - Target Concept:" << endl;
  cout <<    "///" << "   ";
  Result *target = createTarget();
  for (int i = 0; i < numAttr; i++) {
    char cur = (target[i] == r_false) ? 'f' : (target[i] == r_true) ? 't' : '?';
    cout << cur;
    (*ans) << cur;
    if (i < numAttr -1) {
      cout << " ";
      (*ans) << " ";
    }
  }
  cout << endl << "///" << endl;
  cout <<    "///" << "  - Answer Data:" << endl;
  cout <<    "///" << "   " << 1 << " answer data created" << endl;
  cout <<    "///" << endl;

  // step 3. make training data from target concept.
  cout <<    "///" << "  - Training Data:" << endl;
  cout <<    "///" << "   " << numTrain << " training data created" << endl;
  cout <<    "///" << endl;
  (*train) << numAttr << endl; // write the number of attributes
  for (int i = 0; i < numTrain; i++) {
    bool isTrue = (rand() % 2 == 0) ? false : true;
    bool *tmp_train = createInstance(target, isTrue);
    
    for (int j = 0; j < numAttr + 1; j++) {
      char cur = (tmp_train[j] == true) ? 't' : 'f';
      (*train) << cur;
      if (j < numAttr)
        (*train) << " ";
    }
    
    (*train) << endl;
  }

  // step 4. make input data from target concept.
  cout <<    "///" << "  - Input Data:" << endl;
  cout <<    "///" << "   " << numInput << " input data created" << endl;
  cout <<    "///" << endl;
  for (int i = 0; i < numInput; i++) {
    bool isTrue = (rand() % 2 == 0) ? false : true;
    bool *tmp_input = createInstance(target, isTrue);

    for (int j = 0; j < numAttr; j++) {
      char cur = (tmp_input[j] == true) ? 't' : 'f';
      (*inp) << cur;
      if (j < numAttr - 1)
        (*inp) << " ";
    }

    (*inp) << endl;
  }
  
  cout <<    "///" << "                               -- finish"
  << endl << "///" << endl;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
///
///  ID3_Machine
///

ID3_Machine::ID3_Machine(ios *input, ios *training, ios *answer, ostream *output)
  : ML_Machine(input, training, answer, output) {
  attSizes = NULL;
  id3 = NULL;
}

ID3_Machine::~ID3_Machine() {
  if (id3)
    delete(id3);
}

////////////////////////////////////////////////////////////////////////////////
///
///  @brief ID3 train procedure
///  @details train text file format
///           1st line :  the number of attr
///           2nd line :  attr size array
///           3rd line~:  train data ...
///

bool ID3_Machine::train()
{
  cout <<    "///" << "  ID3 Algorithm is training..."
  << endl;

  ATTVAL **train_dat = NULL;
  ATTVAL **valid_dat = NULL;
  int nr_train = 0;
  int nr_valid = 0;

  // training should reference istream class
  if (istream *train = dynamic_cast <istream*> (training)) {
    // get the number of attributes
    nr_att = istream_to_int(train);
    ignoreBlank(train);
    
    attSizes = new ATTVAL[nr_att];
    // get the size of each attributes
    for (int i = 0; i < nr_att; i++) {
      attSizes[i] = (ATTVAL) istream_to_int(train);
      ignoreBlank(train);
    }
    this->attSizes = attSizes;
    
    // get the size of real data in train
    int full_size_of_train = istream_to_int(train);
    ignoreBlank(train);
    
    // define the size of train & validation set
    nr_valid = full_size_of_train / 4;
    nr_train = full_size_of_train - nr_valid;
    train_dat = new ATTVAL*[nr_train];
    for (int i = 0; i < nr_train; i++)
      train_dat[i] = new ATTVAL[nr_att];
    valid_dat = new ATTVAL*[nr_valid];
    for (int i = 0; i < nr_valid; i++)
      valid_dat[i] = new ATTVAL[nr_att];
   
    // create ID3 engine
    id3 = new ID3(attSizes, nr_att, train_dat, valid_dat, nr_train, nr_valid);

    int cur_pos = 0;
    do {
      for (int i = 0; i < nr_att; i++) {
        ATTVAL tmp = (ATTVAL) istream_to_int(train);
        ignoreBlank(train);
        if (cur_pos < nr_valid) { // validation set
          valid_dat[cur_pos][i] = tmp;
        } else { // train set
          train_dat[cur_pos - nr_valid][i] = tmp;
        }
      }
      cur_pos++;
    } while (!train->eof() && !train->bad());
    
    cout <<    "///" << "    - making a decision tree" << endl;
    cout <<    "///" << endl;
    id3->makeTree(); // train decision tree
    cout << endl << "///" << endl;
    cout <<    "///" << "    - pruning the decision tree" << endl;
    id3->prune(); // prune decision tree

    cout <<    "///" << "                               -- finish"
    << endl << "///" << endl;

    return true;
  } else { // reference error
    return false;
  }

  return false; // code error
}

bool ID3_Machine::predict()
{
  /* these three counters are for perfomance test with generated inputs */
  int total = 0;      ///< the number of total inputs
  int wrong = 0;      ///< the number of inputs predicted wrong
  int tmp = 0;

  cout <<    "///" << "  ID3 Algorithm is predicting..."
    << endl;

  if (istream *inp = dynamic_cast <istream*> (input)) {
    ignoreBlank(inp); // prevent blank error
    
    DTree *ans_tree = NULL;
    if (answer) {
      ans_tree = answer_tree_gen();
      ans_tree->setAttSizeArray(attSizes);
      ans_tree->setNrAtt(nr_att);
    }
    
    do {
      ATTVAL* in = new ATTVAL[nr_att - 1]; // except for target attr
      for (int i = 0; i < nr_att - 1; i++) {
        in[i] = istream_to_int(inp);
        ignoreBlank(inp);
        // update output too
        (*output) << in[i] << " ";
      }
      ATTVAL out = id3->predict(in);
      (*output) << "=> " << out << endl;
      if (answer) {
        if (out != ans_tree->getLeaf(in)->getVal())
          wrong++;
        total++; // increase total counter
      }
      delete []in;
    } while (!inp->eof() && !inp->bad());

    cout <<    "///" << "                               -- finish"
    << endl << "///" << endl;

    if (answer) {
      cout <<    "///" << "  Perfomance Test Result :"
        << endl << "///" << "    - total :        " << total
        << endl << "///" << "    - wrong :        " << wrong
        << endl << "///" << "    - Success Rate : "
        << (total - wrong) * 100 / total << " %"
        << endl << "///" << endl;
      delete (ans_tree);
    }

    return true;
  } else { // reference error
    return false;
  }

  return false; // code error
}

void ID3_Machine::recursive_make_answer_tree(DTreeRoot *parent, bool *remainAtt, 
  ostream *ans, int indent, bool first, bool last, DTree *tree, int ch_index) {
  // count the number of remaining attrs
  int nr_remain_attr = 0;
  for (int i = 0; i < nr_att - 1; i++)
    if (remainAtt[i])
      nr_remain_attr++;

  bool isLeaf = (nr_remain_attr <= 1) ? true : (rand() % nr_remain_attr == 0) ?
    true : false;

  if (!isLeaf) { // DTreeRoot
    int attIndex;
    for (int i = 0; i < nr_att - 1; i++)
      if (remainAtt[i])
        attIndex = i;

    int randStep = rand() % (nr_att - 1);
    for (int i = 0; i < randStep; i++) {
      int tmpIndex = attIndex + 1;

      while (!remainAtt[tmpIndex]) {
        tmpIndex++;
        if (tmpIndex >= nr_att - 1)
          tmpIndex = 0;
      } 

      attIndex = tmpIndex;
    }
    
    // handle standard out
    if (indent != 0) {
      if (first) {
        cout << " - ";
        cout << '(' << attIndex << ')';
      } else {
        cout << endl << "///\t   ";
        for (int i = 0; i < indent - 1; i++)
          cout << "      ";
        if (last)
          cout << " \\ ";
        else
          cout << " + ";
        cout << '(' << attIndex << ')';
      }
    } else {
      cout << "///\t" << '(' << attIndex << ')';
    }
    
    // handle answer file
    for (int i = 0; i < indent; i++)
      (*ans) << '\t';
    (*ans) << '(' << attIndex << ')' << endl;
    
    DTreeNode **child_array = new DTreeNode*[attSizes[attIndex]];
    for (int i = 0; i < attSizes[attIndex]; i++)
      child_array[i] = NULL;

    DTreeRoot *cur = new DTreeRoot(attIndex, child_array, attSizes[attIndex]);
    
    if (!parent) { // must be root
      if (tree) {
        tree->setRoot(cur);
      } else { // fatal error
        cout << "ERROR: root must have tree info" << endl;
        exit(1);
      }
    } else { // not root
      if (tree) { // must have no tree
        cout  << "ERROR: non-root must have no tree info" << endl;
        exit(1);
      } else {
        parent->setOneChild(cur, ch_index);
      }
    }

    bool *cpy_remain = new bool[nr_att - 1];
    for (int i = 0; i < nr_att - 1; i++)
      cpy_remain[i] = remainAtt[i];
    cpy_remain[attIndex] = false;

    for (int i = 0; i < attSizes[attIndex]; i++) {
      recursive_make_answer_tree(cur, cpy_remain, ans, indent + 1,
        (i == 0) ? true : false, (i == attSizes[attIndex] - 1) ? true : false, 
        NULL, i);
    }

    free(cpy_remain);
  }
  else { // DTreeLeaf
    int pred_val = rand() % attSizes[nr_att - 1];

    // handle standard out
    if (indent != 0) {
      if (first) {
        cout << " - ";
        cout << '{' << pred_val << '}';
      } else {
        cout << endl << "///\t   ";
        for (int i = 0; i < indent - 1; i++)
          cout << "      ";
        if (last)
          cout << " \\ ";
        else
          cout << " + ";
        cout << '{' << pred_val << '}';
      }
    } else {
      cout << "///\t" << '{' << pred_val << '}';
    }
        
    // handle answer file
    for (int i = 0; i < indent; i++)
      (*ans) << '\t';
    (*ans) << '{' << pred_val << '}' << endl;

    DTreeLeaf *cur = new DTreeLeaf(pred_val);

    if (!parent) { // must be root
      if (tree) {
        tree->setRoot(cur);
      } else { // fatal error
        cout << "ERROR: root must have tree info" << endl;
        exit(1);
      }
    } else { // not root
      if (tree) { // must have no tree
        cout  << "ERROR: non-root must have no tree info" << endl;
        exit(1);
      } else {
        parent->setOneChild(cur, ch_index);
      }
    }
  }
}

DTree* ID3_Machine::make_answer_tree(ostream *ans) {
  DTree* answer_tree = NULL;
  answer_tree = new DTree(); // make empty tree
  
  bool *remainAtt = new bool[nr_att - 1];
  for (int i = 0; i < nr_att - 1; i++)
    remainAtt[i] = true;

  recursive_make_answer_tree(NULL, remainAtt, ans, 0, false, false, answer_tree, 0);
  
  free (remainAtt);
  cout << endl << "///";

  return answer_tree;
}

ATTVAL* ID3_Machine::createInstance() {
  ATTVAL *ret = (ATTVAL*) new ATTVAL[nr_att];
  
  for (int i = 0; i < nr_att; i++)
    ret[i] = rand() % attSizes[i];

  return ret;
}

#define MAX_ID3_ATTR      5      ///< the maximum number of possible attributes
#define MIN_ID3_ATTR      2       ///< the minimum number of possible attributes
#define MAX_ID3_INPUT     100     ///< the maximum number of input data
#define MIN_ID3_INPUT     5       ///< the minimum number of input data
#define MAX_ID3_ATTR_SIZE 4      ///< the maximum number of attribute size
#define MIN_ID3_ATTR_SIZE 2       ///< the minimum number of attribute size
#define POWER_TRAIN       10      ///< the power coefficient of train data number
#define ID3_ERROR_FREQ    20      ///< how often error data is created

bool ID3_Machine::generate()
{
  cout <<    "///" << "  ID3 Algorithm is generating input & \
training data..." << endl;
  cout <<    "///" << endl;
  cout <<    "///" << "  [Brief]" << endl;
  cout <<    "///" << "  - Training Data File Format:" << endl;
  cout <<    "///" << "   top of line         : the number of attributes \
including target attr" << endl;
  cout <<    "///" << "   second line         : attribute size array" << endl;
  cout <<    "///" << "   the following lines : sets of integer value \
represented training data" << endl;
  cout <<    "///" << endl;
  cout <<    "///" << "  - Input Data File Format:" << endl;
  cout <<    "///" << "   entire lines : sets of integer value represented \
unknown input data" << endl;
  cout <<    "///" << endl;
  
  srand(time(0)); // time seeding to random
  ostream *inp = dynamic_cast <ostream*> (input);
  ostream *train = dynamic_cast <ostream*> (training);
  ostream *ans = dynamic_cast <ostream*> (answer);
  if (!inp || !train || !ans)
    return false;
    
  // step 1. define target information.
  
  // the number of attributes ( MIN_ID3_ATTR ~ MAX_ID3_ATTR )
  int numAttr = MIN_ID3_ATTR + (rand() % (MAX_ID3_ATTR - MIN_ID3_ATTR + 1));
  // the number of training data ( 2 ^ (numAttr) ~ 2 ^ (1.5 * numAttr) )
  int numTrain = POWER_TRAIN * powerOfTwo(numAttr + (rand() % (numAttr / 2)));
  // the number of input data ( MIN_INPUT ~ MAX_INPUT )
  int numInput = MIN_ID3_INPUT + (rand() % (MAX_ID3_INPUT - MIN_ID3_INPUT + 1));
  nr_att = numAttr;
  (*train) << numAttr << endl; // write the number of attributes

  attSizes = new ATTVAL[numAttr];
  for (int i = 0; i < numAttr; i++)
    attSizes[i] = MIN_ID3_ATTR_SIZE + (rand() % (MAX_ID3_ATTR_SIZE - MIN_ID3_ATTR_SIZE +1));
  
  // step 2. create a target concept.
  cout <<    "///" << "  [Details]" << endl;
  cout <<    "///" << "  - Attribute size:" << endl;
  cout <<    "///" << "    ";
  for (int i = 0; i < numAttr; i++) {
    cout << attSizes[i] << " ";
    (*train) << attSizes[i] << " ";
  }
  (*train) << endl;
  cout << endl << "///" << "  - Target Concept:" << endl;
 
  DTree* answer_tree = make_answer_tree(ans);
  answer_tree->setAttSizeArray(attSizes);
  answer_tree->setNrAtt(nr_att);

  cout << endl << "///" << endl;
  cout <<    "///" << "  - Answer Data:" << endl;
  cout <<    "///" << "   " << 1 << " answer data created" << endl;
  cout <<    "///" << endl;

  // step 3. make training data from target concept.
  cout <<    "///" << "  - Training Data:" << endl;
  cout <<    "///" << "   " << numTrain << " training data created" << endl;
  cout <<    "///" << endl;
  (*train) << numTrain << endl;
  for (int i = 0; i < numTrain; i++) {
    ATTVAL *tmp_train = createInstance();
    
    for (int j = 0; j < numAttr - 1; j++)
      (*train) << tmp_train[j] << " ";
    ATTVAL target = answer_tree->getLeaf(tmp_train)->getVal();
    target = (rand() % ID3_ERROR_FREQ == 0) ?
      (target + (rand() % (attSizes[numAttr - 1] - 1) + 1)) % attSizes[numAttr - 1] :      target;
    (*train) << target << endl;  
    delete (tmp_train);
  }

  // step 4. make input data from target concept.
  cout <<    "///" << "  - Input Data:" << endl;
  cout <<    "///" << "   " << numInput << " input data created" << endl;
  cout <<    "///" << endl;
  for (int i = 0; i < numInput; i++) {
    ATTVAL *tmp_input = createInstance();

    for (int j = 0; j < numAttr - 1; j++)
      (*inp) << tmp_input[j] << " ";
    (*inp) << endl;
    delete (tmp_input);
  }
  
  cout <<    "///" << "                               -- finish"
  << endl << "///" << endl;
  
  return true;
}

DTree* ID3_Machine::answer_tree_gen()
{
  DTree* answer_tree = NULL;
  if (istream *ans = dynamic_cast <istream*> (answer)) {
    answer_tree = new DTree(); // make empty tree
    ignoreBlank(ans);
    recursive_answer_tree_gen(NULL, ans, answer_tree, 0);
  }
  else {
    cout << "ERROR: answer istreamize fail" << endl;
    exit(1);
  }
  return answer_tree;
}

void ID3_Machine::recursive_answer_tree_gen(DTreeRoot *parent, istream *ans,
  DTree *tree, int ch_index) {
  if (ans->peek() == '(') { // DTreeRoot
    ans->get(); // consume '('
    int attIndex = istream_to_int_by_token(ans, ')');
    DTreeRoot *cur = new DTreeRoot(attIndex, new DTreeNode*[attSizes[attIndex]],
                        attSizes[attIndex]);
    ans->get(); // consume ')'
    ignoreBlank(ans);
    
    if (!parent) { // must be root
      if (tree) {
        tree->setRoot(cur);
      } else { // fatal error
        cout << "ERROR: root must have tree info" << endl;
        exit(1);
      }
    } else { // not root
      if (tree) { // must have no tree
        cout  << "ERROR: non-root must have no tree info" << endl;
        exit(1);
      } else {
        parent->setOneChild(cur, ch_index);
      }
    }

    for (int i = 0; i < attSizes[attIndex]; i++)
      recursive_answer_tree_gen(cur, ans, NULL, i);
  }
  else if (ans->peek() == '{') { // DTreeLeaf
    ans->get(); // consume '{'
    DTreeLeaf *cur = new DTreeLeaf(istream_to_int_by_token(ans, '}'));
    ans->get(); // consume '}'
    ignoreBlank(ans);

    if (!parent) { // must be root
      if (tree) {
        tree->setRoot(cur);
      } else { // fatal error
        cout << "ERROR: root must have tree info" << endl;
        exit(1);
      }
    } else { // not root
      if (tree) { // must have no tree
        cout  << "ERROR: non-root must have no tree info" << endl;
        exit(1);
      } else {
        parent->setOneChild(cur, ch_index);
      }
    }
  }
  else {
    cout << "ERROR: token error" << endl;
    exit(1);
  }
}

