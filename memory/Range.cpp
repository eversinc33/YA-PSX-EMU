#include "Range.h"

bool Range::contains(const uint32_t &address) const {
    if (address >= this->start && address < this->start+this->length) {
        return true;
    }
    return false;
}
