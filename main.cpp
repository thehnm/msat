#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

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

void readCNFFile(std::string filename)
{
    std::string line;
    std::ifstream myfile;
    myfile.open(filename);

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
        }
        // Read clauses
        else
        {
            token = std::strtok(cLine, " ");
            while (token != NULL)
            {
                int64_t lit = convertCStringToInt(token);
                if (lit == 0)
                {
                    break;
                }

                std::cout << lit << ", ";
                token = strtok(NULL, " ");
            }
            std::cout << std::endl;
        }
    }
}

int main(int argc, char **argv)
{
    readCNFFile(argv[1]);
    return 0;
}