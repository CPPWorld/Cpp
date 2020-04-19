#include "GearBox.h"
#include "GearBoxImpl.h"

GearBox g_gearBox;

void Car( const int nOperation_i, const int nGear_i = 1 ){
    switch( nOperation_i ){
        case 1: { // Change Gear
            g_gearBox.ChangeGear( nGear_i ); return;
        }
        case 2: { // set to Neutral
            g_gearBox.Neutral(); return;
        }
        case 3: { // set to reverse
            g_gearBox.Reverse(); return;
        }
        default: { // Error
            return;
        }
    }
}

void DriveCar() {
    Car( 1, 1 );// Change Gear to 1st gear
    Car( 1, 2 );// Change Gear to 2nd gear
    Car( 1, 1 );// Change Gear to 1st gear
    Car( 2 ); // set to Neutral
    Car( 3 ); // set to reverse
}

int main(){
    DriveCar();
    return 0;
}