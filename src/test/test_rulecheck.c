#include <stdio.h>
#include <stdlib.h>
#include "chess.h"
#include "rules.h"
int main(void) {
    struct GameState gs;
    printf("Testing rule checker...\n");
    init_game(&gs);

    /* Test a basic legal move: white pawn from F2 to F4 */
    struct Move m;
    m.from.f = 5;  /* F = index 5 */
    m.from.r = 1;  /* rank 2 = index 1 */
    m.to.f   = 5;  /* F */
    m.to.r   = 3;  /* rank 4 = index 3 */

    if (is_legal_move(&gs, m)) {
        printf("Test 1 PASSED: F2 to F4 is legal.\n");
    } else {
        printf("Test 1 FAILED: F2 to F4 should be legal.\n");
    }

    /* Test an illegal move: white pawn from F2 to F6 */
    struct Move m2;
    m2.from.f = 5;
    m2.from.r = 1;
    m2.to.f   = 5;
    m2.to.r   = 5;  /* rank 6 = index 5 */

    if (!is_legal_move(&gs, m2)) {
        printf("Test 2 PASSED: F2 to F6 is illegal.\n");
    } else {
        printf("Test 2 FAILED: F2 to F6 should be illegal.\n");
    }

    printf("Rule check tests complete!\n");
    return 0;
}
