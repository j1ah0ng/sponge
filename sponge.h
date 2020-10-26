#ifndef SPONGE_H
#define SPONGE_H

#include <unordered_map>
#include <string>
#include <vector>

using std::unordered_map;
using std::string;
using std::vector;

namespace sponge {

static const char EXCEPTIONS[] = {'C', 'I', 'l', 'K', 'o', 'S',
                                  'u', 'V', 'W', 'X', 'Z'};
static const char NOT_EXCEPT[] = {'c', 'i', 'L', 'k', 'O', 's',
                                  'U', 'v', 'w', 'x', 'z'};
static constexpr int REN = static_cast<int>(sizeof(EXCEPTIONS));

/* a disgustingly simple state machine with two states
 * 1. upper case
 * 2. lower case
 * the transition is defined by whether or not the total number of upper case
 * characters is less than the total number of lower case characters.
 */
struct StateMachine {
    int m_consec_up;
    int m_consec_down;
    StateMachine(void) : m_consec_up(0), m_consec_down(0) {}
    ~StateMachine() {};

    bool NextIsUppercase(void) {
        return (m_consec_up < m_consec_down) ? true : false;
    }
};

struct ArgParser {
    unordered_map<string, string> *m_args;
    vector<string> *m_positional_args;

    ArgParser(int argc, char *argv[]) {

        auto StringIsArg = [&](string s) {
            if (s.length() >= 2) {
                if (s.length() >= 3) {
                    if (s[0] == s[1] && s[0] == '-') {
                        return true;
                    }
                }
                else if (s[0] == '-') {
                    return true;
                }
            }
            else return false;
        };

        // create a vector<string> of all arguments
        m_args = new unordered_map<string, string>();
        auto tokens = new vector<string>();
        for (int i = 0; i < argc; ++i) {
            tokens->push_back(string(const_cast<const char*>(argv[i])));
        }

        // iterate
        for (auto itr = tokens->begin(); itr != tokens->end(); ++itr) {
            // check arguments
            if (StringIsArg(*itr)) {
                // check next
                if (StringIsArg(*(itr + 1))) {
                    m_args->insert({
                            itr->substr(
                                    itr->find_last_of('-') + 1, 
                                    itr->npos),
                            *(itr + 1)});
                    itr = tokens->erase(itr, itr + 1) - 1;
                }
                // single argument
                else {
                    m_args->insert({
                            itr->substr(
                                    itr->find_last_of('-') + 1, 
                                    itr->npos),
                            NULL});
                    itr = tokens->erase(itr) - 1;
                }
            }
        }

        this->m_positional_args = tokens;
    }

    ~ArgParser() {
        delete this->m_args;
        delete this->m_positional_args;
    }

    bool Has(string arg) {
        return this->m_args->find(arg) != m_args->end();
    }

    string GetParameter(string arg) {
        return this->m_args->find(arg)->second;
    }
};
 
} // namespace sponge

#endif
