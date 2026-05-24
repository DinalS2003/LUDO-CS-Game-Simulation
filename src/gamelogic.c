#include "types.h"
#include <stdio.h>
#include <stdlib.h>

// Initialize global variables
int piece_positions[NUM_PLAYERS][NUM_PIECES];
int piece_in_home_stretch[NUM_PLAYERS][NUM_PIECES];
int pieces_on_board[NUM_PLAYERS];
int pieces_in_base[NUM_PLAYERS];
int block_owner[BOARD_SIZE];
int block_size[BOARD_SIZE];
int piece_capture_count[NUM_PLAYERS][NUM_PIECES];
int energized[NUM_PLAYERS][NUM_PIECES];
int sick[NUM_PLAYERS][NUM_PIECES];
int briefing[NUM_PLAYERS][NUM_PIECES];
int player_directions[NUM_PLAYERS][NUM_PIECES];
int passed_end_counter[NUM_PLAYERS][NUM_PIECES];
int bhawana_teleported_effect[NUM_PLAYERS] = {0};
int kotuwa_teleported_effect[NUM_PLAYERS] = {0};
int current_mystery_box_cell = -1;  // -1 means no mystery box is active initially
int mystery_round_counter = 0;


const char* player_colors[NUM_PLAYERS] = {"Yellow", "Blue", "Red", "Green"};
const char* piece_names[NUM_PLAYERS][NUM_PIECES] = {
    {"Y1", "Y2", "Y3", "Y4"},
    {"B1", "B2", "B3", "B4"},
    {"R1", "R2", "R3", "R4"},
    {"G1", "G2", "G3", "G4"}
};

const int starting_positions[NUM_PLAYERS] = {2, 15, 28, 41};
const int ending_positions[NUM_PLAYERS] = {0, 13, 26, 39};
const int bhawana = 9;
const int kotuwa = 27;
const int pita_kotuwa = 46;

void initialize_game() {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        pieces_on_board[i] = 0;
        pieces_in_base[i] = NUM_PIECES;
        for (int j = 0; j < NUM_PIECES; j++) {
            piece_positions[i][j] = -1;  // All pieces start at base
            piece_in_home_stretch[i][j] = 0; // Piece not in home stretch
            player_directions[i][j] = 1; // Default direction is clockwise
            passed_end_counter[i][j] = 0;  // pass count initially 0
            piece_capture_count[i][j] = 0; // No captures initially
            energized[i][j] = 0;
            sick[i][j] = 0;
            briefing[i][j] = 0;
        }
    }
    for (int k = 0; k < BOARD_SIZE; k++) {
        block_size[k] = 0; // No blocks initially
        block_owner[k] = -1; // No owner initially
    }
}
void play_game() {
    int rolls[NUM_PLAYERS];
    int roll_6_count = 0;
    int round = 1;

    // Each player rolls once to determine the starting player
    for (int i = 0; i < NUM_PLAYERS; i++) {
        rolls[i] = roll_dice();
        printf("%s player rolls %d.\n", player_colors[i], rolls[i]);
    }

    int current_player = starting_player(rolls);
    printf("%s player has the highest roll and will begin the game.\n", player_colors[current_player]);

    // Game loop
    while (1) {
        if (current_player == 0) {
            // Print the round start message only at the beginning of each round
            printf("\n=== Round %d Begins ===\n", round);

            // Handle the mystery box logic once per round
            if (mystery_round_counter > 0) {
                mystery_round_counter--;
                if (mystery_round_counter == 0) {
                    printf("The mystery cell at location L%d has disappeared.\n", current_mystery_box_cell);
                    current_mystery_box_cell = -1;
                }
            }

            if ((round - 2) % 4 == 0) {
                current_mystery_box_cell = spawn_mystery_box(round);
                mystery_round_counter = 4;
            }
            if (current_mystery_box_cell >= 0) {
                printf("The mystery cell is at location L%d and will be in that location for the next %d rounds.\n", 
                       current_mystery_box_cell, mystery_round_counter);
            }
        }

        printf("%s player's turn.\n", player_colors[current_player]);

        int roll = roll_dice();
        printf("%s player rolled %d.\n", player_colors[current_player], roll);

        if (roll == 6) {
            roll_6_count++;

            if (roll_6_count == 3) {
                printf("%s player rolled three consecutive sixes and skips this turn.\n", player_colors[current_player]);
                roll_6_count = 0;
                current_player = (current_player + 1) % NUM_PLAYERS;  // Move to the next player
                continue;
            } else {
                printf("%s player rolled a 6 and gets another turn.\n", player_colors[current_player]);
            }
        } else {
            roll_6_count = 0;
        }

        int piece_to_move = find_piece_to_move(current_player, roll);
        if (piece_to_move != -1) {
            if (break_block_if_needed(current_player, piece_to_move)) {
                continue;
            }
            move_piece(current_player, piece_to_move, roll);
        } else {
            printf("No valid moves for %s player.\n", player_colors[current_player]);
        }

        if (roll != 6) {
            current_player = (current_player + 1) % NUM_PLAYERS;  // Move to the next player
        }

        if (current_player == 0) {
            print_round_status(round);
            if (check_winner()) {
                break;
            }
            round++;
        }
    }
}

            
int roll_dice() {
    return (rand() % 6) + 1;
}


