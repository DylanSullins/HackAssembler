#include <string>
#include <iostream>
#include <fstream>

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
    return line;
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
        std::string assemblyLine = TranslateLine(line);
    }
}