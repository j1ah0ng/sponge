#ifdef __linux__
#include <X11/Xlib.h>
#include <unistd.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "./sponge.h"

static int hexColor = 0;
static const std::string filterText = "\\text{} ";

std::string genHex();

int main(int argc, char *argv[]) {
    // error checking
    if (argc < 2) {
        std::cerr << "bad input\n";
        std::cerr << "please provide more than just the name of the function\n";
        std::cerr << "use '-l' to generate output in latex markup\n";
        std::cerr << "use '-n' to generate new lines for each word\n";
        std::cerr << "use '-nlc' to colorize latex output\n";
        return -1;
    }

    srand(time(0));
    hexColor = rand() % 256;

    std::vector<char> vec = {};
    State_Machine *mach = new State_Machine();
    int startingIdx = 1;

    // allow for latex generation with flag -l
    bool generateLatex = false;
    bool newLine = false;
    bool colorize = false;
    if (argv[1] == static_cast<std::string>("-l")) {
        generateLatex = true;
        startingIdx = 2;
    }
    if (argv[1] == static_cast<std::string>("-nl") ||
        argv[1] == static_cast<std::string>("-ln") ||
        argv[1] == static_cast<std::string>("-nlc")) {
        newLine = true;
        generateLatex = true;
        startingIdx = 2;
        if (argv[1] == static_cast<std::string>("-nlc")) {
            colorize = true;
        }
    }
    if (argv[1] == static_cast<std::string>("-n")) {
        newLine = true;
        startingIdx = 2;
    }

    // iterate over arguments
    for (int word = startingIdx; word < argc; ++word) {
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

    if (generateLatex && !newLine) {
        out = "\\( \\Huge \\text{" + out.substr(0, out.length() - 1) + "} \\)";
    }

    if (newLine) {
        std::deque<int> newLinePos;
        for (unsigned int i = 0; i < out.length(); i++) {
            if (out[i] == ' ' && i != out.length() - 1) {
                out[i] = '\n';
            }
        }

        if (colorize) {
            int outLen = out.length() - 1;

            for (int i = 0; i < outLen; i++) {
                if (out[outLen - i - 1] != '\n' && out[outLen - i - 1] != ' ' &&
                    outLen - i != 0) {
                    out.insert(outLen - i - 1,
                               "\\textcolor{" + genHex() + "}{");
                    out.insert(outLen - i - 1, "} ");
                }
            }
        }

        for (unsigned int i = 0; i < out.length(); i++) {
            if (out[i] == '\n') {
                newLinePos.push_front(i);
            }
        }

        if (generateLatex && out.length() != 0) {
            out.insert(out.length() - 1, "} \\)");
            for (unsigned int i = 0; i < newLinePos.size(); i++) {
                out.insert(newLinePos[i] + 1, "\\( \\Huge \\text{");
                out.insert(newLinePos[i], "} \\)");
            }
            out.insert(0, "\\( \\Huge \\text{");
        }
        if (colorize) {
            for (long unsigned int i = out.find(filterText);
                 i != std::string::npos; i = out.find(filterText)) {
                out.erase(i, filterText.length());
            }
        }
    }

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

std::string genHex() {
    hexColor += 10;
    hexColor %= 360;

    uint32_t rgbColor = 0x00000000;
    double chroma = 1 * 200;
    double m = 200 - chroma;
    double x = chroma * (1 - fabs(fmod((double(hexColor) / 60), 2) - 1));

    if (hexColor >= 0 && hexColor <= 60) {
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
    } else if (hexColor > 60 && hexColor <= 120) {
        // setRGB(x + m, chroma + m, m);
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
    } else if (hexColor > 120 && hexColor <= 180) {
        // setRGB(m, chroma + m, x + m);
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
    } else if (hexColor > 180 && hexColor <= 240) {
        // setRGB(m, x + m, chroma + m);
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
    } else if (hexColor > 240 && hexColor <= 300) {
        // setRGB(x + m, m, chroma + m);
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
    } else if (hexColor > 300 && hexColor <= 360) {
        // setRGB(chroma + m, m, x + m);
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
    } else {
        // setRGB(m, m, m);
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
    }

    rgbColor += 0xFF;

    std::stringstream stream;
    stream << "#" << std::setfill('0') << std::setw(sizeof(uint32_t) * 2)
           << std::hex << rgbColor;

    return stream.str();
}
