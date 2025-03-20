#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <filesystem>

static int eqCount = 0;
static int endCount = 0;
static int gtCount = 0;
static int ltCount = 0;
static int retCount = 0;





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
        else if (command == "function")
        {
            newLine.push_back("("+keyword+")\n");
            for (int i=0; i<std::stoi(value); ++i)
            {
                newLine.push_back("@SP\nA=M\nM=0\n@SP\nM=M+1\n");
            }
        }
        else if (command == "call")
        {
            std::string retLabel = keyword + "$ret." + std::to_string(retCount++);
            newLine.push_back("@"+retLabel+"\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"); // Return label
            newLine.push_back("@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
            newLine.push_back("@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
            newLine.push_back("@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
            newLine.push_back("@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
            newLine.push_back("@SP\nD=M\n@5\nD=D-A\n@"+value+"\nD=D-A\n@ARG\nM=D\n");
            newLine.push_back("@SP\nD=M\n@LCL\nM=D\n");
            newLine.push_back("@"+keyword+"\n0;JMP\n("+retLabel+")\n");
        }
    }
    else if (tokens.size() == 2)
    {
        std::string command = tokens[0];
        std::string label = tokens[1];
        if (command == "label")
        {
            newLine.push_back("("+label+")\n");
        }
        else if (command == "goto")
        {
            newLine.push_back("@"+label+"\n0;JMP\n");
        }
        else if (command == "if-goto")
        {
            newLine.push_back("@SP\nAM=M-1\nD=M\n@"+label+"\nD;JNE\n");
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
        else if (command == "return")
        {
            newLine.push_back("@LCL\nD=M\n@R13\nM=D\n");
            newLine.push_back("@5\nA=D-A\nD=M\n@R14\nM=D\n");
            newLine.push_back("@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n");
            newLine.push_back("@ARG\nD=M+1\n@SP\nM=D\n");
            newLine.push_back("@R13\nAM=M-1\nD=M\n@THAT\nM=D\n");
            newLine.push_back("@R13\nAM=M-1\nD=M\n@THIS\nM=D\n");
            newLine.push_back("@R13\nAM=M-1\nD=M\n@ARG\nM=D\n");
            newLine.push_back("@R13\nAM=M-1\nD=M\n@LCL\nM=D\n");
            newLine.push_back("@R14\nA=M\n0;JMP\n");
            
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
    if (argc != 2) return -1;
    std::string inputPath = argv[1];
    std::string outputPath;
    std::vector<std::string> vmFiles;
    if (std::filesystem::is_directory(inputPath)){
        outputPath = inputPath + std::filesystem::path(inputPath).stem().string() + ".asm";
        for (const auto& entry : std::filesystem::directory_iterator(inputPath))
        {
            if (entry.path().extension() == ".vm")
            {
                vmFiles.push_back(entry.path().string());
            }
        }
    }
    else if (std::filesystem::is_regular_file(inputPath) && std::filesystem::path(inputPath).extension() == ".vm")
    {
        vmFiles.push_back(inputPath);
        outputPath = std::filesystem::path(inputPath).replace_extension(".asm").string();
    }
    else 
    {
        std::cerr << "Invalid input: Must be a .vm file or a directory containing .vm files.\n";
        return -1;
    }
    std::cout << outputPath << std::endl;
    std::ofstream fout(outputPath, std::ofstream::out | std::ofstream::binary);

    if (vmFiles.size() > 1)
    {
        fout    << "// Bootstrap Code\n"
                << "@256\nD=A\n@SP\nM=D\n"  // SP = 256
                << "// Call Sys.init\n"
                << "@Sys.init$ret.0\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
                << "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
                << "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
                << "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
                << "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"
                << "@SP\nD=M\n@5\nD=D-A\n@ARG\nM=D\n"
                << "@SP\nD=M\n@LCL\nM=D\n"
                << "@Sys.init\n0;JMP\n"
                << "(Sys.init$ret.0)\n";
    }
    for (const auto& vmFile : vmFiles)
    {
        std::ifstream fin(vmFile);
        std::string filename = std::filesystem::path(vmFile).stem().string();
        std::cout << " . . . Processing: " << vmFile << std::endl;

        std::string line;
        while (std::getline(fin, line)) 
        {
            line = RemoveWhiteSpaceAndComments(line);
            if (line.empty()) continue;
            std::string assemblyLine = TranslateLine(line, filename);
            fout << assemblyLine;
        }
    }
    std::cout << "Translation Complete! " << outputPath << " Created!" << std::endl;
    return 0;
}