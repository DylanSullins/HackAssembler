#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <unordered_map>

static int eqCount = 0;
static int endCount = 0;
static int gtCount = 0;
static int ltCount = 0;



std::unordered_map<std::string, std::string> CommandTable =
{
    {"add", "@SP\nAM=M-1\nD=M\nA=A-1\nM=D+M\n"},
    {"sub", "@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D\n"},
    {"neg", "@SP\nA=M-1\nM=-M\n"},
    {"eq",  "@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n@EQUAL_"+(std::to_string(eqCount++))+"\nD;JEQ\n@SP\nA=M-1\nM=0\n@END"+(std::to_string(endCount++))+"\n0;JMP\n(EQUAL)\n@SP\nA=M-1\nM=-1\n(END)"},
    {"gt",  "@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n@GT\nD;JGT\n@SP\nA=M-1\nM=0\n@END\n0;JMP\n(GT)\n@SP\nA=M-1\nM=-1\n(END)"},
    {"lt",  "\n"},
    {"and", "\n"},
    {"or",  "\n"},
    {"not", "\n"},
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

std::string TranslateLine(std::string line, std::string filename)
{
    std::string commentedOrigLine = "// " + line + '\n';
    std::stringstream inSS(line);
    std::string substr, translatedLine;
    std::vector<std::string> tokens;
    std::vector<std::string> newLine;
    newLine.push_back(commentedOrigLine);
    while(inSS >> substr)
    {
        tokens.push_back(substr);
    }
    
    if (tokens.size() == 3 )
    {
        std::string command = tokens[0];
        std::string keyword = tokens[1];
        std::string value = tokens[2];

        std::string segment =   (keyword == "local")    ? "LCL"     :
                                (keyword == "argument") ? "ARG"     :
                                (keyword == "this")     ? "THIS"    : 
                                (keyword == "that")     ? "THAT"    :
                                (keyword == "screen")   ? "SCREEN"  :
                                (keyword == "keyboard") ? "KBD"     : "OTHER";
        std::string ptrAddress = (std::stoi(value) == 0) ? "3" : "4";

        if (command == "push")
        {
            if (keyword == "constant")
            {
                newLine.push_back("@"+value+'\n'+"D=A\n");
            }
            else if (keyword == "temp")
            {
                int addr = 5 + std::stoi(value);
                newLine.push_back("@"+std::to_string(addr)+"\nD=M\n");
            }
            else if (keyword == "local" || keyword == "argument" || keyword == "this" || keyword == "that")
            {

                newLine.push_back("@"+segment+"\nD=M\n@"+value+"\nA=D+A\nD=M\n");
            }
            else if (keyword == "pointer")
            {
                newLine.push_back("@"+ptrAddress+"\nD=M\n");
            }
            else if (keyword == "static")
            {
                std::string staticVar = filename + "." + value;
                newLine.push_back("@"+staticVar+"\nD=M\n");
            }
            newLine.push_back("@SP\nA=M\nM=D\n@SP\nM=M+1\n");
        }
        else if (command == "pop")
        {
            if (keyword == "local" || keyword == "this" || keyword == "that" || keyword == "argument")
            {  
                newLine.push_back("@"+segment+"\nD=M\n@"+value+"\nD=D+A\n@R13\nM=D\n");
                newLine.push_back("@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n");
            }
            else if (keyword == "temp")
            {
                newLine.push_back("@SP\nAM=M-1\nD=M\n");
                int addr = 5 + std::stoi(value);
                newLine.push_back("@"+std::to_string(addr)+"\nM=D\n");
            }
            else if (keyword == "pointer")
            {
                newLine.push_back("@SP\nAM=M-1\nD=M\n@"+ptrAddress+"\nM=D\n");
            }
            else if (keyword == "static")
            {
                std::string staticVar = filename + "." + value;
                newLine.push_back("@SP\nAM=M-1\nD=M\n@"+staticVar+"\nM=D\n");
            }
        }
    }
    else if (tokens.size() == 1)
    {
        std::string command = tokens[0];
        if (command == "add")
        {
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nM=D+M\n");
        }
        else if (command == "sub")
        {
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nM=M-D\n");
        }
        else if (command == "neg")
        {
            newLine.push_back("@SP\nA=M-1\nM=-M\n");
        }
        else if (command == "eq")
        {
            std::string eqLabel = "EQ_" + std::to_string(eqCount++);
            std::string endLabel = "END_" + std::to_string(endCount++);
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n@"+eqLabel+"\nD;JEQ\n@SP\nA=M-1\nM=0\n@"+endLabel+"\n0;JMP\n("+eqLabel+")\n@SP\nA=M-1\nM=-1\n("+endLabel+")\n");
        }
        else if (command == "gt")
        {
            std::string gtLabel = "GT_" + std::to_string(eqCount++);
            std::string endLabel = "END_" + std::to_string(endCount++);
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n@"+gtLabel+"\nD;JGT\n@SP\nA=M-1\nM=0\n@"+endLabel+"\n0;JMP\n("+gtLabel+")\n@SP\nA=M-1\nM=-1\n("+endLabel+")\n");
        }
        else if (command == "lt")
        {
            std::string ltLabel = "LT_" + std::to_string(eqCount++);
            std::string endLabel = "END_" + std::to_string(endCount++);
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n@"+ltLabel+"\nD;JLT\n@SP\nA=M-1\nM=0\n@"+endLabel+"\n0;JMP\n("+ltLabel+")\n@SP\nA=M-1\nM=-1\n("+endLabel+")\n");
        }
        else if (command == "and")
        {
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nM=D&M\n");
        }
        else if (command == "or")
        {
            newLine.push_back("@SP\nAM=M-1\nD=M\nA=A-1\nM=D|M\n");
        }
        else if (command == "not")
        {
            newLine.push_back("@SP\nA=M-1\nM=!M\n");
        }
        else 
        {
            std::cout << "ERROR: COMMAND TOKEN NOT FOUND";
            return "";
        }
    }
    for (int i = 0; i < newLine.size(); ++i) {
        translatedLine += newLine.at(i);
    }
    return translatedLine;
}

int main(int argc, char* argv[])
{
    if (argc != 3) return -1;
    std::string filenameIn, filenameOut;
    filenameIn = argv[1];
    filenameOut = argv[2];

    std::ifstream fin(filenameIn);
    std::ofstream fout(filenameOut, std::ofstream::out | std::ofstream::binary);
    int directoryPos = filenameIn.find_last_of('\\');
    if (directoryPos == -1) directoryPos = 0;
    int extensionPos = filenameIn.find_last_of('.');
    if (extensionPos == -1) return -1;
    std::string filename = filenameIn.substr(directoryPos + 1, extensionPos - directoryPos - 1);
    std::cout << "Beginning translation of file '" << filenameIn << "' to Hack Assembly file '" << filenameOut << "'" << std::endl;
    std::string line;
    while (std::getline(fin, line))
    {
        line = RemoveWhiteSpaceAndComments(line);
        if (line == "" || line == " ")
        {
            continue;
        }
        std::string assemblyLine = TranslateLine(line, filename);
        fout << assemblyLine;
    }
    std::cout << "Translation Complete! " << filenameOut << " Created!" << std::endl;
}