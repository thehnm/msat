/**
 * Main idea used from https://sahandsaba.com/understanding-sat-by-implementing-a-simple-sat-solver-in-python.html
 */

#include "utilities.h"

typedef uint64_t literal;
typedef uint64_t variable;

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

void setClauseSat(watchlist &wl, uint64_t true_literal, bool boolean_value)
{
    for (pclause p : wl[true_literal])
        p->satisfied = boolean_value;
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
                     literal false_literal,
                     vector<variable> &assignment)
{
    while (!wl[false_literal].empty())
    {
        pclause pc = wl[false_literal].back();
        bool found_alt = false;
        for (literal alt_lit : pc->literals)
        {
            variable alt_var = alt_lit >> 1;
            uint64_t odd = alt_lit & 1;
            if (assignment[alt_var] == UNASSIGNED || assignment[alt_var] == odd ^ 1)
            {
                found_alt = true;
                wl[false_literal].pop_back();
                wl[alt_lit].push_back(pc);
                break;
            }
        }
        if (!found_alt)
        {
            return false;
        }
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
           vector<variable> &assignment,
           uint64_t d)
{
    uint64_t n = assignment.size();
    variable state[n];
    for (int i = 0; i < n; i++)
        state[i] = 0;

    while (1)
    {
        cout << d << state[d] << endl;
        if (d == (n-1))
        {
            cout << "v ";
            for (uint64_t i = 1; i < n; ++i)
            {
                if (assignment[i])
                    cout << i << " ";
                else
                    cout << "-" << i << " ";
            }
            cout << endl;
            return true;
        }
        if (d == 0)
            return false;

        switch (state[d])
        {
        case 0:
        {
            state[d] = 1;
            assignment[d] = FALSE;
        }
        break;
        case 1:
        {
            state[d] = 2;
            assignment[d] = TRUE;
        }
        break;
        case 2:
        {
            assignment[d] = UNASSIGNED;
            state[d] = 0;
            --d;
        }
            continue;
            break;
        }
        bool res = updateWatchlist(wl, (d << 1) | (assignment[d]), assignment);
        if (res)
            ++d;
        continue;
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

    vector<variable> assignment(numvars + 1, UNASSIGNED);

    watchlist wl(2 * (numvars + 1), vector<pclause>());

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