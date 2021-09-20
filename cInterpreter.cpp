#include "cInterpreter.h"
#include <cmath>
#include <stack>
#include <memory>
#include <map>
#define EPS 0.00000000000000000001
#include <iostream>



bool operator<(const cKey& left, const cKey& right)
{
    if(left.c != right.c)
        return left.c < right.c;
    else
        return left.tag < right.tag;
}



cInterpreter::cInterpreter()
{
    mError.clear();
    addDefaultFunction("+", 2,
        [](const std::vector<double>& args)->double { return args[0] + args[1]; }, prior::sum, tocMean::bin_func);
    addDefaultFunction("-", 2,
        [](const std::vector<double>& args)->double { return args[0] - args[1]; }, prior::sum, tocMean::bin_func);
    addDefaultFunction("*", 2,
        [](const std::vector<double>& args)->double { return args[0] * args[1]; }, prior::mult, tocMean::bin_func);
    addDefaultFunction("/", 2,
        [](const std::vector<double>& args)->double
            { if(fabs(args[1]) < EPS) throw std::invalid_argument("Zero denominator."); return args[0] / args[1]; }, prior::mult, tocMean::bin_func);
    addDefaultFunction("^", 2, [](const std::vector<double>& args)->double { return pow(args[0], args[1]); }, prior::power, tocMean::bin_func);
    addDefaultFunction("-", 1, [](const std::vector<double>& args)->double { return -args[0]; }, prior::sum, tocMean::prefix_func);

    addDefaultFunction("sin", 1, [](const std::vector<double>& args)->double { return sin(args[0]); }, prior::power, tocMean::prefix_func);
    addDefaultFunction("cos", 1, [](const std::vector<double>& args)->double { return cos(args[0]); }, prior::power, tocMean::prefix_func);
    addDefaultFunction("abs", 1, [](const std::vector<double>& args)->double { return fabs(args[0]); }, prior::power, tocMean::prefix_func);
    addDefaultFunction("round", 1, [](const std::vector<double>& args)->double { return round(args[0]); }, prior::power,tocMean::prefix_func);

    setArgsDelim(',');
    addBrackets('(', ')');
    addBrackets('[', ']', true, "round");

    addDefaultFunction("sum", 2,
        [](const std::vector<double>& args)->double { return args[0] + args[1]; }, prior::sum, tocMean::prefix_func);
}



cInterpreter::~cInterpreter()
{
     mNumbers.clear();
     mDefaultWords.clear();
     mUserWords.clear();
     mSigns.clear();
     mSplited.clear();
}



void cInterpreter::setArgsDelim(char c)
{
    std::shared_ptr<cTocken> comma = std::make_shared<cArgDelim>(c);
    mSigns.insert({cKey(c, tocTag::nevermind), comma});
}



void cInterpreter::addNumber(double value)
{
    std::shared_ptr<cTocken> num = std::make_shared<cNumber>(value);
    mNumbers.insert({value,  num});
}



void cInterpreter::addNumber(const std::string& value)
{
    std::shared_ptr<cTocken> num = std::make_shared<cNumber>(value);
    mNumbers.insert({num->value(), num});
}



void cInterpreter::addVariable(const std::string& name, bool isArg)
{
    std::shared_ptr<cTocken> var = std::make_shared<cVariable>(name, isArg);
    mUserWords.insert({name, var});
}



void cInterpreter::addDefaultFunction(const std::string& name, int argNum, func_vec f, prior priority, tocMean mean)
{
    auto func = std::make_shared<cDefaultFunction>(name, argNum, f, priority, mean);
    if(func->type() == tocType::sign)
    {
        tocTag tag = tocTag::nevermind;
        auto it = mSigns.find(cKey(name[0], tocTag::nevermind));
        if(it != mSigns.end())
        {
            if(func->argNum() == 1)
            {
                mSigns.insert({cKey(name[0], tocTag::binary), it->second});
                mSigns.erase(it);
                tag = tocTag::unary;
            }
            else if(func->argNum() == 2)
            {
                mSigns.insert({cKey(name[0], tocTag::unary), it->second});
                mSigns.erase(it);
                tag = tocTag::binary;
            }
        }
        mSigns.insert({cKey(name[0], tag), func});
    }
    else
        mDefaultWords.insert({name, func});
}




/*
void cInterpreter::addUserFunction(const std::string&  name, int argNum, const cFunctor& f, prior priority)
{

}
*/



