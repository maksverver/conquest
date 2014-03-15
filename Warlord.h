#ifndef WARLORD_H_INCLUDED
#define WARLORD_H_INCLUDED

#include "Player.h"
#include "SimplePlayer2.h"  // TEMP

class Warlord : public SimplePlayer2
{
    virtual std::vector<int> pick_starting_countries(
        const World      &world,
        std::vector<int> options,
        int              num_countries,
        int              timeout_ms );
/*
    virtual std::vector<Placement> place_armies(
        const World &world,
        int         num_armies,
        int         timeout_ms );

    virtual std::vector<Movement> attack_transfer(
        const World &world,
        int         timeout_ms );
*/
};

#endif  /* ndef WARLORD_H_INCLUDED */
