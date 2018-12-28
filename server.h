#ifndef SERVER_H

#include "minicom.h"

INTERFACE(i_adder, minicom::i_miniobj) {
    MINIMETHOD(void, set_augend, int augend);
    MINIMETHOD(int, get_augend, void);
    MINIMETHOD(int, add, int addend);
};

REGISTER_INTERFACE(i_adder);

#define SERVER_H
#endif
