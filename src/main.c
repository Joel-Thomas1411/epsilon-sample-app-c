/* ================================================================== */
/*  PB Sim – Top-down cycling game for NumWorks (EADK C)              */
/*  Dodge traffic across 3 lanes. Don't crash!                        */
/* ================================================================== */ 

#include <eadk.h>
#include <stdbool.h>
#include <stdint.h>


/* ------------------------------------------------------------------ */
/*  App metadata (required by EADK)                                    */
/* ------------------------------------------------------------------ */
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "PB Sim";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

/* ------------------------------------------------------------------ */
/*  Screen constants                                                   */
/* ------------------------------------------------------------------ */
#define SCR_W  320
#define SCR_H  240

/* ------------------------------------------------------------------ */
/*  Road layout                                                        */
/* ------------------------------------------------------------------ */
#define ROAD_L    60
#define ROAD_R    260
#define ROAD_W    200
#define LANE_W    66        /* ~ROAD_W / 3 */
#define EDGE_W    3         /* yellow edge line width */

/* Lane center X for each lane (sprite left-edge) */
#define LANE_0_X  (ROAD_L + (LANE_W - CAR_W) / 2)   /* ~82 */
#define LANE_1_X  (ROAD_L + LANE_W + (LANE_W - CAR_W) / 2)  /* ~148 */
#define LANE_2_X  (ROAD_L + 2*LANE_W + (LANE_W - CAR_W) / 2) /* ~214 */

/* Player uses its own centering */
#define PLAYER_LANE_0  (ROAD_L + (LANE_W - PLAYER_W) / 2)
#define PLAYER_LANE_1  (ROAD_L + LANE_W + (LANE_W - PLAYER_W) / 2)
#define PLAYER_LANE_2  (ROAD_L + 2*LANE_W + (LANE_W - PLAYER_W) / 2)

#define PLAYER_Y  195      /* fixed Y near bottom */

/* Lane marking dimensions */
#define MARK_W    4
#define MARK_H    18
#define MARK_GAP  14

/* ------------------------------------------------------------------ */
/*  Traffic                                                            */
/* ------------------------------------------------------------------ */
#define MAX_CARS  8

typedef struct {
    int lane;           /* 0, 1, 2 */
    int y;              /* top of car */
    eadk_color_t color; /* body color */
    bool active;
} Car;

static const eadk_color_t car_colors[] = {
    C_CAR_BLUE, C_CAR_RED, C_CAR_YEL, C_CAR_GRN, C_CAR_WHT
};
#define NUM_CAR_COLORS 5

/* ------------------------------------------------------------------ */
/*  PRNG (xorshift32)                                                  */
/* ------------------------------------------------------------------ */
static uint32_t rng_state;

static void rng_seed(void) {
    rng_state = (uint32_t)(eadk_timing_millis() ^ 0xDEADBEEF);
    if (rng_state == 0) rng_state = 42;
}

