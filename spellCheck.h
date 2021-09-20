#ifndef SPELLCHECK_H
#define SPELLCHECK_H



#include <string>



/*Utilites for checking correct grammar of provided tockens: numbers, words and signs.*/



enum class tocType
{
    number,
    word,
    sign,
    none,
};




namespace errorCodes
{
    const int ok = 1;
}



bool isWordPart(char c); //isAlpha || c == '_'
bool isDecimalSign(char c);
bool isArgDelim(char c);

//returns 1 if the spelling is correct
int checkSign(std::string s);
int checkWord(std::string s);
int checkNumber(std::string s);
int checkSpelling(tocType type, std::string s);

std::string errorDescription(int code);



#endif // SPELLCHECK_H
