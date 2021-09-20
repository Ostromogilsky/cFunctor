#ifndef CFUNCTOR_H
#define CFUNCTOR_H
#include "cInterpreter.h"
#include <set>

/*v 2.1*/
/*cFunctor class allows to dynamically create function objects, parsing std::string with math expression.*/
/*Operators +, -, *, /, operator()(const cFunctor&) will be added in the next version.*/


class cFunctor
{
    std::string mError;
    static std::unique_ptr<cInterpreter> mInterpreter;
    std::string mName, mMathString;
    std::vector<double> mArgsVal;
    std::vector<std::shared_ptr<cVariable>> mVars;
    std::list<std::shared_ptr<cTocken>> mRPN;
    static bool mIsInited;
    static std::set<std::string> mDefaultArgs;

    void buildArgVec() { return; };
    template<typename ... Args> void buildArgVec(double first, Args ... other) { mArgsVal.push_back(first); buildArgVec(other...); }
    double calc();
public:
    cFunctor(const std::string& name, const std::string& mathStr, const std::set<std::string>& argNames = mDefaultArgs);
    static void initInterpreter(const cInterpreter& interpreter);
    static bool isInited() { return mIsInited; }

    template<typename ... Args> double operator()(Args ...args);
    bool hasError() { return !mError.empty(); }
    std::string error() const { return mError; }
};



//template realisation/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ... Args>
double cFunctor::operator()(Args ...args)
{
    if(hasError())
        return 0.0;
    buildArgVec(args...);
    if(mVars.size() != mArgsVal.size())
    {
        mError = "Wrong number of arguments provided.";
        return 0.0;
    }
    int i = 0;
    for(auto& var: mVars)
    {
        var->setValue(mArgsVal[i]);
        i++;
    }

    double res = calc();
    for(auto& var: mVars)
        var->clearValue();
    return res;
}



#endif // CFUNCTOR_H
