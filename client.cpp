/* Plays a game of Conquest using the AI Games protocol. */

#include "Player.h"
#include "SimplePlayer3.h"

#include <algorithm>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>

static void compiler_check()
{
    std::cerr << "Wordsize: " << __WORDSIZE << '\n';
#ifdef __x86_64__
    std::cerr << "x86-64 ISA\n";
#endif
#ifdef __i386__
    std::cerr << "i386 ISA\n";
#endif
    std::cerr << "GCC version " << __VERSION__ << '\n';
#ifdef __OPTIMIZE__
    std::cerr << "Optimizing.\n";
#else
    std::cerr << "NOT optimizing!\n";
#endif
#ifdef __NO_INLINE__
    std::cerr << "NOT inlining!\n";
#else
    std::cerr << "Inlining.\n";
#endif
}

int main()
{
    compiler_check();

    /* Seed RNG (just time -- lame.  I usually mix in getpid() but I don't
       don't know if that's available on the judge's system.) */
    srand(time(NULL));

    // Client state:
    Map map;
    World world(map);
    std::string your_bot_id     = "YOUR_BOT_ID";
    std::string opponent_bot_id = "OPPONENT_BOT_ID";
    int starting_armies         = 5;

    // Actual player instance:
    std::auto_ptr<Player> player(new SimplePlayer3());

    // Process input, line by line:
    std::string line;
    while (getline(std::cin, line))
    {
        // Parse line:
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (command == "settings")
        {
            std::string key;
            iss >> key;
            if (key == "your_bot")
            {
                iss >> your_bot_id;
            }
            else
            if (key == "opponent_bot")
            {
                iss >> opponent_bot_id;
            }
            else
            if (key == "starting_armies")
            {
                // Why do we even get this information?  We should be able
                // to derive this ourselves from the available info.
                iss >> starting_armies;
            }
            else
            {
                std::cerr << "Unknown info-command received!  Line was: '" << line << "'\n";
            }
        }
        else
        if (command == "setup_map")
        {
            std::string key;
            iss >> key;
            if (key == "super_regions")
            {
                map.continents.clear();
                Continent ct = Continent();
                while (iss >> ct.id >> ct.bonus) map.continents.push_back(ct);
            }
            else
            if (key == "regions")
            {
                map.countries.clear();
                int continent_id;
                Country cy = Country();
                while (iss >> cy.id >> continent_id)
                {
                    int ct = map.continent_index(continent_id);
                    if (cy.continent >= 0)
                    {
                        map.continents[ct].countries.push_back(map.countries.size());
                        cy.continent = ct;
                        map.countries.push_back(cy);
                    }
                    else
                    {
                        std::cerr << "Unknown continent-id: " << continent_id << "!\n";
                    }
                }
            }
            else
            if (key == "neighbors")
            {
                int country_id;
                std::string neighbours;
                for (Country &cy : map.countries)
                {
                    cy.neighbours.clear();
                }
                while (iss >> country_id >> neighbours)
                {
                    int i = map.country_index(country_id);
                    if (i >= 0)
                    {
                        std::replace(neighbours.begin(), neighbours.end(), ',', ' ');
                        std::istringstream neighbours_iss(neighbours);
                        int neighbour_id;
                        while (neighbours_iss >> neighbour_id)
                        {
                            int j = map.country_index(neighbour_id);
                            if (j >= 0)
                            {
                                map.countries[i].neighbours.push_back(j);
                                map.countries[j].neighbours.push_back(i);
                            }
                            else
                            {
                                std::cerr << "Unknown country-id: " << country_id << "!\n";
                            }
                        }
                    }
                    else
                    {
                        std::cerr << "Unknown country-id: " << country_id << "!\n";
                    }
                }
                for (Country &cy : map.countries)
                {
                    std::vector<int> &v = cy.neighbours;
                    std::sort(v.begin(), v.end());
                    v.erase(std::unique(v.begin(), v.end()), v.end());
                }
            }
            else
            {
                std::cerr << "Unknown setup_map-command received!  Line was: '" << line << "'\n";
            }
        }
        else
        if (command == "pick_starting_regions")
        {
            int timeout = 0;
            iss >> timeout;
            std::vector<int> options;
            int country_id;
            while (iss >> country_id)
            {
                int i = map.country_index(country_id);
                if (i >= 0) options.push_back(i);
                else std::cerr << "Unknown country-id: " << country_id << "!\n";
            }
            const int num_countries = 6;  // fixed, for now!
            std::vector<int> selection = player->pick_starting_countries(
                world, options, num_countries, timeout );
            for (size_t i = 0; i < selection.size(); ++i)
            {
                if (i > 0) std::cout << ' ';
                std::cout << map.countries[selection[i]].id;
            }
            std::cout << std::endl;
        }
        else
        if (command == "update_map")
        {
            Occupation initial_occupation = { 0, 0 };
            world.occupations.assign(map.countries.size(), initial_occupation);

            int country_id;
            std::string name;
            int armies;
            while (iss >> country_id >> name >> armies)
            {
                int i = map.country_index(country_id);
                if (i >= 0)
                {
                    int owner = (name == your_bot_id) - (name == opponent_bot_id);
                    if (owner != 0 || name == "neutral")
                    {
                        if (armies > 0 && armies < 1000000000)
                        {
                            world.occupations[i].owner  = owner;
                            world.occupations[i].armies = armies;
                        }
                        else
                        {
                            std::cerr << "Invalid number of armies: " << armies << "!\n";
                        }
                    }
                    else
                    {
                        std::cerr << "Unknown player-name: " << name << "!\n";
                    }
                }
                else
                {
                    std::cerr << "Unknown country-id: " << country_id << "!\n";
                }
            }
        }
        else
        if (command == "opponent_moves")
        {
            // not yet implemented -- who even cares about this information?
        }
        else
        if (command == "go")
        {
            int timeout = 0;
            std::string phase;
            iss >> phase >> timeout;
            if (phase == "place_armies")
            {
                std::vector<Placement> placements =
                    player->place_armies(world, starting_armies, timeout);
                if (placements.empty())
                {
                    std::cout << "No moves" << std::endl;
                }
                else
                {
                    for (auto it = placements.begin(); it != placements.end(); ++it)
                    {
                        if (it != placements.begin()) std::cout << ',';
                        std::cout << your_bot_id << " place_armies "
                                  << map.countries[it->dst].id << ' '
                                  << it->armies;

                        // NOTE: state is also updated here, because we only
                        //       get map updates after attack/transfer phase:
                        world.occupations[it->dst].armies += it->armies;
                    }
                    std::cout << std::endl;
                }
            }
            else
            if (phase == "attack/transfer")
            {
                std::vector<Movement> movements =
                    player->attack_transfer(world, timeout);
                if (movements.empty())
                {
                    std::cout << "No moves" << std::endl;
                }
                else
                {
                    for (auto it = movements.begin(); it != movements.end(); ++it)
                    {
                        if (it != movements.begin()) std::cout << ',';
                        std::cout << your_bot_id << " attack/transfer "
                                  << map.countries[it->src].id << ' '
                                  << map.countries[it->dst].id << ' '
                                  << it->armies;
                    }
                    std::cout << std::endl;
                }
            }
            else
            {
                std::cout << "No moves" << std::endl;
                std::cerr << "Unknown go-command received!  Line was: '" << line << "'\n";
            }
        }
        else
        {
            std::cerr << "Unknown command received!  Line was: '" << line << "'\n";
        }
    }
}
