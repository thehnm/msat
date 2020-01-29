/**
 * Main idea used from https://sahandsaba.com/understanding-sat-by-implementing-a-simple-sat-solver-in-python.html
 */

#include "utilities.h"

const uint8_t FALSE = 0;
const uint8_t TRUE = 1;
const uint8_t UNASSIGNED = 2;

const uint8_t BOOLEAN_VALUES[2] = {FALSE, TRUE};

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
                     watchlist &wl,
                     clause_database &database)
{
    for (pclause clause : database.clauses)
    {
        wl[clause->literals.at(0)].push_back(clause);
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
bool updateWatchlist(watchlist &wl,
                     uint64_t false_literal,
                     vector<uint64_t> &assignment)
{
    while (wl[false_literal].size() > 0)
    {
        pclause pc = wl[false_literal].back();
        bool found_alt = false;
        for (uint64_t i = 0; i < pc->literals.size(); i++)
        {
            uint8_t alt_lit = pc->literals.at(i);
            uint64_t alt_var = alt_lit >> 1;
            uint16_t odd = alt_lit & 1;

            if (assignment[alt_var] == UNASSIGNED || assignment[alt_var] == (odd ^ 1))
            {
                found_alt = true;
                wl[false_literal].pop_back();
                wl[alt_lit].push_back(pc);
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
bool solve(watchlist &wl,
           vector<uint64_t> &assignment,
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
                if (!updateWatchlist(wl, (d << 1) | a, assignment))
                {
                    assignment[d] = UNASSIGNED;
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
                assignment[d] = UNASSIGNED;
                d--;
            }
        }
    }
}

void delete_pointers(clause_database &database)
{
    for (pclause clause : database.clauses)
        delete clause;
}

int main(int argc, char **argv)
{
    cout << "c Starting" << endl;

    clause_database database(0);
    uint64_t numvars;
    uint64_t numclauses;

    readCNFFile(argv[1], database, numvars, numclauses);

    if (database.numclauses != numclauses)
    {
        cout << "c ERROR: Number of clauses do not match with number of clauses read!" << endl;
        exit(1);
    }

    vector<uint64_t> assignment(numvars, UNASSIGNED);

    watchlist wl(2 * numvars + 1, vector<pclause>());

    setup_watchlist(numvars, wl, database);

    cout << "c Start solving" << endl;
    if (solve(wl, assignment, 1))
    {
        cout << "c SATISFIABLE" << endl;
        delete_pointers(database);
        return 0;
    }
    else
    {
        cout << "c UNSATISFIABLE" << endl;
        delete_pointers(database);
        return 1;
    }
}