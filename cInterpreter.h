#ifndef CINTERPRETER_H
#define CINTERPRETER_H
#include "cTocken.h"
#include <memory>
#include <map>
#include <list>
#include <ostream>



/*Expandable math expression parser. Creates std::list with tockens pointers.*/



enum class tocTag
{
    unary, //for unary operatoins
    binary, //for binary operatoins
    nevermind, //if there are no the same symbols
};



struct cKey
{
    char c;
    tocTag tag;
    cKey(char argC, tocTag argTag) : c(argC), tag(argTag) {}
};
bool operator<(const cKey& left, const cKey& right);



class cInterpreter
{
private:
    std::string mError;
    std::map<double, std::shared_ptr<cTocken>> mNumbers;
    std::map<std::string, std::shared_ptr<cTocken>> mDefaultWords;
    std::map<std::string, std::shared_ptr<cTocken>> mUserWords;
    std::map<cKey, std::shared_ptr<cTocken>> mSigns;

    std::list<std::shared_ptr<cTocken>> mSplited;
    std::list<std::shared_ptr<cTocken>> mRPN;

    void setArgsDelim(char c);
    void addNumber(double value);
    void addNumber(const std::string& value);
    void addDefaultFunction(const std::string&  name, int argNum, func_vec f, prior priority, tocMean mean);
    void addBrackets(char open, char close, bool isFunction = false, std::string funcName = "");

    tocMean processTocken(std::string& tockenName, tocType& type, std::list<std::shared_ptr<cTocken>>& tockens);
    bool split(const std::string& s);
    bool toRPN(); //returns false if errors detected
    bool checkBrackets();
    bool checkArgs();
    bool insertImplMult();
public:
    cInterpreter();
    virtual ~cInterpreter();

    void addVariable(const std::string&  name, bool isArg); //argument or parameter
    //void addUserFunction(const std::string&  name, int argNum, const cFunctor& f, prior priority);
    bool rpn(const std::string& s, std::list<std::shared_ptr<cTocken>>& rpn);
    bool hasError() { return !mError.empty(); }
    std::string error() const { return mError; }
};



#endif // CINTERPRETER_H
