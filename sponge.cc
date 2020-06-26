#ifdef __linux__
#include <X11/Xlib.h>
#include <unistd.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
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
    State_Machine *mach = new State_Machine();

    // iterate over arguments
    for (int word = 1; word < argc; ++word) {
        int index = 0;
        std::ifstream inputFile;
        inputFile.open(argv[word]);
        std::string inputText = argv[word];

        // this argument is a file
        if (inputFile) {
            std::ostringstream fileString;
            fileString << inputFile.rdbuf();
            inputText = fileString.str();
            inputText.erase(
                std::remove(inputText.begin(), inputText.end(), '\n'),
                inputText.end());
        }

        while (inputText[index] != '\0') {
            // Initial index
            if (index == 0) {
                // do not edit the character
                if (isalpha(inputText[index])) {
                    for (int i = 0; i < 26; ++i) {
                        if (isupper(inputText[index])) {
                            ++(mach->consec_up);
                            vec.push_back(inputText[index]);
                            break;
                        } else if (islower(inputText[index])) {
                            ++(mach->consec_down);
                            vec.push_back(inputText[index]);
                            break;
                        }
                    }
                } else
                    vec.push_back(inputText[index]);
            } else {
                bool was_except = false;
                for (int i = 0; i < REN; ++i) {
                    if (inputText[index] == EXCEPTIONS[i]) {
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
                    if (isalpha(inputText[index])) {
                        if (mach->next_is_uppercase()) {
                            ++(mach->consec_up);
                            vec.push_back(toupper(inputText[index]));
                        } else {
                            ++(mach->consec_down);
                            vec.push_back(tolower(inputText[index]));
                        }
                    } else {
                        vec.push_back(inputText[index]);
                    }
                }
            }
            ++index;
        }
        vec.push_back(' ');
    }

    delete mach;

    std::string out(vec.begin(), vec.end());

    std::cout << out << std::endl;

    // add to clipboard, linux only

#ifdef __linux__
    if (isatty(fileno(stdout))) {
        std::ostringstream cmd;
        cmd << "echo -n " << std::quoted(out) << " | xclip -sel clip";
        std::system(cmd.str().c_str());
    }
#endif

    return 0;
}
