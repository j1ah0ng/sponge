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

using sponge::StateMachine;
using sponge::kExceptions;
using sponge::kInvertedExceptions;
using sponge::kLen;

static int hex_color = 0;
static const std::string filter_text = "\\text{} ";

std::string GenHex();

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
    hex_color = rand() % 256;

    std::vector<char> vec = {};
    StateMachine *mach = new StateMachine();
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
                            ++(mach->m_consec_up);
                            vec.push_back(inputText[index]);
                            break;
                        } else if (islower(inputText[index])) {
                            ++(mach->m_consec_down);
                            vec.push_back(inputText[index]);
                            break;
                        }
                    }
                } else
                    vec.push_back(inputText[index]);
            } else {
                bool was_except = false;
                for (int i = 0; i < kLen; ++i) {
                    if (inputText[index] == kExceptions[i]) {
                        // exceptions are BAD so we get INVERT
                        if (isupper(kInvertedExceptions[i])) {
                            ++(mach->m_consec_up);
                        } else {
                            ++(mach->m_consec_down);
                        }
                        vec.push_back(kInvertedExceptions[i]);
                        was_except = true;
                        break;
                    }
                }
                if (!was_except) {
                    if (isalpha(inputText[index])) {
                        if (mach->NextIsUppercase()) {
                            ++(mach->m_consec_up);
                            vec.push_back(toupper(inputText[index]));
                        } else {
                            ++(mach->m_consec_down);
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
                               "\\textcolor{" + GenHex() + "}{");
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
            for (long unsigned int i = out.find(filter_text);
                 i != std::string::npos; i = out.find(filter_text)) {
                out.erase(i, filter_text.length());
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

std::string GenHex() {
    hex_color += 10;
    hex_color %= 360;

    uint32_t rgbColor = 0x00000000;
    double chroma = 1 * 200;
    double m = 200 - chroma;
    double x = chroma * (1 - fabs(fmod((double(hex_color) / 60), 2) - 1));

    if (hex_color >= 0 && hex_color <= 60) {
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
    } else if (hex_color > 60 && hex_color <= 120) {
        // setRGB(x + m, chroma + m, m);
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
    } else if (hex_color > 120 && hex_color <= 180) {
        // setRGB(m, chroma + m, x + m);
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
    } else if (hex_color > 180 && hex_color <= 240) {
        // setRGB(m, x + m, chroma + m);
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
    } else if (hex_color > 240 && hex_color <= 300) {
        // setRGB(x + m, m, chroma + m);
        rgbColor += x + m;
        rgbColor <<= 8;
        rgbColor += m;
        rgbColor <<= 8;
        rgbColor += chroma + m;
        rgbColor <<= 8;
    } else if (hex_color > 300 && hex_color <= 360) {
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
