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

    for (minicom::class_count_t i = 0; i < lib.class_count(); ++i) {
        auto klass = lib.class_list()[i];
        std::cout << "using class: " << klass << '\n';

        i_adder *adder = lib.make_class_as<i_adder>(klass);
        if (!adder) {
            std::cerr << "can't get i_adder for " << klass << '\n';
            return 0;
        }

        adder->set_augend(17);
        std::cout << "augend is " << adder->get_augend() << '\n';
        std::cout << "adder->add(5) = " << adder->add(5) << '\n';
        adder->release_ref();
    }

    return 0;
}
