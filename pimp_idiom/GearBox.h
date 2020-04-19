// Exposed Interface used by client modules
#pragma once
#include <memory>

// Interface class 'GearBox'
class GearBox
{
public:
    void ChangeGear( const int nGear_i );
    void Neutral();
    void Reverse();

    GearBox();
    ~GearBox();

public:
    // Forward declaration to implementation class
    class GearBoxImpl; 

    // Opaque Pointer using the
    // forward declared class type
    std::unique_ptr<GearBoxImpl>m_GearsImpl;
};