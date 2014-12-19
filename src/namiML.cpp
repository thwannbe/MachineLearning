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
///
/// @section reference_section Reference
/// MACHINE LEARNING - TOM M. MITCHELL
/// 
/// @section purpose_section Purpose
/// Application trial for personal studying
///

#include <iomanip>      ///< for managing console output
#include <cstdlib>      ///< for exit()
#include <cstring>      ///< string compare
#include "namiML.h"

#define VERSION "v1.1"

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
  cout <<    "///" << "  Usage: namiML -m [MODE] -i [INPUT] -t [T-DATA] (-o [OUTPUT])"
  << endl << "///"
  << endl << "///" << "  OPTION LIST"
  << endl << "///"
  << endl << "///" << "    -h       print this help message (no other options)"
  << endl << "///" << "    -m       choose machine learning mode"
  << endl << "///" << "    -i       choose a new instance file"
  << endl << "///" << "    -t       choose a training data file"
  << endl << "///" << "    -o       determine an output file name (optional)"
  << endl << "///"
  << endl << "///" << "  [MODE]     mode type"
  << endl << "///" << "  [INPUT]    input file name"
  << endl << "///" << "  [T-DATA]   training data file name"
  << endl << "///" << "  [OUTPUT]   output file name"
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
/// @brief find given option from arguments vector
/// 
/// @param option option type character
/// @param argc the number of total given arguments
/// @param argv arguments vector
/// @retval index option index from arguments vector
/// @retval 0 error return, not index 0 (pointing executing program itself)
int findOption(char option, int argc, char* argv[]) {
  if (argc <= 1)
    return 0;  // means error return

  for (int i = 1; i < argc; i++)
    if (argv[i][0] == '-')
      if (argv[i][1] == option)
        return i;

  return 0; // no findout  
}

int main(int argc, char* argv[])  ///< taking options
{
  cout <<    "////////////////////////////////////////////////////////////////////////////////"
  << endl << "///"
  << endl << "///" << "           NAtive Mechanic Intelligent Machine Learning Framework"
  << endl << "///" << "                               NamiML " << VERSION
  << endl << "///" << endl;
  
  // check no option, and undefined option
  if (argc == 1) { // no options
    printError("NO OPTIONS");
    namiTerm();
    exit(1);
  } else {
    if (findOption('h', argc, argv) && argc > 2) {
      printError("HELP OPTION COME ALONE");
      namiTerm();
      exit(1);
    } else if (!findOption('h', argc, argv)) {
      bool validOpt = false;
      for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
          if (argv[i][1] != 'm' && argv[i][1] != 'i' && \
              argv[i][1] != 't' && argv[i][1] != 'o') {
            printError("UNKNOWN OPTIONS");
            namiTerm();
            exit(1);
          }
          validOpt = true;
        }
        else if (validOpt) {
          if (argv[i][0] == '-') { // don't worry about -h, already handled
            printError("OPTIONS ERROR");
            namiTerm();
            exit(1);
          }
          validOpt = false;
        }
        else { // no option error
          printError("OPTIONS ERROR");
          namiTerm();
          exit(1);
        }
      }
      if (validOpt) { // normal pair of options should end with validOpt as false
        printError("NO INPUT");
        namiTerm();
        exit(1);
      }
      if (!findOption('m', argc, argv) || !findOption('i', argc, argv) || \
          !findOption('t', argc, argv)) {
        printError("NOT ENOUGH OPTIONS");
        namiTerm();
        exit(1);
      }
    }
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

  ofstream* output;    ///< output file stream
  ifstream* input;     ///< new instance file stream
  ifstream* training;  ///< training data file stream
  ML_Machine* machine; ///< machine learning engine
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
    printError("OUTPUT FILE OPEN ERROR");
    namiTerm();
    exit(1);
  }

  // select machine algorithm
  const char* mode = argv[findOption('m', argc, argv) + 1];
  if (strcmp(mode, "concept") == 0) {
    machine = new CE_Machine(input, training, output);
  }
  else {
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
  
  input->close();
  training->close();
  output->close();
  namiTerm();
  return 0;  
}

////////////////////////////////////////////////////////////////////////////////
///
///  ML_Machine
///

ML_Machine::ML_Machine(istream *input, istream *training, ostream *output)
  : input(input), training(training), output(output) {}

ML_Machine::~ML_Machine() {}

////////////////////////////////////////////////////////////////////////////////
///
///  CE_Machine
///

CE_Machine::CE_Machine(istream *input, istream *training, ostream *output)
  : ML_Machine(input, training, output) {
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

  while (training->peek() != '\n') {
    attrNum *= 10;
    int testVal = training->get();
    if (testVal < 48 || testVal > 57)
      return false;
    attrNum += testVal - 48; // 48 - ASCII code '0' value
  }
  ignoreBlank(training);

  // update attribute number
  size = attrNum;

  // create ce engine
  ce = new CE(size);  

  do {
    bool* td = new bool[size + 1];  // including target attribute
    for (int i = 0; i < attrNum + 1; i++) {
      if (training->eof() || training->bad()) // middle check
        return false;
      
      char c = training->get();
      if (c != 't' && c != 'f') // only 't' or 'f' can read
        return false;
      
      td[i] = (c == 't') ? true : false;
      ignoreBlank(training);
    }
    if (!ce->updateVS(td)) // update version space
      return false;
  } while (!training->eof() && !training->bad());
  
  cout <<    "///" << "                               -- finish"
  << endl << "///" << endl;

  return true;
}

bool CE_Machine::predict()
{
  cout <<    "///" << "  Candidate Elimination Algorithm is predicting..."
  << endl;
  
  ignoreBlank(input); // prevent blank error

  do {
    bool* in = new bool[size];
    for (int i = 0; i < size; i++) {
      if (input->eof() || input->bad()) // middle check
        return false;
      
      char c = input->get();
      if (c != 't' && c != 'f') // only 't' or 'f' can read
        return false;
      
      in[i] = (c == 't') ? true : false;
      ignoreBlank(input);

      // update output too
      (*output) << c << " ";
    }
    char out;
    switch(ce->predict(in)) {
      case r_true: out = 't'; break;
      case r_false: out = 'f'; break;
      case r_dontknow: out = '?'; break;
      default: out = 'x'; break; // error
    }
    (*output) << "=> " << out << endl;
  } while (!input->eof() && !input->bad());
  
  cout <<    "///" << "                               -- finish"
  << endl << "///" << endl;

  return true;
}

void CE_Machine::ignoreBlank(istream *i)
{
  while(i->peek() == '\n' || i->peek() == '\t' || i->peek() == ' ')
    i->get();
}
