#include "SimplePlayer3.h"
#include <algorithm>
#include <utility>
#include <iostream>  // debug
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

vector<Placement> SimplePlayer3::place_armies(
    const World &world, int num_armies, int UNUSED timeout_ms )
{
    vector<char> nearby_continent(world.map.continents.size(), 0);
    for (size_t i = 0; i < world.map.countries.size(); ++i)
    {
        if (world.occupations[i].owner <= 0) continue;
        const Country &cy = world.map.countries[i];
        nearby_continent[cy.continent] = true;
        for (size_t n = 0; n < cy.neighbours.size(); ++n)
        {
            nearby_continent[world.map.countries[cy.neighbours[n]].continent] = true;
        }
    }

    this->best_continent = -1;
    int best_score = 0;
    for (size_t i = 0; i < world.map.continents.size(); ++i)
    {
        if (!nearby_continent[i]) continue;

        int my_countries = 0, opponent_armies = 0,
            opponent_countries = 0, neutral_countries = 0;
        for (size_t j = 0; j < world.map.continents[i].countries.size(); ++j)
        {
            const Occupation &occ = world.occupations[world.map.continents[i].countries[j]];
            if (occ.owner > 0) ++my_countries;
            else if (occ.owner < 0) ++opponent_countries, opponent_armies += occ.armies;
            else ++neutral_countries;
        }
        if (neutral_countries + opponent_countries == 0) continue;

        int score;
        score = 500
              - 10*(neutral_countries + opponent_countries) 
              - opponent_armies
              + 5*world.map.continents[i].bonus;
        if (score > best_score)
        {
            best_score = score;
            best_continent = i;
        }
    }
    assert(best_continent >= 0);

    // Put all reinforcements on the country with the most hostile armies
    vector<pair<int, int> > options;
    for (size_t i = 0; i < world.occupations.size(); ++i)
    {
        if (world.occupations[i].owner <= 0) continue;

        int score = 0;
        const vector<int> &neighbours = world.map.countries[i].neighbours;
        for ( vector<int>::const_iterator it = neighbours.begin();
                it != neighbours.end(); ++it )
        {
            if ( world.map.countries[*it].continent == best_continent &&
                 world.occupations[*it].owner <= 0 )
            {
                score += world.occupations[*it].armies;
            }
        }
        options.push_back(make_pair(score, (int)i));
    }
    assert(!options.empty());

    vector<Placement> placements;
    int i = max_element(options.begin(), options.end())->second;
    Placement pl = { i, num_armies };
    placements.push_back(pl);
    return placements;
}

vector<Movement> SimplePlayer3::attack_transfer(
    const World &world, int UNUSED timeout_ms )
{
    // Find fringe countries
    vector<char> fringe(world.map.countries.size(), 0);
    for (size_t i = 0; i < world.map.countries.size(); ++i)
    {
        if (world.occupations[i].owner <= 0) continue;

        const vector<int> &neighbours = world.map.countries[i].neighbours;
        for ( vector<int>::const_iterator it = neighbours.begin();
                it != neighbours.end(); ++it )
        {
            if (world.occupations[*it].owner <= 0) fringe[i] = true;
        }
    }

    vector<Movement> movements;
    vector<char> attacked(world.occupations.size(), 0);
    for (size_t i = 0; i < world.occupations.size(); ++i)
    {
        if (world.occupations[i].owner <= 0) continue;

        int armies = world.occupations[i].armies - 1;
        if (armies <= 0) continue;

        const vector<int> &neighbours = world.map.countries[i].neighbours;

        if (!fringe[i])
        {
            // Transfer from internal to fringe countries:
            for ( vector<int>::const_iterator it = neighbours.begin();
                    it != neighbours.end(); ++it )
            {
                if (fringe[*it])
                {
                    Movement movement = { (int)i, *it, armies };
                    movements.push_back(movement);
                    break;
                }
            }
            continue;
        }

        // Attack opponent in the target continent:
        for ( vector<int>::const_iterator it = neighbours.begin();
                it != neighbours.end(); ++it )
        {
            if (attacked[*it]) continue;
            if ( world.map.countries[*it].continent == best_continent &&
                 world.occupations[*it].owner < 0 )
            {
                int needed = world.occupations[*it].armies*10/6 + 2;
                if (armies >= needed)
                {
                    Movement movement = { (int)i, *it, needed };
                    movements.push_back(movement);
                    armies -= needed;
                    attacked[*it] = true;
                }
            }
        }

        if (armies < world.occupations[i].armies - 1)
        {
            movements.back().armies += armies;
            continue;
        }

        // If no opponent found, attack neutral country instead:
        for ( vector<int>::const_iterator it = neighbours.begin();
                it != neighbours.end(); ++it )
        {
            if (attacked[*it]) continue;
            if ( world.map.countries[*it].continent == best_continent &&
                 world.occupations[*it].owner == 0 )
            {
                int needed = world.occupations[*it].armies*10/6 + 2;
                if (armies >= needed)
                {
                    Movement movement = { (int)i, *it, needed };
                    movements.push_back(movement);
                    armies -= needed;
                    attacked[*it] = true;
                }
            }
        }
        if (armies < world.occupations[i].armies - 1)
        {
            movements.back().armies += armies;
            continue;
        }
    }

    return movements;
}