void cInterpreter::addBrackets(char open, char close, bool isFunction, std::string funcName)
{
    std::shared_ptr<cDefaultFunction> func_ptr = nullptr;
    if(isFunction)
    {
        auto it = mDefaultWords.find(funcName);
        if(it == mDefaultWords.end())
            throw std::logic_error("Bracket function wasn't added.");
        func_ptr = std::static_pointer_cast<cDefaultFunction>(it->second);
    }
    std::shared_ptr<cTocken> lb = std::make_shared<cBracket>(open, true, isFunction, func_ptr);
    std::shared_ptr<cTocken> rb = std::make_shared<cBracket>(close, false, isFunction, func_ptr);
    if(open == close)
        throw std::runtime_error("Same chars for open and close bracket.");
    else
    {
        mSigns.insert({cKey(open, tocTag::nevermind), lb});
        mSigns.insert({cKey(close, tocTag::nevermind), rb});
    }
}



tocMean cInterpreter::processTocken(std::string& tockenName, tocType& type, std::list<std::shared_ptr<cTocken>>& tockens)
{
    int errorCode = checkSpelling(type, tockenName);
    if(errorCode != errorCodes::ok)
    {
        mError = errorDescription(errorCode);
        return tocMean::unknown;
    }

    switch(type)
    {
        case tocType::number:
        {
            auto tocken = std::make_shared<cNumber>(tockenName);
            tockens.push_back(tocken);
            break;
        }
        case tocType::sign:
        {
            auto itDiffer = mSigns.find(cKey(tockenName[0], tocTag::nevermind));
            if(itDiffer != mSigns.end())
                tockens.push_back(itDiffer->second);
            else
            {
                tocTag opTag;
                if(tockens.empty())
                    opTag = tocTag::unary;
                else
                {
                    switch(tockens.back()->meaning())
                    {
                    case tocMean::number:
                    case tocMean::argument:
                    case tocMean::parameter:
                    case tocMean::close_bracket:
                    case tocMean::postfix_func:
                        opTag = tocTag::binary;
                        break;
                    default:
                        opTag = tocTag::unary;
                        break;
                    }
                }
                auto itSameOp = mSigns.find(cKey(tockenName[0], opTag));
                if(itSameOp != mSigns.end())
                    tockens.push_back(itSameOp->second);
                else
                {
                    mError = "Unknown sign.";
                    return tocMean::unknown;
                }
            }
            break;
        }
        case tocType::word:
        {
            auto itDefault = mDefaultWords.find(tockenName);
            if(itDefault == mDefaultWords.end())
            {
                auto itUser = mUserWords.find(tockenName);
                if(itUser == mUserWords.end())
                {
                    auto par = std::make_shared<cVariable>(tockenName, varType::par); //as a parameter
                    mUserWords.insert({tockenName, par});
                    tockens.push_back(par);
                }
                else
                    tockens.push_back(itUser->second);
            }
            else
                tockens.push_back(itDefault->second);
            break;
        }
        case tocType::none:
        {
            mError  = "Empty tocken.";
            return tocMean::unknown;
        }
    }
    tockenName.clear();
    type = tocType::none;
    return tockens.back()->meaning();
}



bool cInterpreter::split(const std::string& s)
{
    mError.clear();
    mSplited.clear();
    tocType curType(tocType::none);
    std::string tockenName = "";
    for(auto c : s)
    {
        if(std::isspace(c))
        {
            if(!tockenName.empty())
            {
                if(processTocken(tockenName, curType, mSplited) == tocMean::unknown)
                    return false;
            }
            curType = tocType::none;

        }
        else if(isDecimalSign(c) || std::isdigit(c))
        {
            if(!tockenName.empty() && curType != tocType::number && curType != tocType::word)
            {
                if(processTocken(tockenName, curType, mSplited) == tocMean::unknown)
                    return false;
            }
            if(curType != tocType::word)
                curType = tocType::number;
            tockenName += c;
        }
        else if(isWordPart(c))
        {
            if(!tockenName.empty() && curType != tocType::word)
            {
                if(processTocken(tockenName, curType, mSplited) == tocMean::unknown)
                    return false;
            }
            curType = tocType::word;
            tockenName += c;
        }
        else //sign
        {
            if(!tockenName.empty())
            {
                if(processTocken(tockenName, curType, mSplited) == tocMean::unknown)
                    return false;
            }
            tockenName += c;
            curType = tocType::sign;
            if(processTocken(tockenName, curType, mSplited) == tocMean::unknown)
                return false;
        }
    }
    if(!tockenName.empty())
    {
        if(processTocken(tockenName, curType, mSplited) == tocMean::unknown)
            return false;
    }
    return true;
}

