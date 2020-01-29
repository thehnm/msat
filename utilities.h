#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

typedef struct Clause
{
    Clause(vector<uint64_t> l, bool s) : literals(l), satisfied(s) {}
    vector<uint64_t> literals;
    bool satisfied;
} clause, *pclause;

typedef struct ClauseDatabase
{
    ClauseDatabase(uint64_t i) : numclauses(i) {}
    vector<pclause> clauses;
    uint64_t numclauses;
    bool satisfied;
} clause_database;

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
bool readCNFFile(string filename, clause_database &database, uint64_t &numvars, uint64_t &numclauses)
{
    string line;
    ifstream myfile;
    myfile.open(filename);
    vector<uint64_t> buffer;

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
            buffer.push_back((abs(lit) << 1) | neg);
        }

        database.clauses.push_back(new clause(buffer, false));
        database.numclauses = database.numclauses + 1;
        buffer.clear();
    }
    return 0;
}