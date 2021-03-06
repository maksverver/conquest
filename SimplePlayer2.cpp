#include "SimplePlayer2.h"
#include <algorithm>
#include <utility>
#include <iostream>  // debug
#include "assert.h"
using namespace std;

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

vector<int> SimplePlayer2::pick_starting_countries(
    const World UNUSED &world, vector<int> options,
    int num_countries, int UNUSED timeout_ms )
{
    // For now, just pick randomly:
    random_shuffle(options.begin(), options.end());
    while (options.size() > (size_t)(2*num_countries)) options.pop_back();
    return options;
}

vector<Placement> SimplePlayer2::place_armies(
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
        score = 5000
              - 10*(neutral_countries + opponent_countries) 
              - opponent_armies
              + 5*world.map.continents[i].bonus;
        assert(score > 0);
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

vector<Movement> SimplePlayer2::attack_transfer(
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
