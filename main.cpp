/**
 * Main idea used from https://sahandsaba.com/understanding-sat-by-implementing-a-simple-sat-solver-in-python.html
 */

#include "utilities.h"

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
                    assignment[d] = 2;
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
                assignment[d] = 2;
                d--;
            }
        }
    }
}

void delete_pointers(clause_database &clauses)
{
    for (pvec clause : clauses)
        delete clause;
}

int main(int argc, char **argv)
{
    cout << "c Starting" << endl;

    clause_database clauses;
    vec assignment;
    uint64_t numvars;
    uint64_t numclauses;

    readCNFFile(argv[1], clauses, numvars, numclauses);

    if (clauses.size() != numclauses)
    {
        cout << "c ERROR: Number of clauses do not match with number of clauses read!" << endl;
        exit(1);
    }

    for (uint64_t i = 0; i < numvars + 1; i++)
        assignment.push_back(2);

    watchlist_vec watchlist;
    for (uint64_t w = 0; w < 2 * numvars + 1; w++)
    {
        watchlist.push_back(vector<pvec>());
    }

    setup_watchlist(numvars, watchlist, clauses);

    cout << "c Start solving" << endl;
    if (solve(watchlist, assignment, 1))
    {
        cout << "c SATISFIABLE" << endl;
        delete_pointers(clauses);
        return 0;
    }
    else
    {
        cout << "c UNSATISFIABLE" << endl;
        delete_pointers(clauses);
        return 1;
    }
}