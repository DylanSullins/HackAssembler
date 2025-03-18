#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <unordered_map>

std::unordered_map<std::string, std::string> SymbolTable = 
{
    {"local", "LC"},
    {"argument", "ARG"},
    {"this", "THIS"},
    {"that", "THAT"},
    {"screen", "SCREEN"},
    {"keyboard", "KBD"},
};

std::unordered_map<std::string, std::string> CommandTable =
{
    {"add", "@SP\nAM=M-1\nD=M\nA=A-1\nM=D+M\n"},
    {"sub", "@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D\n"},
};

std::string RemoveWhiteSpaceAndComments(std::string line)
{
    int firstChar = line.find_first_not_of(" \n\r\t");
    int lastChar = line.find_last_not_of(" \n\r\t");
    if (firstChar == -1 || lastChar == -1) return "";
    std::string trimmedLine = line.substr(firstChar, lastChar - firstChar + 1);
    int commentStart = trimmedLine.find("//");
    std::string decommentedLine;
    if (commentStart == -1) decommentedLine = trimmedLine;
    else if (commentStart == 0) decommentedLine = "";
    else decommentedLine = trimmedLine.substr(0, commentStart);
    return decommentedLine;
}

std::string TranslateLine(std::string line)
{
    std::string commentedOrigLine = "// " + line + '\n';
    std::stringstream inSS(line);
    std::string substr, translatedLine;
    std::vector<std::string> tokens;
    std::vector<std::string> newLine;
    newLine.push_back(commentedOrigLine);
    while(inSS.good())
    {
        std::getline(inSS, substr, ' ');
        tokens.push_back(substr);
    }
    
    if (tokens.size() == 3 )
    {

        if (tokens[1] == "constant")
        {
            newLine.push_back("@"+tokens[2]+'\n'+"D=A\n");
        }
        else if (tokens[1] == "temp")
        {
            int addr = 5 + std::stoi(tokens[2]);
            newLine.push_back("@"+std::to_string(addr)+'\n');
        }
        else
        {
            if (SymbolTable.find(tokens[1]) != SymbolTable.end()) newLine.push_back("@"+SymbolTable[tokens[1]]+'\n'+"D=M\n");
            else std::cout << "ERROR: SYMBOL TOKEN NOT FOUND";
        }
        if (tokens[0] == "push")
        {
            newLine.push_back("@SP\nA=M\nM=D\n@SP\nM=M+1\n");
        } 
        else if (tokens[0] == "pop")
        {
            newLine.push_back("@SP\nAM=M-1\nD=M\n");
        }
    }
    else if (tokens.size() == 1)
    {
        if (CommandTable.find(tokens[0]) != CommandTable.end()) newLine.push_back(CommandTable.at(tokens[0]));
        else std::cout << "ERROR: COMMAND TOKEN NOT FOUND";
    }
    for (int i = 0; i < newLine.size(); ++i) {
        translatedLine += newLine.at(i);
    }
    return std::accumulate(newLine.begin(), newLine.end(), std::string{});
}

int main(int argc, char* argv[])
{
    if (argc != 3) return -1;
    std::string filenameIn, filenameOut;
    filenameIn = argv[1];
    filenameOut = argv[2];

    std::ifstream fin(argv[1]);
    std::ofstream fout(filenameOut, std::ofstream::out | std::ofstream::binary);

    std::string line;
    while (std::getline(fin, line))
    {
        line = RemoveWhiteSpaceAndComments(line);
        if (line == "" || line == " ")
        {
            continue;
        }
        std::string assemblyLine = TranslateLine(line);
        fout << assemblyLine;
    }
}