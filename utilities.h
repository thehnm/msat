#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

typedef vector<uint64_t> *pclause;
typedef vector<uint64_t> clause;
typedef vector<pclause> clause_database;
typedef vector<vector<pclause>> watchlist;

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
bool readCNFFile(string filename, clause_database &clauses, uint64_t &numvars, uint64_t &numclauses)
{
    string line;
    ifstream myfile;
    myfile.open(filename);
    vector<uint64_t> bufferClause;

    int64_t numberOfClauses;
    int64_t numberOfVariables;

    while (getline(myfile, line))
    {
        if (line[0] == 'c')
            continue;

        char *cLine = new char[line.size() + 1];
        strncpy(cLine, line.c_str(), line.size());
        cLine[line.size()] = '\0';
        uint64_t clauseCounter = 0;

        // Read header
        if (cLine[0] == 'p')
        {
            if (eagerMatch(cLine, "p cnf"))
            {
                numvars = getInt(cLine);
                cout << "c Number of variables: " << numvars << endl;
                numclauses = getInt(cLine);
                cout << "c Number of clauses: " << numclauses << endl;
            }
            else
            {
                cout << "c Wrong header format!" << endl;
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
            if (lit < 0)
                neg = 1;
            uint64_t newLit = (abs(lit) << 1) | neg;
            bufferClause.push_back(newLit);
        }

        clauses.push_back(new clause(bufferClause));
        bufferClause.clear();
    }
    return 0;
}