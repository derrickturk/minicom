#include "server.h"

class adder : public i_adder,
              public minicom::atomic_rc<adder>,
              public minicom::downcast_to<adder, minicom::i_miniobj, i_adder> {
  public:
    adder() noexcept
        : augend_(0) {}

    MINIMETHODIMPL(minicom::i_miniobj*, downcast, const char* iname)
    {
        return minicom::downcast_to<
            adder,
            minicom::i_miniobj,
            i_adder
        >::downcast(iname);
    }

    MINIMETHODIMPL(void, add_ref, void)
    {
        minicom::atomic_rc<adder>::add_ref();
    }

    MINIMETHODIMPL(void, release_ref, void)
    {
        minicom::atomic_rc<adder>::release_ref();
    }

    MINIMETHODIMPL(void, set_augend, int augend)
    {
        augend_ = augend;
    }

    MINIMETHODIMPL(int, get_augend, void)
    {
        return augend_;
    }

    MINIMETHODIMPL(int, add, int addend)
    {
        return addend + augend_;
    }

  private:
    int augend_;
};

REGISTER_CLASS(adder);

MINIEXPORT(minicom::i_miniobj*, minicom_factory, const char *klass)
{
    return minicom::factory_for<adder>::construct(klass);
}
