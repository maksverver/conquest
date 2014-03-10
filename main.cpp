#include "Arbiter.h"
#include "MainWindow.h"
#include "GuiPlayer.h"
#include "SimplePlayer.h"
#include "SimplePlayer2.h"
#include <QApplication>
#include <iostream>
#include <assert.h>
#include <time.h>

static const Map the_map = Map::getDefault();

World random_starting_world()
{
    World world(the_map);

    // Pick list of starting options; two per continent
    std::vector<int> options;
    for (size_t i = 0; i < world.map.continents.size(); ++i)
    {
        std::vector<int> countries = world.map.continents[i].countries;
        assert(countries.size() >= 2);
        std::random_shuffle(countries.begin(), countries.end());
        options.push_back(countries[0]);
        options.push_back(countries[1]);
    }

    // Randomly assign to players:
    std::random_shuffle(options.begin(), options.end());
    assert(options.size() >= 6);
    for (int i = 0; i < 6; ++i)
    {
        world.occupations[options[i]].owner = 1 - 2*(i%2);
    }

    return world;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    if (argc == 2 && QString(argv[1]) == "-")
    {
        // Analysis mode -- read visualization data from stdin.
        QApplication app(argc, argv);
        MainWindow main_window;
        main_window.show();
        return app.exec();
    }
    else
    {
        if (true)
        {
            // Play one game, and print log to stdout:
            SimplePlayer  player1;
            SimplePlayer2 player2;
            World world = random_starting_world();
            Arbiter arbiter(world, player1, player2, &std::cout);
            int winner = arbiter.play_game(100);
            std::cout << ( winner > 0 ? "player1" :
                           winner < 0 ? "player2" : "Nobody" )
                       << " won" << std::endl;
        }
        else
        {
            int x = 0, y = 0;
            for (int n = 0; n < 1000; ++n)
            {
                SimplePlayer  player1;
                SimplePlayer2 player2;
                World world = random_starting_world();
                Arbiter arbiter(world, player1, player2);
                int winner = arbiter.play_game(100);
                if (winner > 0) { ++x; std::cerr << '1'; } else
                if (winner < 0) { ++y; std::cerr << '2'; } else std::cerr << '0';
                if ((n + 1) % 100 == 0) std::cerr << '\n';
            }
            std::cout << x << " vs " << y << std::endl;
        }
    }
}
