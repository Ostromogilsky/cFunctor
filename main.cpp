#include "cFunctor.h"
#include <iostream>

/*usage of cFunctor*/
int main(void)
{
    cInterpreter i;
    cFunctor::initInterpreter(i);
    cFunctor f("f", "2sin x cos x *1.1/abs y"); //any math string

    std::cout << f(1, 2) << "\n"; //prints zero if there is an error
    //no exceptions because probable Qt usage
    std::cout << f.error().c_str() << "\n";
    return 0;
}
