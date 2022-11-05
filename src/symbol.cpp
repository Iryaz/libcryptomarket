#include <libcryptomarket.h>

using namespace libcryptomarket;

int GetPrecision(double val)
{
    string step_s = std::to_string(val);
    bool flag = false;
    int digit = 0;
    for (auto c : step_s) {
        if (c == '.') {
            flag = true;
            digit++;
            continue;
        }

        if (flag == true) {
            int c1 = std::atoi(&c);
            if (c1 > 0) {
                return digit;
            }

            digit++;
        }
    }

    return digit;
}

Symbol::Symbol(bool valid)
{
   Valid_ = valid;
}

bool Symbol::IsValid()
{
    return Valid_;
}

int Symbol::GetPricePrecision()
{
    return GetPrecision(PriceStep_);
}

int Symbol::GetQtyPrecision()
{
    return GetPrecision(QtyStep_);
}

double Symbol::GetPriceStep()
{
    return PriceStep_;
}

double Symbol::GetQtyStep()
{
    return QtyStep_;
}

void Symbol::SetPriceStep(double step)
{
    PriceStep_ = step;
}

void Symbol::SetQtyStep(double step)
{
    QtyStep_ = step;
}

void Symbol::SetExchange(const string& name)
{
    Exchange_ = name;
}

const string& Symbol::GetExchange()
{
    return Exchange_;
}
