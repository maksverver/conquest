#include "world.h"
#include <cstdlib>
#include <algorithm>

template<class T>
void insert_sorted(std::vector<T> &v, const T &x)
{
    v.insert(std::lower_bound(v.begin(), v.end(), x), x);
}

World World::getDefault()
{
    static const int num_nodes  = 42;
    static const int num_groups =  6;
    static const int num_edges  = 82;

    static const int bonus[num_groups] = {
        5, 2, 5, 3, 7, 2
    };

    static const int groups[num_nodes] = {  /* NOTE: indices are 1-based! */
        1, 1, 1, 1, 1, 1, 1, 1, 1,              // North America (5 bonus)
        2, 2, 2, 2,                             // South America (2 bonus)
        3, 3, 3, 3, 3, 3, 3,                    // Europe (5 bonus)
        4, 4, 4, 4, 4, 4,                       // Africa (3 bonus)
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     // Asia (7 bonus)
        6, 6, 6, 6                              // Australia (2 bonus)
    };

    static const int edges[num_edges][2] = {  /* NOTE: indices are 1-based! */
        {  1, 2 }, {  1, 4 }, {  1,30 }, {  2, 3 }, {  2, 4 }, {  2, 5 },
        {  3, 5 }, {  3, 6 }, {  3,14 }, {  4, 5 }, {  4, 7 }, {  5, 6 },
        {  5, 7 }, {  5, 8 }, {  6, 8 }, {  7, 8 }, {  7, 9 }, {  8, 9 },
        {  9,10 }, { 10,11 }, { 10,12 }, { 11,12 }, { 11,13 }, { 12,13 },
        { 12,21 }, { 14,15 }, { 14,16 }, { 15,16 }, { 15,18 }, { 15,19 },
        { 16,17 }, { 17,19 }, { 17,20 }, { 17,27 }, { 17,32 }, { 17,36 },
        { 18,19 }, { 18,20 }, { 18,21 }, { 19,20 }, { 20,21 }, { 20,22 },
        { 20,36 }, { 21,22 }, { 21,23 }, { 21,24 }, { 22,23 }, { 22,36 },
        { 23,24 }, { 23,25 }, { 23,26 }, { 23,36 }, { 24,25 }, { 25,26 },
        { 27,28 }, { 27,32 }, { 27,33 }, { 28,29 }, { 28,31 }, { 28,33 },
        { 28,34 }, { 29,30 }, { 29,31 }, { 30,31 }, { 30,34 }, { 30,35 },
        { 31,34 }, { 32,33 }, { 32,36 }, { 32,37 }, { 33,34 }, { 33,37 },
        { 33,38 }, { 34,35 }, { 36,37 }, { 37,38 }, { 38,39 }, { 39,40 },
        { 39,41 }, { 40,41 }, { 40,42 }, { 41,42 } };


    World world;
    world.continents.reserve(num_groups);
    for (int n = 0; n < num_groups; ++n)
    {
        Continent ct = Continent();
        ct.id    = n + 1;
        ct.bonus = bonus[n];
        world.continents.push_back(ct);
    }
    world.countries.reserve(num_nodes);
    for (int n = 0; n < num_nodes; ++n)
    {
        Country cy = Country();
        cy.id        = n + 1;
        cy.continent = groups[n] - 1;
        cy.owner     = 0;
        cy.armies    = 2;
        world.countries.push_back(cy);
        world.continents[cy.continent].countries.push_back(n);
    }
    for (int n = 0; n < num_edges; ++n)
    {
        int i = edges[n][0] - 1;
        int j = edges[n][1] - 1;
        insert_sorted(world.countries[i].neighbours, j);
        insert_sorted(world.countries[j].neighbours, i);
    }
    return world;
}

int World::continent_index(int continent_id) const
{
    for (size_t i = 0; i < continents.size(); ++i)
    {
        if (continents[i].id == continent_id) return i;
    }
    return -1;
}

int World::country_index(int country_id) const
{
    for (size_t i = 0; i < countries.size(); ++i)
    {
        if (countries[i].id == country_id) return i;
    }
    return -1;
}

void World::applyFog(int player)
{
    for (size_t i = 0; i < countries.size(); ++i)
    {
        if (countries[i].armies > 0)
        {
            bool visible = countries[i].owner == player;
            if (!visible)
            {
                for (size_t n = 0; n < countries[i].neighbours.size(); ++n)
                {
                    if (countries[countries[i].neighbours[n]].owner == player)
                    {
                        visible = true;
                        break;
                    }
                }
            }
            if (!visible)
            {
                countries[i].owner = 0;
                countries[i].armies = 0;
            }
        }
    }
}

void World::dualize()
{
    for (size_t i = 0; i < countries.size(); ++i)
    {
        countries[i].owner = -countries[i].owner;
    }
}
