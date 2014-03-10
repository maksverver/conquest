#include "SimplePlayer.h"
#include <algorithm>
#include <utility>
#include <iostream>  // debug
using namespace std;

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#endif

vector<int> SimplePlayer::pick_starting_countries(
    const World UNUSED &world, vector<int> options,
    int num_countries, int UNUSED timeout_ms )
{
    // For now, just pick randomly:
    random_shuffle(options.begin(), options.end());
    while (options.size() > (size_t)num_countries) options.pop_back();
    return options;
}

vector<Placement> SimplePlayer::place_armies(
    const World &world, int num_armies, int UNUSED timeout_ms )
{
    // Put all reinforcements on the country with the most hostile armies
    // nearby (breaking ties by the most neutral armies).
    vector<pair<pair<int,int>, int> > options;
    for (size_t i = 0; i < world.countries.size(); ++i)
    {
        if (world.countries[i].owner > 0)
        {
            pair<int,int> score;
            const vector<int> &neighbours = world.countries[i].neighbours;
            for ( vector<int>::const_iterator it = neighbours.begin();
                  it != neighbours.end(); ++it )
            {
                const Country &neighbour = world.countries[*it];
                if (neighbour.owner <  0) score.first += neighbour.armies;
                if (neighbour.owner == 0) score.second += neighbour.armies;
            }
            options.push_back(make_pair(score, (int)i));
        }
    }
    vector<Placement> placements;
    if (!options.empty())
    {
        int i = max_element(options.begin(), options.end())->second;
        Placement pl = { i, num_armies };
        placements.push_back(pl);
    }
    return placements;
}

vector<Movement> SimplePlayer::attack_transfer(
    const World &world, int UNUSED timeout_ms )
{
    // Aggressively attack!
    vector<Movement> movements;
    for (size_t i = 0; i < world.countries.size(); ++i)
    {
        const Country &cy = world.countries[i];
        if (cy.owner > 0 && cy.armies > 1)
        {
            vector<pair<int,int> > score;
            for (vector<int>::const_iterator it = cy.neighbours.begin(); it != cy.neighbours.end(); ++it)
            {
                const Country &neighbour = world.countries[*it];
                if (neighbour.owner <= 0)
                {
                    score.push_back(make_pair(neighbour.armies, *it));
                }
            }
            if (!score.empty())
            {
                sort(score.begin(), score.end());
                int armies_left = cy.armies - 1;
                size_t n = 0;  // number of attacks
                while (n < score.size())
                {
                    int needed = score[n].first*7/6 + 2;
                    if (armies_left < needed) break;
                    Movement move = { (int)i, score[n].second, needed };
                    movements.push_back(move);
                    armies_left -= needed;
                    ++n;

                }
                if (n > 0)
                {
                    // Add any remaining armies to the last attack:
                    movements.back().armies += armies_left;
                }
            }
        }
    }
    return movements;
}
