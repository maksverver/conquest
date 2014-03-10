#ifndef SIMPLE_PLAYER_H_INCLUDED
#define SIMPLE_PLAYER_H_INCLUDED

#include "Player.h"

class SimplePlayer : public Player
{
    virtual std::vector<int> pick_starting_countries(
        const World      &world,
        std::vector<int> options,
        int              num_countries,
        int              timeout_ms );

    virtual std::vector<Placement> place_armies(
        const World &world,
        int         num_armies,
        int         timeout_ms );

    virtual std::vector<Movement> attack_transfer(
        const World &world,
        int         timeout_ms );
};

#endif  /* ndef SIMPLE_PLAYER_H_INCLUDED */
