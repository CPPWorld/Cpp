#pragma once
#include "GearBox.h"
// Internal class which should not be exposed. Just for 
// maintainability this class is spereated to new .h and
// .cpp file. It is just an extension of GearBox.cpp but
// required for 'GearBox' interface functions.
#include "GearAssemply.h" 

class GearBox::GearBoxImpl
{
public:
    // Mirrored the 'GearBox' interfaces public functions
    // inside GearBoxImpl class
    void ChangeGear( const int nGear_i );
    void Neutral();
    void Reverse();

    GearBoxImpl();
    ~GearBoxImpl();

private:
    // All the private members functions of interface
    // 'GearBox' is moved to 'GearBoxImpl' class as private
    int CalculateGearRatio();
    int ValidateGearAssemply();

private:
    // All the private members variables of interface
    // 'GearBox'is moved to 'GearBoxImpl' class as private
    GearAssemply m_Gears;
    int m_nCurrentGear = 1;
    double m_nGearRatio = 0.542;
    int m_nGearHandlePosition = 1;
};