int coin_toss() {
    return rand() % 2;  // 0 for heads (clockwise), 1 for tails (counterclockwise)
}

int find_piece_to_move(int player, int roll) {
    for (int i = 0; i < NUM_PIECES; i++) {
        if (piece_positions[player][i] == -1 && roll == 6 && !piece_in_home_stretch[player][i]) {
            return i;  // Move piece from base to position 1 when a 6 is rolled
        }

        if (piece_positions[player][i] >= 0 && !piece_in_home_stretch[player][i]) {
            return i;  // Move pieces that are already on the board but not in the homestretch
        }

        if (piece_in_home_stretch[player][i] && piece_positions[player][i] != HOME_STRAIGHT_LENGTH) {
            return i;  // Move pieces in the homestretch if roll fits
        }
    }
    return -1;  // No valid piece to move
}

void move_piece(int player, int piece, int roll) {
    int current_position = piece_positions[player][piece];
    int new_position;
    int direction = player_directions[player][piece];  // Use direction for the player

    // Handle Bhawana, Kotuwa, and Pita Kotuwa effects
    roll = teleported_to_bhawana(player, piece, roll);
    roll = teleported_to_kotuwa(player, piece, roll);
    direction = teleported_to_pitakotuwa(player, piece, direction);

    if (current_position == -1 && roll == 6) {
        // Moving from base to board
        new_position = starting_positions[player];
        player_directions[player][piece] = coin_toss();
        printf("%s player moves piece %s to the starting point.\n", player_colors[player], piece_names[player][piece]);
        pieces_on_board[player]++;
        pieces_in_base[player]--;
    } else if (piece_in_home_stretch[player][piece]) {
        // Moving within the home stretch
        new_position = current_position + roll;
        if (new_position > HOME_STRAIGHT_LENGTH) {
            new_position = current_position;
            printf("%s's piece %s needs an exact roll to enter home.\n", player_colors[player], piece_names[player][piece]);
            return;
        } else if (new_position == HOME_STRAIGHT_LENGTH) {
            // Piece reaches home
            printf("%s's piece %s has reached home!\n", player_colors[player], piece_names[player][piece]);
            pieces_on_board[player]--;
        }
    } else {
        // Moving on the standard path
        if (direction == 1) {
            new_position = (current_position + roll) % BOARD_SIZE;
            if (new_position >= ending_positions[player] && current_position <= ending_positions[player]) {
                passed_end_counter[player][piece]++;
                if (new_position > ending_positions[player]) {
                    piece_in_home_stretch[player][piece] = 1;
                    new_position = (current_position + roll) % BOARD_SIZE - (ending_positions[player] + 1);
                    if (new_position == HOME_STRAIGHT_LENGTH) {
                        printf("%s's piece %s has reached home!\n", player_colors[player], piece_names[player][piece]);
                        pieces_on_board[player]--;
                    }
                }
            }
        } else {
            new_position = (current_position - roll + BOARD_SIZE) % BOARD_SIZE;
            if (new_position <= ending_positions[player] && current_position > ending_positions[player]) {
                passed_end_counter[player][piece]++;
            }

            if (passed_end_counter[player][piece] == 2) {
                if (current_position - roll < ending_positions[player]) {
                    piece_in_home_stretch[player][piece] = 1;
                    new_position = (ending_positions[player] - 1) - (current_position - roll);
                    if (new_position == HOME_STRAIGHT_LENGTH) {
                        printf("%s's piece %s has reached home!\n", player_colors[player], piece_names[player][piece]);
                        pieces_on_board[player]--;
                    }
                }
            }
        }

        printf("%s's piece %s moves from L%d to L%d by %d units in %s direction.\n", 
               player_colors[player], piece_names[player][piece], 
               current_position, new_position, roll, 
               direction == 1 ? "clockwise" : "counterclockwise");
    }

    // Check if the piece lands on the current mystery box cell
    if (new_position == current_mystery_box_cell && new_position >= 0 && !piece_in_home_stretch[player][piece]) {
        printf("%s's piece %s lands on the mystery cell and is teleported.\n", player_colors[player], piece_names[player][piece]);
        int random_position = rand() % 6;  // Generate a random index from 0 to 5

        switch (random_position) {
            case 0:
                // Teleport to base
                piece_positions[player][piece] = -1;
                pieces_on_board[player]--;
                pieces_in_base[player]++;
                printf("%s's piece %s teleported to Base.\n", player_colors[player], piece_names[player][piece]);
                return;

            case 1:
                // Teleport to approach
                new_position = ending_positions[player];
                printf("%s's piece %s teleported to Approach.\n", player_colors[player], piece_names[player][piece]);
                break;

            case 2:
                // Teleport to X
                new_position = starting_positions[player];
                printf("%s's piece %s teleported to X.\n", player_colors[player], piece_names[player][piece]);
                break;

            case 3:
                // Teleport to Bhawana
                new_position = bhawana;
                bhawana_teleported_effect[player] = TELEPORT_EFFECT_ROUNDS;
                printf("%s's piece %s teleported to Bhawana.\n", player_colors[player], piece_names[player][piece]);
                break;

            case 4:
                // Teleport to Kotuwa
                new_position = kotuwa;
                kotuwa_teleported_effect[player] = TELEPORT_EFFECT_ROUNDS;
                printf("%s's piece %s teleported to Kotuwa.\n", player_colors[player], piece_names[player][piece]);
                break;

            case 5:
                // Teleport to Pita-Kotuwa
                new_position = pita_kotuwa;
                direction = teleported_to_pitakotuwa(player, piece, direction);
                printf("%s's piece %s teleported to Pita-Kotuwa.\n", player_colors[player], piece_names[player][piece]);
                break;
        }
    }

    handle_block_logic(player, piece, new_position);
    capture_piece(player, new_position, piece);
    piece_positions[player][piece] = new_position;

    printf("%s's piece %s moved to position L%d.\n", player_colors[player], piece_names[player][piece], new_position);
}



