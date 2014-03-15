#include "SimplePlayer4.h"
#include <algorithm>
#include <utility>
#include <iostream>  //DEBUG
#include <assert.h>
using namespace std;

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

vector<int> SimplePlayer4::pick_starting_countries( const World UNUSED &world,
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

vector<Placement> SimplePlayer4::place_armies(
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

    std::vector<int> defenders_needed(world.map.countries.size(), 0);
    this->locked = vector<char>(world.map.countries.size(), 0);
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
        if (neutral_countries + opponent_countries == 0)
        {
            /* I control this continent.
               Check if I need to defend it against the opponent: */
            for (size_t j = 0; j < world.map.countries.size(); ++j)
            {
                if (world.occupations[j].owner < 0)
                {
                    for (int k : world.map.countries[j].neighbours)
                    {
                        if (world.map.countries[k].continent == (int)i)
                        {
                            // Country j threatens continent i at country k!
                            int his_armies  = world.occupations[j].armies;
                            int my_armies   = world.occupations[k].armies;
                            int need_armies = (his_armies + 5)*7/10 + 2;
                            if (need_armies > my_armies)
                            {
                                defenders_needed[k] += need_armies - my_armies;
                                locked[k] = true;
                            }
                        }
                    }
                }
            }
            continue;
        }

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

    // First, add defenders:
    vector<Placement> placements;
    {
        vector<pair<int,int> > armies_country;
        for (size_t i = 0; i < world.map.countries.size(); ++i)
        {
            if (defenders_needed[i] > 0)
            {
                armies_country.push_back(std::make_pair(defenders_needed[i], (int)i));
            }
        }
        sort(armies_country.begin(), armies_country.end());
        for (auto const &ac : armies_country)
        {
            int extra = std::min(num_armies, ac.first);
            if (extra > 0)
            {
                Placement pt = { ac.second, extra };
                placements.push_back(pt);
                num_armies -= extra;
            }
        }
    }

    if (num_armies > 0)
    {
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

        int i = max_element(options.begin(), options.end())->second;
        Placement pt = { i, num_armies };
        placements.push_back(pt);
    }
    return placements;
}

vector<Movement> SimplePlayer4::attack_transfer(
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

        if (locked[i]) continue;

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
