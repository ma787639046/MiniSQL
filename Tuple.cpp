#include "Tuple.h"

void Tuple::showTuple() {
    for (size_t i = 0; i < keys.size(); i++) {
        switch (keys[i].type)
        {
        case INT:	std::cout << keys[i].INT_VALUE << "\t";
            break;
        case FLOAT:	std::cout << keys[i].FLOAT_VALUE << "\t";
            break;
        default:    std::cout << keys[i].STRING_VALUE << "\t";
            break;
        }
    }
    std::cout << std::endl;
}