#include "Warlord.h"
#include <algorithm>
using namespace std;

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

vector<int> Warlord::pick_starting_countries( const World UNUSED &world,
        vector<int> options, int num_countries, int UNUSED timeout_ms )
{
    static const vector<int> preferences = {
        /* Australia */
        40,  /* Western Australia */
        38,  /* Indonesia */
        39,  /* New Guinea */
        41,  /* Eastern Australia */
        37,  /* Siam */

        /* South America */
        11,  /* Brazil */
        10,  /* Peru */
         9,  /* Venezuela */
        12,  /* Argentina */

        /* Africa */
        23,  /* Congo */
        22,  /* East Africa */
        20,  /* North Africa */
        21,  /* Egypt */
        24,  /* South Africa */
        25,  /* Madagascar */
    };
    vector<int> selection;
    for (int i : preferences)
    {
        if (find(options.begin(), options.end(), i) != options.end())
        {
            selection.push_back(i);
            if (selection.size() == (size_t)(2*num_countries)) break;
        }
    }
    return selection;
}
/*
vector<Placement> Warlord::place_armies( const World &world, int num_armies,
                                         int UNUSED timeout_ms )
{
    vector<Placement> placements;
    return placements;
}

std::vector<Movement> Warlord::attack_transfer( const World &world,
                                                int UNUSED timeout_ms )
{
    std::vector<Movement> movements;
    return movements;
}
*/