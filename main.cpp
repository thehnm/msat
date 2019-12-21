#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

int64_t convertCStringToInt(const char *literal)
{
    int x = 0;
    bool neg = false;
    if (*literal == '-')
    {
        neg = true;
        ++literal;
    }
    while (*literal >= '0' && *literal <= '9')
    {
        x = (x * 10) + (*literal - '0');
        ++literal;
    }
    if (neg)
    {
        x = -x;
    }
    return x;
}

void readCNFFile(std::vector<std::vector<int64_t>> &clause, std::string filename)
{
    std::string line;
    std::ifstream myfile;
    myfile.open(filename);
    std::vector<int64_t> bufferClause;
    char *token;

    while (std::getline(myfile, line))
    {
        if (line[0] == 'c')
        {
            continue;
        }

        char cLine[line.size() + 1];
        std::strncpy(cLine, line.c_str(), line.size());
        cLine[line.size()] = '\0';

        // Read header
        if (cLine[0] == 'p')
        {
            std::cout << "Header read." << std::endl;
            continue;
        }

        // Read clauses
        token = std::strtok(cLine, " ");
        while (token != NULL)
        {
            int64_t lit = convertCStringToInt(token);
            if (lit == 0)
            {
                break;
            }

            bufferClause.push_back(lit);
            token = strtok(NULL, " ");
        }
        clause.push_back(bufferClause);
        bufferClause.clear();
    }
}

int main(int argc, char **argv)
{
    std::vector<std::vector<int64_t>> clauses;
    std::vector<int64_t> variables;

    readCNFFile(clauses, argv[1]);
    for (std::vector<int64_t> c : clauses)
    {
        for (int64_t l : c)
        {
            std::cout << l << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}