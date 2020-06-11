#include "page.h"

void Page::clear() {
    filename = "";
    blockID = -1;
    pinNum = 0;
    isDirty = false;
    clockReference = false;
    valid = true;
    memset(page, 0, PAGESIZE);
}