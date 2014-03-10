#ifndef ARBITER_H_INCLUDED
#define ARBITER_H_INCLUDED

#include "Player.h"
#include <vector>
#include <ostream>

/* IMPORTANT: this arbiter does NOT attempt to validate any moves! */

class Arbiter
{
public:
    Arbiter(const World &w, Player &p1, Player &p2, std::ostream *log = 0);
    ~Arbiter();

    int play_game(int max_turns);
    void play_turn();
    int winner() const;

protected:
    std::vector<Movement> get_attack_transfer_moves(int player);
    void play_placement_phase();
    void play_movement_phase();
    void place_armies( int player, int armies,
                       const std::vector<Placement> &placements );
    void attack_transfer( int player, const Movement &movement,
                          std::vector<int> &reinforcements );
    void change_owner(Country &country, int new_owner);

    void log_map(std::vector<int> *reinforcements = 0);

protected:
    World world;
    Player &player1, &player2;
    std::ostream *log;

    int bonus[2];                   // #bonus armies for each player
    int total_countries[2];         // #countries occupied by each player
    int (*continent_countries)[2];  // #countries occupied per continent
};

#endif /* ndef ARBITER_H_INCLUDED */
