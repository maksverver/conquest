#ifndef WORLD_H_INCLUDED
#define WORLD_H_INCLUDED

#include <vector>

struct Continent
{
    int              id, bonus;
    std::vector<int> countries;
};

struct Country
{
    int              id, continent;
    std::vector<int> neighbours;
};

struct Map
{
    static Map getDefault();

    std::vector<Continent>  continents;
    std::vector<Country>    countries;

    int continent_index(int continent_id) const;
    int country_index(int country_id) const;
};

struct Occupation
{
    int owner, armies;
};

struct World
{
    World(const Map &map);
    World(const World &world);
    World(const World &world, int player);

    const Map &map;
    std::vector<Occupation> occupations;
};

struct Placement
{
    int dst, armies;
};

struct Movement
{
    int src, dst, armies;
};

#endif /* ndef WORLD_H_INCLUDED */
