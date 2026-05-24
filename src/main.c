#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"


int main() {
    srand(time(NULL));  // Seed the random number generator
    initialize_game();
    play_game();
    return 0;
}

