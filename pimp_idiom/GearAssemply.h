#pragma once
class GearAssemply
{
public:
    void SetGearPosition( const int nPos_i );
    int SetPedalPosition( const double dPedalpos_i );

private:
    void calculateGearPower();

private:
    int m_nNumberOfGearsPlates = 32;
    double m_dCurrentPedalPos = 3.6845;
    int m_nGearPower = 250;
};

