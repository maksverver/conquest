#include "SimplePlayer3.h"
#include <algorithm>
#include "assert.h"
using namespace std;

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

vector<int> SimplePlayer3::pick_starting_countries(
    const World &world, vector<int> options,
    int num_countries, int UNUSED timeout_ms )
{
    std::vector<int> continent_score;
    for (size_t i = 0; i < world.map.continents.size(); ++i)
    {
        int internal_border = 0;
        int external_border = 0;
        for (size_t j = 0; j < world.map.countries.size(); ++j)
        {
            int is_internal = 0, is_external = 0;
            for (size_t k = 0; k < world.map.countries[j].neighbours.size(); ++k)
            {
                int n = world.map.countries[j].neighbours[k];
                if ( world.map.countries[j].continent == (int)i &&
                     world.map.countries[n].continent != (int)i ) is_internal = 1;
                if ( world.map.countries[j].continent != (int)i &&
                     world.map.countries[n].continent == (int)i ) is_external = 1;
            }
            internal_border += is_internal;
            external_border += is_external;
        }
        continent_score.push_back( internal_border + external_border +
            10*std::min(internal_border, external_border) );
    }

    std::vector<std::pair<int, int> > scored_options;
    for (size_t i = 0; i < options.size(); ++i)
    {
        scored_options.push_back(std::make_pair(
            continent_score[world.map.countries[options[i]].continent],
            options[i] ));
    }
    std::sort(scored_options.begin(), scored_options.end());
    for (size_t i = 0; i < options.size(); ++i)
    {
        options[i] = scored_options[i].second;
    }
    while (options.size() > (size_t)(2*num_countries)) options.pop_back();
    return options;
}
