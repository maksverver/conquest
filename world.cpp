#include "world.h"
#include <assert.h>
#include <stdlib.h>
#include <algorithm>

template<class T>
void insert_sorted(std::vector<T> &v, const T &x)
{
    v.insert(std::lower_bound(v.begin(), v.end(), x), x);
}

Map Map::getDefault()
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

    Map map;
    map.continents.reserve(num_groups);
    for (int n = 0; n < num_groups; ++n)
    {
        Continent ct =  { n + 1, bonus[n], std::vector<int>() };
        map.continents.push_back(ct);
    }
    map.countries.reserve(num_nodes);
    for (int n = 0; n < num_nodes; ++n)
    {
        Country cy = { n + 1, groups[n] - 1, std::vector<int>() };
        map.countries.push_back(cy);
        map.continents[cy.continent].countries.push_back(n);
    }
    for (int n = 0; n < num_edges; ++n)
    {
        int i = edges[n][0] - 1;
        int j = edges[n][1] - 1;
        insert_sorted(map.countries[i].neighbours, j);
        insert_sorted(map.countries[j].neighbours, i);
    }
    return map;
}

int Map::continent_index(int continent_id) const
{
    for (size_t i = 0; i < continents.size(); ++i)
    {
        if (continents[i].id == continent_id) return i;
    }
    return -1;
}

int Map::country_index(int country_id) const
{
    for (size_t i = 0; i < countries.size(); ++i)
    {
        if (countries[i].id == country_id) return i;
    }
    return -1;
}

World::World(const Map &m)
    : map(m)
{
    Occupation initial_occupation = { 0 /* owner */, 2 /* armies */ };
    occupations.assign(m.countries.size(), initial_occupation);
}

World::World(const World &world)
    : map(world.map), occupations(world.occupations)
{
}

World::World(const World &world, int player)
    : map(world.map), occupations(world.occupations)
{
    assert(player == +1 || player == -1);

    // Apply fog-of-war
    for (size_t i = 0; i < occupations.size(); ++i)
    {
        bool visible = occupations[i].owner == player;
        if (!visible)
        {
            for (size_t n = 0; n < map.countries[i].neighbours.size(); ++n)
            {
                if (occupations[map.countries[i].neighbours[n]].owner == player)
                {
                    visible = true;
                    break;
                }
            }
        }
        if (!visible)
        {
            occupations[i].owner = 0;
            occupations[i].armies = 0;
        }
    }

    if (player < 0)
    {
        // Invert owner ids so that +1 refers to the current player:
        for (size_t i = 0; i < occupations.size(); ++i)
        {
            occupations[i].owner = -occupations[i].owner;
        }
    }
}
