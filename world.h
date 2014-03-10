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
    int              owner, armies;
    std::vector<int> neighbours;
};

struct World
{
    std::vector<Continent>  continents;
    std::vector<Country>    countries;

    int continent_index(int continent_id) const;
    int country_index(int country_id) const;
    static World getDefault();
    void applyFog(int player);
    void dualize();
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
