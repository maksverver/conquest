#ifndef SIMPLE_PLAYER3_H_INCLUDED
#define SIMPLE_PLAYER3_H_INCLUDED

#include "SimplePlayer2.h"

class SimplePlayer3 : public SimplePlayer2
{
    virtual std::vector<int> pick_starting_countries(
        const World      &world,
        std::vector<int> options,
        int              num_countries,
        int              timeout_ms );

protected:
    int best_continent;
};

#endif  /* ndef SIMPLE_PLAYER3_H_INCLUDED */
