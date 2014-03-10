#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "world.h"

class Player
{
public:
    virtual ~Player() { }

    /* Player returns a selection of `count` countries from `options`
       ordered by preference.  Typically, the player will receive count/2 of
       his preferred countries. */
    virtual std::vector<int> pick_starting_countries(
        const World      &world,        // initial world map
        std::vector<int> options,       // possible starting country indices
        int              num_countries, // number of options to select
        int              timeout_ms     // max. thinking time in ms
    ) = 0;

    /* Each turn, the player must place up to `num_armies` extra armies on
       some of his controlled countries. */
    virtual std::vector<Placement> place_armies(
        const World &world,             // world state at start of this round
        int         num_armies,         // max. number of armies to place
        int         timeout_ms          // max. thinking time in ms
    ) = 0;

    /* After placing armies, the player may make some attack/transfer moves.
       All but one army from each controlled country may participate in either
       a transfer (to another occupied country) or an attack (to a neutral or
       opponent-controlled country). */
    virtual std::vector<Movement> attack_transfer(
        const World &world,             // world state after placement
        int         timeout_ms          // max. thinking time in ms
    ) = 0;

};

#endif /* ndef PLAYER_H_INCLUDED */
