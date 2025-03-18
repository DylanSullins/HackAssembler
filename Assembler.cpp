#include <fstream>
#include <string>
#include <iostream>
#include <bitset>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, std::string> CompTable = {
    {"0", "0101010"},
    {"1", "0111111"},
    {"-1", "0111010"},
    {"D", "0001100"},
    {"A", "0110000"},
    {"!D", "0001101"},
    {"!A", "0110001"},
    {"D+1", "0011111"},
    {"A+1", "0110111"},
    {"D-1", "0001110"},
    {"A-1", "0110010"},
    {"D+A", "0000010"},
    {"D-A", "0010011"},
    {"A-D", "0000111"},
    {"D&A", "0000000"},
    {"D|A", "0010101"},
    {"M", "1110000"},
    {"!M", "1110001"},
    {"-M", "1110011"},
    {"M+1", "1110111"},
    {"M-1", "1110010"},
    {"D+M", "1000010"},
    {"D-M", "1010011"},
    {"M-D", "1000111"},
    {"D&M", "1000000"},
    {"D|M", "1010101"},
};

std::unordered_map<std::string, std::string> DestTable = {
    {"", "000"},
    {"M", "001"},
    {"D", "010"},
    {"DM", "011"},
    {"MD", "011"},
    {"A", "100"},
    {"AM", "101"},
    {"AD", "110"},
    {"AMD", "111"},
    {"ADM", "111"},

};

std::unordered_map<std::string, std::string> JumpTable = {
    {"", "000"},
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"},
};
std::unordered_map<std::string, std::string> LabelTable = {
    {"SP",     "0000000000000000"},
    {"LCL",    "0000000000000001"},
    {"ARG",    "0000000000000010"},
    {"THIS",   "0000000000000011"},
    {"THAT",   "0000000000000100"},
    {"SCREEN", "0100000000000000"},
    {"KBD",    "0110000000000000"},
};


std::vector<char> digitChars = {'0','1','2','3','4','5','6','7','8','9'};

std::string RemoveWhiteSpaceAndComments(std::string line)
{
    int firstChar = line.find_first_not_of(" \n\r\t");
    int lastChar = line.find_last_not_of(" \n\r\t");
    if (firstChar == -1 || lastChar == -1) return "";
    int commentStart = line.find("//");
    std::string trimmedLine = line.substr(firstChar, lastChar + 1);
    std::string decommentedLine;
    if (commentStart == -1) decommentedLine = trimmedLine;
    else decommentedLine = trimmedLine.substr(0, commentStart);
    return decommentedLine;
}

void FirstPass(std::ifstream& fin, int& lineNum)
{
    std::cout << "First Pass on File . . . " << std::endl;
    std::string line;
    while (std::getline(fin, line))
    {
        line = RemoveWhiteSpaceAndComments(line);
        if (line == "") continue;
        ++lineNum;
        if (line[0] == '(') 
        {
            int rightParenPos = line.find(")");
            std::string label = line.substr(1, rightParenPos - 1);
            LabelTable[label] = std::bitset<16>(--lineNum).to_string();
            std::cout << "  ---> Label: " << label << " at line " << lineNum << " or " << LabelTable[label] << std::endl;
        }
    }
}

std::string ConvertLineToBinary(std::string line, int& lineNum, int& nextVarAddr)
{
    if (line[0] == '@')
    {
        
        // A-Instructions
        std::string instruction = line.substr(1,line.length());
        if (!isdigit(instruction[0]))
        {
            if (LabelTable.find(instruction) == LabelTable.end())
            {
                LabelTable[instruction] = std::bitset<16>(nextVarAddr++).to_string();
            }
            return LabelTable[instruction];
        }
        else
        {
            int decInstruction = std::stoi(instruction);
            std::string binaryInstruction = std::bitset<15>(decInstruction).to_string();
            binaryInstruction = "0" + binaryInstruction;
            return binaryInstruction;
        }
    } 
    else if (line[0] == '(')
    {
        return "";
    }
    else 
    {
        // C-Instructions
        int equalsPos = line.find("=");
        int semiColonPos = line.find(";");
        std::string dest, comp, jump = "";
        // Dest Parse
        if (equalsPos != -1) 
        {
            dest = line.substr(0, equalsPos);
        }
        // Jump Parse
        if (semiColonPos != -1) {
            jump = line.substr(semiColonPos+1, -1);
        }
        // Comp Parse
        if (equalsPos != -1 && semiColonPos != -1) {
            comp = line.substr(equalsPos+1, semiColonPos);
        }
        else if (equalsPos == -1 && semiColonPos != -1) 
        {
            comp = line.substr(0, semiColonPos);
        }
        else if (equalsPos != -1 && semiColonPos == -1)
        {
            comp = line.substr(equalsPos+1, -1);
        }
        // Conversion to bits
        std::string compBits, destBits, jumpBits;
        compBits = CompTable[comp];
        destBits = DestTable[dest];
        jumpBits = JumpTable[jump];
        return "111" + compBits + destBits + jumpBits;
    }
    return "";
}

void ProcessLine(std::string line, int& lineNum, int& nextVarAddr, std::ofstream& fout)
{
    line = RemoveWhiteSpaceAndComments(line);
    if (line == "")
    {
        --lineNum;
        return;
    }
    std::string binaryLine = ConvertLineToBinary(line, lineNum, nextVarAddr);
    if (binaryLine == "")
    {
        --lineNum;
        return;
    }
    fout << binaryLine << '\n';
}

int main(int argc, char *argv[])
{
    if (argc < 3) return -1;
    std::string filename = argv[1];
    std::string filenameOut = argv[2];
    std::cout << "Assembling file '" << filename << "' to binary Hack file '" << filenameOut << "'" << std::endl;
    
    std::ifstream fin(filename);
    std::ofstream fout(filenameOut, std::ofstream::out | std::ofstream::binary);
    for (int i=0; i <= 15; ++i) {
        LabelTable["R" + std::to_string(i)] = std::bitset<16>(i).to_string();
    }
    int nextVarAddr = 16;
    std::string line;
    int lineNum = 0;
    FirstPass(fin, lineNum);
    fin.clear();
    fin.seekg(0);
    std::cout << "Second Pass on File . . . " << std::endl;
    while (std::getline(fin, line))
    {
        ProcessLine(line, lineNum, nextVarAddr, fout);
        ++lineNum;
    }
    std::cout << "Assembly Complete! " << filenameOut << " Created!" << std::endl;
}