void capture_piece(int player, int new_position, int piece) {
    for (int opponent = 0; opponent < NUM_PLAYERS; opponent++) {
        if (opponent == player) continue;  // Skip the current player

        for (int i = 0; i < NUM_PIECES; i++) {
            if (piece_positions[opponent][i] == new_position && !piece_in_home_stretch[opponent][i] && !piece_in_home_stretch[player][i]) {
                printf("%s's piece %s lands on square L%d, captures %s's piece %s, and returns it to the base.\n", 
                       player_colors[player], piece_names[player][piece], 
                       new_position, player_colors[opponent], piece_names[opponent][i]);
                piece_positions[opponent][i] = -1;  // Send the captured piece back to the base
                piece_in_home_stretch[opponent][i] = 0; // Reset homestretch status
                passed_end_counter[opponent][i] = 0; // Reset counter
                player_directions[opponent][i] = -1; // Reset player direction
                pieces_on_board[opponent]--;
                pieces_in_base[opponent]++;
            }
        }
    }
}


void print_piece_locations() {
    for (int player = 0; player < NUM_PLAYERS; player++) {
        printf("============================\n");
        printf("Location of pieces [%s]\n", player_colors[player]);
        printf("============================\n");
        for (int piece = 0; piece < NUM_PIECES; piece++) {
            int position = piece_positions[player][piece];
            if (position == -1 && !piece_in_home_stretch[player][piece]) {
                printf("Piece %s -> Base\n", piece_names[player][piece]);
            } else if (position == HOME_STRAIGHT_LENGTH && piece_in_home_stretch[player][piece]) {
                printf("Piece %s -> Home\n", piece_names[player][piece]);
            } else if (piece_in_home_stretch[player][piece]) {
                printf("Piece %s -> HS%d\n", piece_names[player][piece], position);
            } else {
                printf("Piece %s -> L%d\n", piece_names[player][piece], position);
            }
        }
    }
}

int check_winner() {
    for (int player = 0; player < NUM_PLAYERS; player++) {
        int home_count = 0;
        for (int piece = 0; piece < NUM_PIECES; piece++) {
            if (piece_positions[player][piece] == HOME_STRAIGHT_LENGTH && piece_in_home_stretch[player][piece]) {
                home_count++;
            }
        }
        if (home_count == NUM_PIECES) {
            printf("%s player wins!!!\n", player_colors[player]);
            return 1;
        }
    }
    return 0;
}

int starting_player(int rolls[NUM_PLAYERS]) {
    int max_roll = rolls[0];
    int max_player = 0;
    for (int i = 1; i < NUM_PLAYERS; i++) {
        if (rolls[i] > max_roll) {
            max_roll = rolls[i];
            max_player = i;
        }
    }
    return max_player;
}

