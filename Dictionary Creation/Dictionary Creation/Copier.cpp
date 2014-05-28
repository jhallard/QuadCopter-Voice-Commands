#include <fstream> 
#include <string> 
#include <iostream> 
#include <vector> 
  
int main() { 
    std::cout << "Creating input/output files" << std::endl; 
    std::ifstream dictionary("../cmu07a.dic"), 
                  input("../words.txt"); 
    std::ofstream output("../qc_com.dic"); 
  
    std::cout << "Loading CMU Dictionary" << std::endl; 
    int letterStart[26]; 
    for (int i = 0; i < 26; i++) 
        letterStart[i] = -1; 
    std::vector<std::string> dict; 
    dict.reserve(100000); 
    int j = 0, 
        count = 0; 
  
    while (!dictionary.eof()) { 
        std::string line; 
        std::getline(dictionary, line); 
        dict.push_back(line); 
  
        if (j < 26 && letterStart[j] == -1 && line[0] - 97 == j) { 
            letterStart[j] = count; 
            j++; 
        } 
        count++; 
    } 
  
    std::cout << "Finding Words" << std::endl; 
    while (!input.eof()) { 
        std::string word; 
        std::getline(input, word); 
        std::cout << word << std::endl; 
  
        //Assumes only words will be in input file. 
        int start = letterStart[word[0] - 97]; 
        int end; 
        if (word[0] - 97 < 25) 
            end = letterStart[(word[0] - 97) + 1]; 
        else
            end = count; 
  
        for(int i = start; i < end; i++) { 
            std::string line = dict[i]; 
            std::string dictWord = line.substr(0, line.find_first_of('\t')); 
  
            if (word == dictWord) 
                output << line << std::endl; 
            else if (word == dictWord.substr(0, word.length()) && dictWord[word.length()] == '(') 
                output << line << std::endl; 
        } 
    } 
  
    return 0; 
}