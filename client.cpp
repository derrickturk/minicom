#include "minicom.h"
#include "server.h"
#include "windows.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << (argc ? argv[0] : "client")
            << " server-dll\n";
        return 0;
    }

    minicom::module lib(argv[1]);
    if (!lib) {
        std::cerr << "Unable to load " << argv[1] << '\n';
        return 0;
    }

    i_adder *adder = lib.make_class_as<i_adder>("adder");
    if (!adder) {
        std::cerr << "can't get adder\n";
        return 0;
    }

    adder->set_augend(17);
    std::cout << "augend is " << adder->get_augend() << '\n';
    std::cout << "adder->add(5) = " << adder->add(5) << '\n';
    adder->release_ref();

    return 0;
}
