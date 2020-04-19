#pragma once
// Interface with out using pimpl idiom
#include "GearAssemply.h" // bad. Client is now dependent on this header file also
class GearBox_without_pimpl
{
public:
    void ChangeGear( const int nGear_i );
    void Neutral();
    void Reverse();

    GearBox_without_pimpl();
    ~GearBox_without_pimpl();

private:
    // bad. Client have compile time dependencies even if the functions are private.
    // Compiler does this for supporting inheritance feature
    int CalculateGearRatio();
    int ValidateGearAssemply();

private:
    // bad. Client have compile time dependencies even if the members are private.
    GearAssemply m_Gears;
    int m_nCurrentGear = 1;
    double m_nGearRatio = 0.542;
    int m_nGearHandlePosition = 1;
};

