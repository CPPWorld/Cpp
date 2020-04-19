#include "GearBoxImpl.h"
#include <iostream>
void GearBox::GearBoxImpl::ChangeGear( const int nGear_i ){
    // Actual implementation of ChangeGear() functionality 
    std::cout << "ChangeGear(const int nGear_i)\n";
}

void GearBox::GearBoxImpl::Neutral(){
    // Actual implementation of Neutral() functionality 
    std::cout << "Neutral()\n";
}

void GearBox::GearBoxImpl::Reverse(){
    // Actual implementation of Reverse() functionality 
    std::cout << "Reverse()\n";
}

GearBox::GearBoxImpl::GearBoxImpl()
{
    std::cout << "GearBoxImpl Ctor\n";
}

GearBox::GearBoxImpl::~GearBoxImpl()
{
        std::cout << "GearBoxImpl Dtor\n";
}


// pirvate implementations
int GearBox::GearBoxImpl::CalculateGearRatio(){
    std::cout << "CalculateGearRatio()\n";
    return 0;
}

int GearBox::GearBoxImpl::ValidateGearAssemply(){
    std::cout << "ValidateGearAssemply()\n";
    return 0;
}