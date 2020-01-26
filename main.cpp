#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <stack>
#include <map>

typedef std::vector<uint64_t>* pvec;
typedef std::vector<uint64_t> vec;
typedef std::vector<pvec> clause_database;
typedef std::vector<std::vector<pvec>> watchlist_vec;

/**
 * Main idea used from https://sahandsaba.com/understanding-sat-by-implementing-a-simple-sat-solver-in-python.html
 */

const uint8_t BOOLEAN_VALUES[2] = {0, 1};

// Utility functions

/**
 * @brief 
 * 
 * @param line 
 * @param str 
 * @return true 
 * @return false 
 */
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

// SAT Solver

/**
 * @brief 
 * 
 * @param filename 
 * @param clauses 
 * @param numvars 
 * @return true 
 * @return false 
 */
bool readCNFFile(std::string filename, clause_database &clauses, uint64_t &numvars, uint64_t &numclauses)
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
                std::cout << "c Number of variables: " << numvars << std::endl;
                numclauses = getInt(cLine);
                std::cout << "c Number of clauses: " << numclauses << std::endl;
            }
            else
            {
                std::cout << "c Wrong header format!" << std::endl;
                exit(1);
            }
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
            bufferClause.push_back(newLit);
        }

        clauses.push_back(new vec(bufferClause));
        bufferClause.clear();
    }
    return 0;
}

/**
 * @brief Set the up watchlist object
 * 
 * @param numvars 
 * @param watchlist 
 * @param clauses 
 * @return true 
 * @return false 
 */
bool setup_watchlist(uint64_t numvars,
                     watchlist_vec &watchlist,
                     clause_database &clauses)
{
    for (pvec clause : clauses)
    {
        watchlist[clause->at(0)].push_back(clause);
    }
    return true;
}

/**
 * @brief 
 * 
 * @param watchlist 
 * @param false_literal 
 * @param assignment 
 * @return true 
 * @return false 
 */
bool updateWatchlist(watchlist_vec &watchlist,
                     uint64_t false_literal,
                     vec &assignment)
{
    while (watchlist[false_literal].size() > 0)
    {
        pvec pclause = watchlist[false_literal].back();

        bool found_alt = false;

        for (uint64_t i = 0; i < pclause->size(); i++)
        {
            uint64_t real_var = pclause->at(i) >> 1;
            uint16_t odd = pclause->at(i) & 1;

            if (assignment[real_var] == 2 || assignment[real_var] == (odd ^ 1))
            {
                found_alt = true;
                watchlist[false_literal].pop_back();
                watchlist[pclause->at(1)].push_back(pclause);
                break;
            }
        }

        if (!found_alt)
            return false;
    }

    return true;
}

/**
 * @brief 
 * 
 * @param watchlist 
 * @param assignment 
 * @param d 
 * @return true 
 * @return false 
 */
bool solve(watchlist_vec &watchlist,
           vec &assignment,
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

void delete_pointers(clause_database &clauses)
{
    for (pvec clause : clauses) delete clause;
}

int main(int argc, char **argv)
{
    std::cout << "c Starting" << std::endl;

    clause_database clauses;
    vec assignment;
    uint64_t numvars;
    uint64_t numclauses;

    readCNFFile(argv[1], clauses, numvars, numclauses);

    if (clauses.size() != numclauses)
    {
        std::cout << "c ERROR: Number of clauses do not match with number of clauses read!" << std::endl;
        exit(1);
    }

    for (uint64_t i = 0; i < numvars + 1; i++)
        assignment.push_back(2);

    watchlist_vec watchlist;
    for (uint64_t w = 0; w < 2*numvars+1; w++)
    {
        watchlist.push_back(std::vector<pvec>());
    }

    setup_watchlist(numvars, watchlist, clauses);
    
    std::cout << "c Start solving" << std::endl;
    if (solve(watchlist, assignment, 1))
    {
        std::cout << "c SATISFIABLE" << std::endl;
        delete_pointers(clauses);
        return 0;
    }
    else
    {
        std::cout << "c UNSATISFIABLE" << std::endl;
        delete_pointers(clauses);
        return 1;
    }
}