static uint32_t rng_next(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static int rng_range(int lo, int hi) {
    return lo + (int)(rng_next() % (uint32_t)(hi - lo + 1));
}

/* ------------------------------------------------------------------ */
/*  Simple integer to string                                           */
/* ------------------------------------------------------------------ */
static void itoa_simple(int val, char *buf) {
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[12];
    int i = 0;
    int v = val < 0 ? -val : val;
    while (v > 0) { tmp[i++] = '0' + (v % 10); v /= 10; }
    if (val < 0) tmp[i++] = '-';
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}

/* ------------------------------------------------------------------ */
/*  Lane X lookup tables                                               */
/* ------------------------------------------------------------------ */
static const int player_lane_x[] = { PLAYER_LANE_0, PLAYER_LANE_1, PLAYER_LANE_2 };
static const int car_lane_x[]    = { LANE_0_X, LANE_1_X, LANE_2_X };

/* ------------------------------------------------------------------ */
/*  Draw the static road background                                    */
/* ------------------------------------------------------------------ */
static void draw_road_bg(void) {
    /* Grass */
    fill(0, 0, ROAD_L, SCR_H, C_GRASS);
    fill(ROAD_R, 0, SCR_W - ROAD_R, SCR_H, C_GRASS);

    /* Grass darker stripes */
    for (int gy = 0; gy < SCR_H; gy += 10) {
        fill(0, gy, ROAD_L, 4, C_GRASS_DK);
        fill(ROAD_R, gy, SCR_W - ROAD_R, 4, C_GRASS_DK);
    }

    /* Road surface */
    fill(ROAD_L, 0, ROAD_W, SCR_H, C_ROAD);

    /* Yellow edge lines */
    fill(ROAD_L, 0, EDGE_W, SCR_H, C_EDGE_YEL);
    fill(ROAD_R - EDGE_W, 0, EDGE_W, SCR_H, C_EDGE_YEL);
}

/* ------------------------------------------------------------------ */
/*  Lane markings with scroll offset                                   */
/* ------------------------------------------------------------------ */
static void draw_lane_marks(int offset, eadk_color_t color) {
    int x1 = ROAD_L + LANE_W - MARK_W / 2;
    int x2 = ROAD_L + 2 * LANE_W - MARK_W / 2;
    int period = MARK_H + MARK_GAP;
    int start_y = -(period) + (offset % period);

    for (int y = start_y; y < SCR_H; y += period) {
        int dy = y < 0 ? 0 : y;
        int dh = (y < 0) ? MARK_H + y : MARK_H;
        if (dy + dh > SCR_H) dh = SCR_H - dy;
        if (dh > 0) {
            fill(x1, dy, MARK_W, dh, color);
            fill(x2, dy, MARK_W, dh, color);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Collision detection                                                 */
/* ------------------------------------------------------------------ */
static bool check_collision(int px, int py, Car *cars) {
    for (int i = 0; i < MAX_CARS; i++) {
        if (!cars[i].active) continue;
        int cx = car_lane_x[cars[i].lane];
        int cy = cars[i].y;
        /* AABB overlap */
        if (px < cx + CAR_W && px + PLAYER_W > cx &&
            py < cy + CAR_H && py + PLAYER_H > cy) {
            return true;
        }
    }
    return false;
}

/* ------------------------------------------------------------------ */
/*  Draw supply & demand curve diagram                                 */
/* ------------------------------------------------------------------ */
static void draw_sd_diagram(int ox, int oy, int w, int h) {
    /* Axes */
    fill(ox + 14, oy, 2, h, C_BLACK);      /* Y axis (Price) */
    fill(ox + 14, oy + h - 2, w - 14, 2, C_BLACK); /* X axis (Quantity) */

    /* Axis labels */
    eadk_display_draw_string("P", (eadk_point_t){ox + 3, oy + 2}, false, C_BLACK, C_WHITE);
    eadk_display_draw_string("Q", (eadk_point_t){ox + w - 12, oy + h - 14}, false, C_BLACK, C_WHITE);

    /* Demand curve (top-left to bottom-right) – red */
    int steps = (w - 24) / 6;
    if (steps < 4) steps = 4;
    for (int i = 0; i <= steps; i++) {
        int px = ox + 18 + i * (w - 28) / steps;
        int py = oy + 6  + i * (h - 18) / steps;
        fill(px, py, 3, 3, C_RED);
    }

    /* Supply curve (bottom-left to top-right) – blue */
    for (int i = 0; i <= steps; i++) {
        int px = ox + 18 + i * (w - 28) / steps;
        int py = oy + h - 8 - i * (h - 18) / steps;
        fill(px, py, 3, 3, C_BLUE);
    }

    /* S and D labels */
    eadk_display_draw_string("D", (eadk_point_t){ox + w - 14, oy + h - 26}, false, C_RED, C_WHITE);
    eadk_display_draw_string("S", (eadk_point_t){ox + w - 14, oy + 4}, false, C_BLUE, C_WHITE);
}

/* ------------------------------------------------------------------ */
/*  Crash screen                                                       */
/* ------------------------------------------------------------------ */
static void show_crash_screen(int score) {
    /* White background */
    fill(0, 0, SCR_W, SCR_H, C_WHITE);

    /* Draw supply & demand diagrams */
    draw_sd_diagram(8, 6, 120, 90);
    draw_sd_diagram(192, 6, 120, 90);
    draw_sd_diagram(8, 140, 100, 75);
    draw_sd_diagram(220, 145, 92, 70);

    /* Draw the suited character standing in center */
    draw_crash_guy(148, 100);

    /* Score display */
    char score_str[16];
    itoa_simple(score, score_str);

    /* Dark banner for score */
    int sw = (int)strlen(score_str);
    int bw = sw * 10 + 20;
    int bx = (SCR_W - bw) / 2;
    fill(bx, 218, bw, 18, C_BLACK);
    eadk_display_draw_string(score_str,
        (eadk_point_t){(SCR_W - sw * 10) / 2, 220},
        true, C_WHITE, C_BLACK);

    /* Wait for key */
    eadk_timing_msleep(500);  /* debounce delay */
    while (true) {
        eadk_keyboard_state_t kbd = eadk_keyboard_scan();
        if (eadk_keyboard_key_down(kbd, eadk_key_ok) ||
            eadk_keyboard_key_down(kbd, eadk_key_back)) {
            eadk_timing_msleep(200);
            return;
        }
        eadk_timing_msleep(30);
    }
}

/* ------------------------------------------------------------------ */
/*  Collision flash effect                                             */
/* ------------------------------------------------------------------ */
static void collision_flash(void) {
    fill(0, 0, SCR_W, SCR_H, C_RED);
    eadk_timing_msleep(80);
    fill(0, 0, SCR_W, SCR_H, C_WHITE);
    eadk_timing_msleep(80);
    fill(0, 0, SCR_W, SCR_H, C_RED);
    eadk_timing_msleep(80);
}

/* ------------------------------------------------------------------ */
/*  Single game round                                                  */
/* ------------------------------------------------------------------ */
static void game_round(void) {
    rng_seed();

    /* --- State --- */
    int lane = 1;           /* start middle */
    int score = 0;
    int mark_offset = 0;
    int speed = 3;
    int spawn_timer = 0;
    int spawn_rate = 28;    /* frames between spawns */
    int frame = 0;
    bool prev_left = false;
    bool prev_right = false;

    Car cars[MAX_CARS];
    for (int i = 0; i < MAX_CARS; i++) cars[i].active = false;

    /* --- Draw initial road --- */
    draw_road_bg();
    draw_lane_marks(0, C_LANE_WHT);
    draw_player(player_lane_x[lane], PLAYER_Y);

    /* --- Main loop --- */
    while (true) {
        uint64_t frame_start = eadk_timing_millis();

        /* === Input === */
        eadk_keyboard_state_t kbd = eadk_keyboard_scan();
        bool cur_left  = eadk_keyboard_key_down(kbd, eadk_key_left);
        bool cur_right = eadk_keyboard_key_down(kbd, eadk_key_right);

        bool moved = false;
        if (cur_left && !prev_left && lane > 0) {
            erase_player(player_lane_x[lane], PLAYER_Y);
            lane--;
            moved = true;
        } else if (cur_right && !prev_right && lane < 2) {
            erase_player(player_lane_x[lane], PLAYER_Y);
            lane++;
            moved = true;
        }
        prev_left = cur_left;
        prev_right = cur_right;

        /* Exit to home */
        if (eadk_keyboard_key_down(kbd, eadk_key_home)) return;

        /* === Erase old lane marks === */
        draw_lane_marks(mark_offset, C_ROAD);

        /* === Update & draw cars === */
        for (int i = 0; i < MAX_CARS; i++) {
            if (!cars[i].active) continue;
            erase_car(car_lane_x[cars[i].lane], cars[i].y);
            cars[i].y += speed;
            if (cars[i].y > SCR_H) {
                cars[i].active = false;
            } else {
                draw_car(car_lane_x[cars[i].lane], cars[i].y, cars[i].color);
            }
        }

        /* === Spawn new cars === */
        spawn_timer--;
        if (spawn_timer <= 0) {
            /* Find a free slot */
            int slot = -1;
            for (int i = 0; i < MAX_CARS; i++) {
                if (!cars[i].active) { slot = i; break; }
            }
            if (slot >= 0) {
                int cl = rng_range(0, 2);
                /* Check if another car is near the top in same lane */
                bool blocked = false;
                for (int i = 0; i < MAX_CARS; i++) {
                    if (cars[i].active && cars[i].lane == cl && cars[i].y < CAR_H + 15) {
                        blocked = true;
                        break;
                    }
                }
                if (!blocked) {
                    cars[slot].active = true;
                    cars[slot].lane = cl;
                    cars[slot].y = -CAR_H;
                    cars[slot].color = car_colors[rng_range(0, NUM_CAR_COLORS - 1)];
                }
            }
            spawn_timer = spawn_rate;
        }

        /* === Update scroll === */
        mark_offset += speed;
        draw_lane_marks(mark_offset, C_LANE_WHT);

        /* === Redraw grass stripes (scrolling effect) === */
        if (frame % 6 == 0) {
            for (int gy = 0; gy < SCR_H; gy += 5) {
                int phase = (gy + mark_offset) % 20;
                eadk_color_t gc = (phase < 10) ? C_GRASS_DK : C_GRASS;
                fill(0, gy, ROAD_L - EDGE_W, 5, gc);
                fill(ROAD_R + EDGE_W, gy, SCR_W - ROAD_R - EDGE_W, 5, gc);
            }
        }

        /* === Draw player === */
        draw_player(player_lane_x[lane], PLAYER_Y);

        /* === Collision check === */
        if (check_collision(player_lane_x[lane], PLAYER_Y, cars)) {
            collision_flash();
            show_crash_screen(score);
            return;
        }

        /* === Score === */
        score++;
        if (frame % 8 == 0) {
            char s[12];
            itoa_simple(score, s);
            /* Draw on grass area */
            fill(ROAD_R + 8, 4, 50, 14, C_GRASS);
            eadk_display_draw_string(s,
                (eadk_point_t){ROAD_R + 10, 5},
                false, C_WHITE, C_GRASS);
        }

        /* === Difficulty ramp === */
        if (frame > 0 && frame % 180 == 0) {
            if (speed < 8) speed++;
            if (spawn_rate > 12) spawn_rate -= 3;
        }

        frame++;

        /* === Frame timing (~30fps target) === */
        eadk_display_wait_for_vblank();
        uint64_t elapsed = eadk_timing_millis() - frame_start;
        if (elapsed < 33) {
            eadk_timing_msleep((uint32_t)(33 - elapsed));
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Entry point                                                        */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    while (true) {
        game_round();
    }
    return 0;
}
