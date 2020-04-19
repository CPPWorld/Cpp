// Important:
// First Include the header file in which
// GearBoxImpl class is declared.
#include "GearBoxImpl.h"

// Interface header file.
#include "GearBox.h"

GearBox::GearBox(){
    m_GearsImpl = std::make_unique<GearBoxImpl>();
}

GearBox::~GearBox() = default;

void GearBox::ChangeGear(const int nGear_i){
    // Use the m_pGearsImpl and delegate to
    // GearBoxImpl::ChangeGear() function
    m_GearsImpl->ChangeGear( nGear_i );
}

void GearBox::Neutral(){
    // Use the m_pGearsImpl and delegate to
    // GearBoxImpl::Neutral() function
    m_GearsImpl->Neutral();
}

void GearBox::Reverse(){
    // Use the m_pGearsImpl and delegate to
    // GearBoxImpl::Reverse() function
    m_GearsImpl->Reverse();
}