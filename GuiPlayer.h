#ifndef GUI_PLAYER_H_INCLUDED
#define GUI_PLAYER_H_INCLUDED

#include "Player.h"
#include "MapControl.h"

class GuiPlayer : public Player
{
public:
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

protected:
    std::vector<Movement> movements;
};

#endif /* ndef GUI_PLAYER_H_INCLUDED */