bool cInterpreter::checkBrackets()
{
    if(mSplited.empty())
        return true;

    std::stack<int> IDs; //for open brackets
    for(auto& tocken : mSplited)
    {
        if(tocken->meaning() == tocMean::open_bracket || tocken->meaning() == tocMean::close_bracket)
        {
            std::shared_ptr<cBracket> br = std::static_pointer_cast<cBracket>(tocken);
            if(br->isOpen())
                IDs.push(br->pairID());
            else if(IDs.empty() || IDs.top() != br->pairID())
            {
                mError = "Invalid brackets sequence.";
                return false;
            }
            else
                IDs.pop();
        }
    }
    if(!IDs.empty())
    {
        mError = "Invalid brackets sequence.";
        return false;
    }
    return true;
}



bool cInterpreter::insertImplMult()
{
    auto itMult = mSigns.find({'*', tocTag::nevermind});
    if(itMult == mSigns.end())
    {
        itMult = mSigns.find({'*', tocTag::binary});
        if(itMult == mSigns.end())
            return false;
    }
    if(mSplited.size() < 2)
        return true;
    tocMean mean;
    tocType type;
    bool needMult = false;
    for(auto it = mSplited.begin(); it != mSplited.end(); it++)
    {
        mean = it->get()->meaning();
        type = it->get()->type();
        if(needMult && (type == tocType::word || mean == tocMean::number || mean == tocMean::open_bracket))
            mSplited.insert(it, itMult->second);
        if(mean == tocMean::number || mean == tocMean::argument || mean == tocMean::parameter || mean == tocMean::close_bracket)
            needMult = true;
        else
            needMult = false;
    }
    return true;
}



bool cInterpreter::toRPN()
{
    mRPN.clear();
    std::stack<std::shared_ptr<cTocken>> stack;
    tocMean mean;
    for(auto& tocken : mSplited)
    {
        mean = tocken->meaning();
        switch (mean)
        {
        case tocMean::unknown:
            mError = "Unknown tocken.";
            return false;
        case tocMean::number:
        case tocMean::argument:
        case tocMean::parameter:
        case tocMean::postfix_func:
            mRPN.push_back(tocken);
            break;
        case tocMean::prefix_func:
        case tocMean::user_func:
        case tocMean::open_bracket:
            stack.push(tocken);
            break;
        case tocMean::bin_func:
            while(!stack.empty() && (stack.top()->meaning() == tocMean::prefix_func || stack.top()->priority() >= tocken->priority()))
            {
                mRPN.push_back(stack.top());
                stack.pop();
            }
            stack.push(tocken);
            break;

        case tocMean::comma:
            while(!stack.empty() && stack.top()->meaning() != tocMean::open_bracket)
            {
                mRPN.push_back(stack.top());
                stack.pop();
            }
                if(stack.empty())
                {
                    mError = "Opening bracket or comma missed.";
                    return false;
                }
                break;

         case tocMean::close_bracket:
            while(!stack.empty() && stack.top()->meaning() != tocMean::open_bracket)
            {
                mRPN.push_back(stack.top());
                stack.pop();
            }
            if(stack.empty())
            {
                mError = "Invalid brackets sequence.";
                return false;
            }
            stack.pop();
            if(tocken->hasValue())
                mRPN.push_back(std::static_pointer_cast<cBracket>(tocken)->func());
            break;
        }
    }
    while(!stack.empty())
    {
        tocMean mean = stack.top()->meaning();
        if(stack.top()->type() != tocType::sign && mean != tocMean::prefix_func && mean != tocMean::postfix_func && mean != tocMean::bin_func)
        {
            mError = "Invalid brackets sequence.";
            return false;
        }
        mRPN.push_back(stack.top());
        stack.pop();
    }
    return checkArgs();
}

bool cInterpreter::checkArgs()
{
    if(mRPN.empty())
        return true;
    int operandNum = 0;
    for(auto& tocken: mRPN)
    {
        switch (tocken->meaning())
        {
            case tocMean::argument:
            case tocMean::parameter:
            case tocMean::number:
            {
                operandNum++;
                break;
            }
            case tocMean::bin_func:
            case tocMean::postfix_func:
            case tocMean::prefix_func:
            case tocMean::user_func:
            {
                if(operandNum < tocken->argNum())
                {
                    mError ="Wrong number of arguments.";
                    return false;
                }
                operandNum -= tocken->argNum();
                operandNum++;
                break;
            }
            default:
                return false;
         }
    }
    return true;
}



bool cInterpreter::rpn(const std::string& s, std::list<std::shared_ptr<cTocken>>& rpn)
{
    if(split(s) && checkBrackets() && insertImplMult() && toRPN() && checkArgs())
    {
        rpn = mRPN;
        return true;
    }
    return false;
}
