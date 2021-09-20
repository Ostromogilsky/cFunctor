#ifndef CTOCKEN_H
#define CTOCKEN_H



#include "spellCheck.h"
#include <string>
#include <vector>
#include <memory>



/*File contains description of tockens in math expression such as numbers, variables, functions, brackets and arguments delimiter.*/



typedef double (*func_vec)(const std::vector<double>&); //for default functions
class cFunctor;



enum class tocMean
{
    number,
    argument,
    parameter,
    prefix_func, //default
    postfix_func,//default
    bin_func, //default
    user_func,
    open_bracket,
    close_bracket,
    comma, //arguments delimeter
    unknown, //if couldnt interpret tocken
};



enum class prior
{
    bracket = 0,
    sum = 1,
    mult = 2,
    power = 3,
};



namespace hasVal
{
    const bool yes = true;
    const bool no = false;
}



namespace varType
{
    const bool arg = true;
    const bool par = false;
}



namespace bracket
{
    const bool open = true;
    const bool close = false;
}



class cTocken
{
protected:
    std::string mName;
    tocMean mMeaning;
    tocType mType;
    prior mPriority;
    unsigned int mArgNum;
    bool mHasValue;
    static const std::vector<double> mEmptyArgs;
public:
    cTocken(const std::string& name, tocMean meaning, tocType type, prior priority, unsigned int argNum, bool mHasValue);
    virtual ~cTocken() = 0;
    std::string name() const { return mName; }
    tocMean meaning() const { return mMeaning; }
    tocType type() const { return mType; }
    prior priority() const {return mPriority; }
    unsigned int argNum() { return mArgNum; }
    bool hasValue() const { return mHasValue; }

    virtual double value(const std::vector<double>& args = mEmptyArgs) = 0;
};



class cNumber : public cTocken
{
private:
    double mValue;
public:
    cNumber(double value);
    cNumber(const std::string& name);
    virtual ~cNumber() {}

    virtual double value(const std::vector<double>& args = mEmptyArgs) override;
};



class cVariable : public cTocken
{
private:
    double mValue;
public:
    cVariable(const std::string& name, bool isArg);
    virtual ~cVariable() {}

    void setValue(double value) { mValue = value; mHasValue = true; }
    void clearValue() { mValue = 0.0; mHasValue = false; }

    virtual double value(const std::vector<double>& args = mEmptyArgs) override;
};



class cDefaultFunction : public cTocken
{
private:
    func_vec mFunction;
public:
    cDefaultFunction(const std::string& name, int argNum, func_vec f, prior priority, tocMean mean);
    virtual ~cDefaultFunction() {}
    virtual double value(const std::vector<double>& args) override;
};


/*
class cUserFunction : public cTocken
{
private:
    unsigned int mArgNum;
    std::unique_ptr<cFunctor> mFunction;
public:
    cUserFunction(std::string name, const cFunctor& f) :
        cTocken(name, tocMean::user_function, tocType::word) { mFunction = std::make_unique<cFunctor>(f); }
    int argNum() const { return mArgNum; }
};
*/



class cBracket : public cTocken
{
private:
    static int mIDGenerator;
    std::shared_ptr<cDefaultFunction> mFunc;
    int mPairID;
public:
    cBracket(char name, bool isOpen, bool isFunction = false, std::shared_ptr<cDefaultFunction> func = nullptr);
    virtual ~cBracket() {}

    int pairID() const { return mPairID; }
    bool isOpen() const { return mMeaning == tocMean::open_bracket; }
    std::shared_ptr<cDefaultFunction> func() const { return mFunc; }
    virtual double value(const std::vector<double>& args) override;
};



class cArgDelim : public cTocken
{
private:
    virtual double value(const std::vector<double>& args = mEmptyArgs) override;
public:
    cArgDelim(char name);
    virtual ~cArgDelim() {}
};



#endif // CTOCKEN_H
