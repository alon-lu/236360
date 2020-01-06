//
// Created by Liad on 06/01/2020.
//

#ifndef HELLOWORLD_REGPOOL_HPP
#define HELLOWORLD_REGPOOL_HPP

#include "bp.hpp"
#include <vector>
#include <string>

using namespace std;

class regPool{
public:
    unsigned int curr;

    regPool():curr(0){};
    string getReg(){
        return "t" + to_string(curr++);
    }
};


#endif //HELLOWORLD_REGPOOL_HPP