int spawn_mystery_box(int round) {
    int position;
    do {
        position = rand() % BOARD_SIZE;
    } while (position == bhawana || position == kotuwa || position == pita_kotuwa); // Ensure no special cell overlap
    return position;
}

int teleported_to_bhawana(int player, int piece, int roll) {
    if (bhawana_teleported_effect[player] > 0) {
        // Randomly decide whether to halve or double the roll
        int modify_roll = rand() % 2;
        if (modify_roll == 0) {
            roll = roll / 2;
            printf("%s's piece %s feels sick and movement speed halves.\n", player_colors[player], piece_names[player][piece]);
        } else {
            roll = roll * 2;
            printf("%s's piece %s feels energized and movement speed doubles.\n", player_colors[player], piece_names[player][piece]);
        }
        bhawana_teleported_effect[player]--;  // Decrease the effect counter after applying the modification
    }
    return roll;
}

int teleported_to_kotuwa(int player, int piece, int roll) {
    if (kotuwa_teleported_effect[player] > 0) {
        roll = 0;
        kotuwa_teleported_effect[player]--;  // Decrease the effect counter after applying the modification
        printf("%s's piece %s attends briefing and cannot move for %d rounds.\n", player_colors[player], piece_names[player][piece], kotuwa_teleported_effect[player] + 1);
    }
    return roll;
}

int teleported_to_pitakotuwa(int player, int piece, int direction) {
    if (player_directions[player][piece] == 1) {
        direction = -1;
        printf("%s's piece %s's direction is reversed to counterclockwise.\n", player_colors[player], piece_names[player][piece]);
    } else {
        direction = 1;
        printf("%s's piece %s's direction is reversed to clockwise.\n", player_colors[player], piece_names[player][piece]);
    }
    return direction;
}

void handle_block_logic(int player, int piece, int position) {
    int pieces_at_position = 0;

    // Count how many pieces the player has at the specified position
    for (int i = 0; i < NUM_PIECES; i++) {
        if (piece_positions[player][i] == position) {
            pieces_at_position++;
        }
    }

    // Update block size and ownership
    block_size[position] = pieces_at_position;

    // If more than one piece is at the same position, it forms a block
    if (block_size[position] > 1) {
        block_owner[position] = player;
        printf("%s's piece %s forms a block at position L%d with %d pieces.\n", 
               player_colors[player], piece_names[player][piece], position, block_size[position]);
    } else if (block_size[position] == 1) {
        // If only one piece remains, it is no longer a block
        block_owner[position] = player;
        printf("%s's piece %s moved to position L%d.\n", player_colors[player], piece_names[player][piece], position);
    } else {
        // No pieces left, so reset block owner and size
        block_owner[position] = -1;
        block_size[position] = 0;
    }
}

int break_block_if_needed(int player, int piece) {
    int position = piece_positions[player][piece];
    if (block_size[position] > 1) {
        printf("%s's piece %s breaks the block at position L%d.\n", player_colors[player], piece_names[player][piece], position);
        block_size[position]--;
        return 1;
    }
    return 0;
}

void apply_bhawana_effect(int player, int piece) {
    if (rand() % 2 == 0) {
        printf("%s's piece %s is energized!\n", player_colors[player], piece_names[player][piece]);
        energized[player][piece] = TELEPORT_EFFECT_ROUNDS;
    } else {
        printf("%s's piece %s is sick!\n", player_colors[player], piece_names[player][piece]);
        sick[player][piece] = TELEPORT_EFFECT_ROUNDS;
    }
}

void apply_kotuwa_effect(int player, int piece) {
    printf("%s's piece %s is attending a briefing and cannot move for %d rounds.\n", player_colors[player], piece_names[player][piece], TELEPORT_EFFECT_ROUNDS);
    briefing[player][piece] = TELEPORT_EFFECT_ROUNDS;
}

void apply_pita_kotuwa_effect(int player, int piece) {
    if (player_directions[player][piece] == 1) {
        player_directions[player][piece] = -1;
        printf("%s's piece %s has changed direction to counterclockwise.\n", player_colors[player], piece_names[player][piece]);
    } else {
        printf("%s's piece %s is teleported to Kotuwa.\n", player_colors[player], piece_names[player][piece]);
        piece_positions[player][piece] = kotuwa;
        apply_kotuwa_effect(player, piece);
    }
}

void print_round_status(int round) {
    printf("\n=== End of Round %d ===\n", round);
    for (int player = 0; player < NUM_PLAYERS; player++) {
        printf("%s player now has %d/4 pieces on the board and %d/4 pieces on the base.\n", 
               player_colors[player], pieces_on_board[player], pieces_in_base[player]);
    }
    print_piece_locations();
    printf("============================\n\n");
}

