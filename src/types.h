#ifndef TYPES_H
#define TYPES_H

#define NUM_PLAYERS 4
#define NUM_PIECES 4
#define BOARD_SIZE 52
#define HOME_STRAIGHT_LENGTH 5  // Number of cells in the home straight
#define TELEPORT_EFFECT_ROUNDS 4  // Number of rounds the effect lasts
#define MAX_BLOCK_SIZE 4

// Global variables
extern int piece_positions[NUM_PLAYERS][NUM_PIECES];
extern int piece_in_home_stretch[NUM_PLAYERS][NUM_PIECES];
extern int pieces_on_board[NUM_PLAYERS];
extern int pieces_in_base[NUM_PLAYERS];
extern int block_owner[BOARD_SIZE];
extern int block_size[BOARD_SIZE];
extern int piece_capture_count[NUM_PLAYERS][NUM_PIECES];
extern int energized[NUM_PLAYERS][NUM_PIECES];
extern int sick[NUM_PLAYERS][NUM_PIECES];
extern int briefing[NUM_PLAYERS][NUM_PIECES];
extern int player_directions[NUM_PLAYERS][NUM_PIECES];
extern int passed_end_counter[NUM_PLAYERS][NUM_PIECES];
extern int bhawana_teleported_effect[NUM_PLAYERS];
extern int kotuwa_teleported_effect[NUM_PLAYERS];

extern const char* player_colors[NUM_PLAYERS];
extern const char* piece_names[NUM_PLAYERS][NUM_PIECES];
extern const int starting_positions[NUM_PLAYERS];
extern const int ending_positions[NUM_PLAYERS];
extern const int bhawana;
extern const int kotuwa;
extern const int pita_kotuwa;

// Function prototypes
void initialize_game();
void play_game();
int roll_dice();
int coin_toss();
void move_piece(int player, int piece, int roll);
int find_piece_to_move(int player, int roll);
void print_piece_locations();
int check_winner();
int starting_player(int rolls[NUM_PLAYERS]);
void capture_piece(int player, int new_position, int piece);
int spawn_mystery_box(int round);
int teleported_to_bhawana(int player, int piece, int roll);
int teleported_to_kotuwa(int player, int piece, int roll);
int teleported_to_pitakotuwa(int player, int piece, int direction);
void handle_block_logic(int player, int piece, int position);
void print_round_status(int round);
int break_block_if_needed(int player, int piece);
void apply_bhawana_effect(int player, int piece);
void apply_kotuwa_effect(int player, int piece);
void apply_pita_kotuwa_effect(int player, int piece);

#endif

