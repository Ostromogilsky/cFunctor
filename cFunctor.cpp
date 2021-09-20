#include "cFunctor.h"
#include <stack>



bool cFunctor::mIsInited = false;

std::unique_ptr<cInterpreter> cFunctor::mInterpreter;

std::set<std::string> cFunctor::mDefaultArgs = std::set<std::string>();

void cFunctor::initInterpreter(const cInterpreter& interpreter)
{
    cFunctor::mInterpreter = std::make_unique<cInterpreter>(interpreter);
    cFunctor::mIsInited = true;
}



cFunctor::cFunctor(const std::string& name, const std::string& mathStr, const std::set<std::string>& argNames) : mName(name), mMathString(mathStr)
{
    mError.clear();
    mArgsVal.clear();
    mVars.clear();
    if(!mIsInited)
        return;
    cFunctor::mInterpreter->rpn(mathStr, mRPN);
    if(mInterpreter->hasError())
    {
        mError = mInterpreter->error();
        return;
    }

    for(auto& tocken: mRPN)
    {
        if(tocken->meaning() == tocMean::parameter)
        {
            auto it = argNames.find(tocken->name());
            bool type;
            if(it != argNames.end() || argNames.empty())
                type = varType::arg;
            else
                type = varType::par;
            bool created = false;
            auto vecIt = mVars.begin();
            for( ; vecIt != mVars.end(); vecIt++)
            {
                if(vecIt->get()->name() == tocken->name())
                {
                    created = true;
                    break;
                }
            }
            if(created)
                tocken = *vecIt;
            else
            {
                std::shared_ptr<cVariable> var = std::make_shared<cVariable>(tocken->name(), type);
                if(tocken->hasValue())
                    var->setValue(tocken->value());
                mVars.push_back(var);
                tocken = var;
            }
        }
    }
    if(argNames.size() > 0 && mVars.size() != argNames.size())
        mError = "Couldn't find arguments' names.";
}



double cFunctor::calc()
{
    if(hasError())
        return 0.0;
    if(mRPN.empty())
    {
        mError = "Trying to calc empty function.";
        return 0.0;
    }
    std::stack<double> operands;
    for(auto& tocken: mRPN)
    {
        switch (tocken->meaning())
        {
            case tocMean::argument:
            case tocMean::parameter:
            case tocMean::number:
            {
                if(!tocken->hasValue())
                {
                    mError ="Trying to read empty variable.";
                    return 0.0;
                }
                operands.push(tocken->value());
                break;
            }
            case tocMean::bin_func:
            case tocMean::postfix_func:
            case tocMean::prefix_func:
            case tocMean::user_func:
            {
                if(operands.size() < tocken->argNum())
                {
                    mError ="Wrong number of arguments.";
                    return 0.0;
                }
                std::vector<double> args;
                args.resize(tocken->argNum());
                for(int i = tocken->argNum() - 1; i >= 0; i--)
                {
                    args[i] = operands.top();
                    operands.pop();
                }
                operands.push(tocken->value(args));
                break;
            }
            default:
            {
                mError = "Wrong symbol in RPN.";
                return 0.0;
            }
        }
    }
    return operands.top();
}
