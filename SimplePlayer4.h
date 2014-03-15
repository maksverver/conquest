#ifndef SIMPLE_PLAYER4_H_INCLUDED
#define SIMPLE_PLAYER4_H_INCLUDED

#include "Player.h"
#include <vector>
#include <string>

/* Like SimplePlayer2, but with static country preference ordering a la Warlord,
   and defends its continents (unlike SimplePlayer2) */
class SimplePlayer4 : public Player
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

private:
    std::vector<char> locked;
    int best_continent;
};

#endif  /* ndef SIMPLE_PLAYER4_H_INCLUDED */
