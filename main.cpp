#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <stack>
#include <map>

/**
 * Main idea used from https://sahandsaba.com/understanding-sat-by-implementing-a-simple-sat-solver-in-python.html
 */

const uint8_t BOOLEAN_VALUES[2] = {0, 1};

static bool eagerMatch(char *&line, const char *str)
{
    for (; *str != '\0'; ++str, ++line)
    {
        if (*str != *line)
            return false;
    }
    return true;
}

/**
 * @brief 
 * 
 * @param line 
 */
static void skipWhiteSpace(char *&line)
{
    while ((*line <= 13 && *line > 9) || *line == 32)
        ++line;
}

/**
 * @brief Get the Int object
 * 
 * @param line 
 * @return int64_t 
 */
static int64_t getInt(char *&line)
{
    skipWhiteSpace(line);
    int x = 0;
    bool neg = false;
    if (*line == '-')
        neg = true, ++line;
    while (*line >= '0' && *line <= '9')
        x = (x * 10) + (*line - '0'), ++line;
    return neg ? -x : x;
}

bool readCNFFile(std::string filename, std::vector<std::vector<uint64_t>> &clauses, uint64_t &numvars)
{
    std::string line;
    std::ifstream myfile;
    myfile.open(filename);
    std::vector<uint64_t> bufferClause;

    int64_t numberOfClauses;
    int64_t numberOfVariables;

    while (std::getline(myfile, line))
    {
        if (line[0] == 'c')
            continue;

        char *cLine = new char[line.size() + 1];
        std::strncpy(cLine, line.c_str(), line.size());
        cLine[line.size()] = '\0';
        uint64_t clauseCounter = 0;

        // Read header
        if (cLine[0] == 'p')
        {
            if (eagerMatch(cLine, "p cnf"))
            {
                numvars = getInt(cLine);
            }
            else
            {
                std::cout << "c Wrong header format!" << std::endl;
                exit(1);
            }

            std::cout << "c Header read" << std::endl;
            continue;
        }

        // Read clauses
        while (cLine != NULL)
        {
            uint64_t neg = 0;
            int64_t lit = getInt(cLine);
            if (lit == 0)
                break;
            if (lit < 0) neg = 1;
            uint64_t newLit = (abs(lit) << 1) | neg;
            std::cout << newLit << " ";
            bufferClause.push_back(newLit);
        }
        std::cout << std::endl;
        clauses.push_back(bufferClause);
        bufferClause.clear();
    }
    return 0;
}

std::map<uint64_t, std::vector<std::vector<uint64_t> *>> setup_watchlist(std::vector<std::vector<uint64_t>> &clauses)
{
    std::map<uint64_t, std::vector<std::vector<uint64_t> *>> watchlist;
    for (std::vector<uint64_t> clause : clauses)
    {
        for (uint64_t c : clause) std::cout << c << std::endl;
        std::vector<uint64_t> *clausePointer = &clause;
        watchlist[clause[0]].push_back(clausePointer);
    }

    return watchlist;
}

bool updateWatchlist(std::map<uint64_t, std::vector<std::vector<uint64_t> *>> &watchlist,
                     uint64_t false_literal,
                     std::vector<uint64_t> &assignment)
{
    while (watchlist[false_literal].size() > 0)
    {
        std::vector<uint64_t> *pclause = watchlist[false_literal].back();
        bool found_alt = false;
        for (uint64_t alt_lit : *(pclause))
        {
            uint64_t real_var = alt_lit >> 1;
            uint16_t odd = alt_lit & 1;
            if (assignment[real_var] == -1 || assignment[real_var] == (odd ^ 1))
            {
                found_alt = true;
                watchlist[false_literal].pop_back();
                watchlist[alt_lit].push_back(pclause);
                break;
            }
        }

        if (!found_alt)
            return false;
    }

    return true;
}

bool solve(std::map<uint64_t, std::vector<std::vector<uint64_t> *>> &watchlist,
           std::vector<uint64_t> &assignment,
           uint64_t d)
{
    uint64_t n = assignment.size();
    uint64_t state[n];
    for (int i = 0; i < n; i++)
        state[i] = 0;

    while (1)
    {
        if (d == n)
            return true;
        bool tried = false;
        for (uint8_t a : BOOLEAN_VALUES)
        {   
            //std::cout << ((state[d] >> a) & 1) << std::endl;
            if (((state[d] >> a) & 1) == 0)
            {
                tried = true;
                state[d] = (state[d] | 1) << a;
                assignment[d] = a;
                if (!updateWatchlist(watchlist, (d << 1) | a, assignment))
                {
                    assignment[d] = -1;
                }
                else
                {
                    d++;
                    break;
                }
            }
        }

        if (!tried)
        {
            if (d == 0)
                return false;
            else
            {
                state[d] = 0;
                assignment[d] = -1;
                d--;
            }
        }
    }
}

int main(int argc, char **argv)
{
    std::vector<std::vector<uint64_t>> clauses;
    std::vector<uint64_t> assignment;
    uint64_t numvars;
    readCNFFile(argv[1], clauses, numvars);
    numvars = 2;
    for (uint64_t i = 0; i < numvars + 1; i++)
        assignment.push_back(2);

    std::map<uint64_t, std::vector<std::vector<uint64_t> *>> watchlist = setup_watchlist(clauses);
    if (solve(watchlist, assignment, 1))
    {
        std::cout << "c SATISFIABLE" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "c UNSATISFIABLE" << std::endl;
        return 1;
    }
}