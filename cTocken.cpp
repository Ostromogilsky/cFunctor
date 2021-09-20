#include "cTocken.h"
#include <cmath>
#include <stdexcept>



cTocken::~cTocken()
{
    /*done*/
}



cTocken::cTocken(const std::string& name, tocMean meaning, tocType type, prior priority, unsigned int argNum, bool hasValue) :
    mName(name), mMeaning(meaning), mType(type), mPriority(priority), mArgNum(argNum), mHasValue(hasValue)
{
    int errorCode = checkSpelling(type, name);
    if(errorCode != errorCodes::ok)
        throw std::logic_error(errorDescription(errorCode));
}
const std::vector<double> cTocken::mEmptyArgs = std::vector<double>();


cNumber::cNumber(double value) :
    cTocken(std::to_string(value), tocMean::number, tocType::number, prior::bracket, 0, hasVal::yes), mValue(value)
{
    /*done*/
}



cNumber::cNumber(const std::string& name) :
    cTocken(name, tocMean::number, tocType::number, prior::bracket, 0, hasVal::yes), mValue(std::stod(name))
{
    /*done*/
}



double cNumber::value(const std::vector<double>& args)
{
    if(args.size() != 0)
        throw std::runtime_error("Wrong number of arguments has provided.");
    return mValue;
}



cVariable::cVariable(const std::string& name, bool isArg) :
    cTocken(name, isArg ? tocMean::argument : tocMean::parameter, tocType::word, prior::bracket, 0, hasVal::no), mValue(0.0)
{
    /*done*/
}



double cVariable::value(const std::vector<double>& args)
{
    if(args.size() != 0)
        throw std::runtime_error("Wrong number of arguments has provided.");
    if(!mHasValue)
        throw std::runtime_error("Tryied to read empty variable.");
    return mValue;
}



cDefaultFunction::cDefaultFunction(const std::string& name, int argNum, func_vec f, prior priority, tocMean mean) :
    cTocken(name, mean, isWordPart(name[0]) ? tocType::word : tocType::sign, priority, argNum, hasVal::yes),
    mFunction(f)
{
    /*done*/
}



double cDefaultFunction::value(const std::vector<double>& args)
{
    if(mArgNum != args.size())
        throw std::invalid_argument("Wrong number of arguments has provided.");
    return mFunction(args);
}



cBracket::cBracket(char name, bool isOpen, bool hasValue, std::shared_ptr<cDefaultFunction> func) :
    cTocken(std::string(1, name), isOpen ? tocMean::open_bracket : tocMean::close_bracket, tocType::sign, prior::bracket, 0, hasValue), mFunc(func)
{
    if(isOpen)
        mIDGenerator++;
    mPairID = mIDGenerator;
}

int cBracket::mIDGenerator = 0;



double cBracket::value(const std::vector<double>& args)
{
    if(!mHasValue)
        throw std::runtime_error("Calling value for non function bracket.");
    return mFunc->value(args);
}



cArgDelim::cArgDelim(char name) : cTocken(std::string(1, name), tocMean::comma, tocType::sign, prior::bracket, 0, hasVal::no)
{
    /*done*/
}



double cArgDelim::value(const std::vector<double>& args)
{
    throw std::runtime_error("Calling value for argument delimiter.");
}
