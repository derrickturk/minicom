#include "server.h"

class adder : public i_adder, public minicom::atomic_rc<adder> {
  public:
    adder() noexcept
        : augend_(0) {}

    MINIMETHODIMPL(minicom::i_miniobj*, downcast, const char* iname)
    {
        TRY_DOWNCAST(minicom::i_miniobj, iname);
        TRY_DOWNCAST(i_adder, iname);
        return nullptr;
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
    TRY_CONSTRUCT(adder, klass);
    return nullptr;
}
