#include "spellCheck.h"



bool isWordPart(char c)
{
    return c == '_' || std::isalpha(c);
}



bool isDecimalSign(char c)
{
    return c == '.';
}



bool isArgDelim(char c)
{
    return c == ',';
}



int checkSign(std::string s)
{
    if(s.length() != 1)
        return -1; //Wrong sign length
    char c = s[0];
    if(std::isdigit(c) || isWordPart(c) || isDecimalSign(c))
        return -2; //Unacceptable symbols in sign
    return errorCodes::ok;
}




int checkWord(std::string s)
{
    if(s.empty())
        return -3; //Empty tocken
    if(!(isWordPart(s[0])))
        return -4; //Wrong symbols in the beginning of the word
    for(auto c : s)
    {
        if(!(std::isdigit(c) || isWordPart(c)))
            return -5; //Unacceptable symbols in the word
    }
    return errorCodes::ok;
}




int checkNumber(std::string s)
{
    if(s.empty())
        return -3; //Empty tocken
    if(isDecimalSign(s[0]) || isDecimalSign(s[s.length() - 1]))
        return -6; //Point in the begining/end of the number
    if(s.length() > 1 && s[0] == '0' && !isDecimalSign(s[1]))
        return -7; //First zero without a point in the number
    bool hasPoint = false;
    for(auto c : s)
    {
        if(!(std::isdigit(c) || isDecimalSign(c)))
            return -8; //Wrong symbols in the number
        if(isDecimalSign(c))
        {
            if(hasPoint)
                return -9; //Several decimal points in the number
            hasPoint = true;
        }
    }
    return errorCodes::ok;
}




int checkSpelling(tocType type, std::string s)
{
    switch (type)
    {
    case tocType::number : return checkNumber(s);
        break;
    case tocType::word : return checkWord(s);
        break;
    case tocType::sign : return checkSign(s);
        break;
    default: return -3; //Empty tocken
        break;
    }
}



std::string errorDescription(int code)
{
    switch (code)
    {
    case 1: return  "No errors.";
        break;
    case -1: return "Unacceptable sign length.";
        break;
    case -2: return "Unacceptable symbols in sign.";
        break;
    case -3: return "Empty tocken.";
        break;
    case -4: return "Unacceptable symbols in the beginning of the word.";
        break;
    case -5: return "Unacceptable symbols in the word.";
        break;
    case -6: return "The decimal point in the begining/end of the number.";
        break;
    case -7: return "The zero in the beginning of the number without a decimal point.";
        break;
    case -8: return "Unacceptable symbols in the number.";
        break;
    case -9: return "Several decimal points in the number.";
        break;
    case -10: return "Tryied to read empty variable.";
        break;
    case -11: return "Wrong number of arguments has provided.";
        break;
    case -12: return "Unknown sign.";
        break;
    default: return "Unknown error code.";
        break;
    }
}
