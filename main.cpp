#include "Arbiter.h"
#include "MainWindow.h"
#include "GuiPlayer.h"
#include "SimplePlayer.h"
#include "SimplePlayer2.h"
#include "SimplePlayer3.h"
#include <QApplication>
#include <assert.h>
#include <iostream>
#include <memory>
#include <time.h>

static const Map the_map = Map::getDefault();
static const int max_turns = 100;

static Player *create_player(QString name)
{
    if (name == "gui")     return new GuiPlayer();
    if (name == "simple1") return new SimplePlayer();
    if (name == "simple2") return new SimplePlayer2();
    if (name == "simple3") return new SimplePlayer3();
    return NULL;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (argc == 2 && QString(argv[1]) == "visualize")
    {
        // Analysis mode -- read visualization data from stdin.
        QApplication app(argc, argv);
        MainWindow main_window;
        main_window.show();
        return app.exec();
    }

    if (argc == 4 && QString(argv[1]) == "play")
    {
        // Play one game, and print log to stdout:
        std::auto_ptr<Player> player1(create_player(argv[2]));
        if (!player1.get())
        {
            std::cerr << "Couldn't create player 1!\n";
            return 1;
        }
        std::auto_ptr<Player> player2(create_player(argv[3]));
        if (!player2.get())
        {
            std::cerr << "Couldn't create player 2!\n";
            return 1;
        }
        Arbiter arbiter(the_map, *player1, *player2, &std::cout);
        int winner = arbiter.play_game(max_turns);
        std::cout << ( winner > 0 ? "player1" :
                       winner < 0 ? "player2" : "Nobody" ) << " won" << '\n';
        return 0;
    }

    if (argc == 5 && QString(argv[1]) == "benchmark")
    {
        int num_games = QString(argv[4]).toInt();
        int player1_wins = 0, player2_wins = 0;
        for (int n = 0; n < num_games; ++n)
        {
            std::auto_ptr<Player> player1(create_player(argv[2]));
            if (!player1.get())
            {
                std::cerr << "Couldn't create player 1!\n";
                return 1;
            }
            std::auto_ptr<Player> player2(create_player(argv[3]));
            if (!player2.get())
            {
                std::cerr << "Couldn't create player 2!\n";
                return 1;
            }
            Arbiter arbiter(the_map, *player1, *player2);
            int winner = arbiter.play_game(max_turns);
            if (winner > 0)
            {
                std::cerr << '1';
                ++player1_wins;
            }
            else
            if (winner < 0)
            {
                std::cerr << '2';
                ++player2_wins;
            }
            else
            {
                std::cerr << '0';
            }
            if ((n + 1)%100 == 0) std::cerr << '\n';
        }
        if (num_games%100 != 0) std::cerr << '\n';
        std::cout << player1_wins << " vs " << player2_wins << std::endl;
        return 0;
    }

    std::cout << "Usage:\n"
        << "\tconquest visualize\n"
        << "\tconquest play <player1> <player2>\n"
        << "\tconquest benchmark <player1> <player2> <num_games>\n";
}
