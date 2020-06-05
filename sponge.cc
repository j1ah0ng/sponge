#include <cstring>
#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "./sponge.h"

int main(int argc, char *argv[]) {

  // error checking
  if (argc < 2) {
    std::cerr << "bad input\n";
    std::cerr << "please provide more than just the name of the function\n";
    return -1;
  }

  std::vector<char> vec = {};

  // iterate over arguments
  for (int word = 1; word < argc; ++word) {
    State_Machine *mach = new State_Machine();

    int index = 0;
    while (argv[word][index] != '\0') {

      // Initial index
      if (index == 0) {
        // do not edit the character
        for (int i = 0; i < 26; ++i) {
          if (isupper(argv[word][index])) {
            ++(mach->consec_up);
            vec.push_back(argv[word][index]);
            break;
          } else if (islower(argv[word][index])) {
            ++(mach->consec_down);
            vec.push_back(argv[word][index]);
            break;
          }
        }
      } else {
        bool was_except = false;
        for (int i = 0; i < REN; ++i) {
          if (argv[word][index] == EXCEPTIONS[i]) {
            // exceptions are BAD so we get INVERT
            if (isupper(NOT_EXCEPT[i])) {
              ++(mach->consec_up);
            } else {
              ++(mach->consec_down);
            }
            vec.push_back(NOT_EXCEPT[i]);
            was_except = true;
            break;
          }
        }
        if (!was_except) {
          if (isalpha(argv[word][index])) {
            if (mach->next_is_uppercase()) {
              ++(mach->consec_up);
              vec.push_back(toupper(argv[word][index]));
            } else {
              ++(mach->consec_down);
              vec.push_back(tolower(argv[word][index]));
            }
          } else {
            vec.push_back(argv[word][index]);
          }
        }
      }
      ++index;
    }
    vec.push_back(' ');

    delete mach;
  }

  vec.push_back('\0');

  for (int i = 0; i < static_cast<int>(vec.size()); ++i) {
    std::cout << vec[i];
  }

  std::cout << std::endl;
}
