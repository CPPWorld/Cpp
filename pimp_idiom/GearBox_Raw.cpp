#include "GearBox_Raw.h"
#include <iostream>

void GearBox_without_pimpl::ChangeGear(const int nGear_i)
{
    std::cout << "ChangeGear(const int nGear_i)\n";
}

void GearBox_without_pimpl::Neutral()
{
    std::cout << "Neutral()\n";
}

void GearBox_without_pimpl::Reverse()
{
    std::cout << "Reverse()\n";
}

GearBox_without_pimpl::GearBox_without_pimpl()
{
    std::cout << "GGearBox_without_pimpl()\n";
}

GearBox_without_pimpl::~GearBox_without_pimpl()
{
}

int GearBox_without_pimpl::CalculateGearRatio()
{
    std::cout << "CalculateGearRatio()\n";
    return 0;
}

int GearBox_without_pimpl::ValidateGearAssemply()
{   
    std::cout << "ValidateGearAssemply()\n";
    return 0;
}
