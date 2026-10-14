#ifndef SPRITES_H
#define SPRITES_H

#include <eadk.h>

/* ------------------------------------------------------------------ */
/*  RGB565 helper                                                      */
/* ------------------------------------------------------------------ */
#define RGB(r,g,b) ((eadk_color_t)(((r>>3)<<11)|((g>>2)<<5)|(b>>3)))

/* ------------------------------------------------------------------ */
/*  Palette                                                            */
/* ------------------------------------------------------------------ */
#define C_TRANS      0x0001  /* transparent marker (won't appear naturally) */

#define C_SKIN       RGB(240,210,180)
#define C_SKIN_SH    RGB(210,180,150)
#define C_HAIR       RGB(60,40,20)
#define C_HAIR_HI    RGB(90,60,30)
#define C_SUIT       RGB(30,30,30)
#define C_SUIT_HI    RGB(55,55,55)
#define C_TIE        RGB(200,30,30)
#define C_TIE_HI     RGB(230,60,60)
#define C_SHIRT      RGB(240,240,240)
#define C_SHOE       RGB(50,30,10)
#define C_WHEEL      RGB(40,40,40)
#define C_SPOKE      RGB(170,170,170)
#define C_FRAME      RGB(80,80,80)
#define C_FRAME_HI   RGB(120,120,120)
#define C_HANDLEBAR  RGB(100,100,100)
#define C_EYE        RGB(40,30,20)
#define C_MOUTH      RGB(180,100,80)
#define C_WINDOW     RGB(140,200,240)
#define C_HEADLIGHT  RGB(255,255,150)
#define C_TAILLIGHT  RGB(200,40,40)

/* Road / environment */
#define C_ROAD       RGB(90,90,90)
#define C_ROAD_DARK  RGB(70,70,70)
#define C_GRASS      RGB(50,160,50)
#define C_GRASS_DK   RGB(35,120,35)
#define C_LANE_WHT   RGB(255,255,255)
#define C_EDGE_YEL   RGB(220,200,40)
#define C_BLACK      RGB(0,0,0)
#define C_WHITE      RGB(255,255,255)
#define C_RED        RGB(220,30,30)
#define C_BLUE       RGB(0,0,200)

/* Car body colors */
#define C_CAR_BLUE   RGB(40,80,200)
#define C_CAR_RED    RGB(200,40,40)
#define C_CAR_YEL    RGB(220,200,40)
#define C_CAR_GRN    RGB(40,180,60)
#define C_CAR_WHT    RGB(230,230,230)

/* ------------------------------------------------------------------ */
/*  Draw helpers – fast rect push                                      */
/* ------------------------------------------------------------------ */
static inline void fill(int x, int y, int w, int h, eadk_color_t c) {
    if (w <= 0 || h <= 0) return;
    eadk_display_push_rect_uniform((eadk_rect_t){x, y, w, h}, c);
}

/* ------------------------------------------------------------------ */
/*  Player sprite  (suited cyclist, top-down, ~24 x 34)                */
/* ------------------------------------------------------------------ */
#define PLAYER_W 24
#define PLAYER_H 34

static void draw_player(int x, int y) {
    /* --- Back wheel --- */
    fill(x+7, y+26, 10, 8, C_WHEEL);
    fill(x+9, y+28, 6, 4, C_SPOKE);

    /* --- Bicycle frame --- */
    fill(x+11, y+12, 3, 18, C_FRAME);
    fill(x+10, y+16, 5, 2, C_FRAME_HI);  /* crossbar */

    /* --- Front wheel --- */
    fill(x+7, y+2, 10, 8, C_WHEEL);
    fill(x+9, y+4, 6, 4, C_SPOKE);

    /* --- Handlebars --- */
    fill(x+4, y+6, 16, 2, C_HANDLEBAR);
    fill(x+3, y+5, 3, 3, C_HANDLEBAR);
    fill(x+18, y+5, 3, 3, C_HANDLEBAR);

    /* --- Legs (suit trousers) --- */
    fill(x+5, y+22, 5, 10, C_SUIT);
    fill(x+14, y+22, 5, 10, C_SUIT);
    /* Trouser highlight */
    fill(x+6, y+23, 2, 8, C_SUIT_HI);
    fill(x+15, y+23, 2, 8, C_SUIT_HI);

    /* --- Shoes --- */
    fill(x+5, y+30, 5, 3, C_SHOE);
    fill(x+14, y+30, 5, 3, C_SHOE);

    /* --- Torso (suit jacket) --- */
    fill(x+6, y+12, 12, 12, C_SUIT);
    /* Jacket lapel highlights */
    fill(x+7, y+13, 2, 8, C_SUIT_HI);
    fill(x+15, y+13, 2, 8, C_SUIT_HI);

    /* --- Shirt collar --- */
    fill(x+9, y+10, 6, 3, C_SHIRT);

    /* --- Tie --- */
    fill(x+11, y+12, 2, 10, C_TIE);
    fill(x+11, y+13, 2, 2, C_TIE_HI);  /* tie knot highlight */

    /* --- Arms (suit sleeves reaching to handlebars) --- */
    fill(x+3, y+8, 4, 14, C_SUIT);
    fill(x+17, y+8, 4, 14, C_SUIT);
    /* Forearm highlight */
    fill(x+4, y+9, 2, 6, C_SUIT_HI);
    fill(x+18, y+9, 2, 6, C_SUIT_HI);

    /* --- Hands on handlebars --- */
    fill(x+3, y+6, 4, 3, C_SKIN);
    fill(x+17, y+6, 4, 3, C_SKIN);

    /* --- Head --- */
    fill(x+7, y+2, 10, 10, C_SKIN);
    fill(x+8, y+3, 8, 8, C_SKIN);
    /* Shadow on sides of face */
    fill(x+7, y+4, 1, 6, C_SKIN_SH);
    fill(x+16, y+4, 1, 6, C_SKIN_SH);

    /* --- Hair --- */
    fill(x+7, y+1, 10, 4, C_HAIR);
    fill(x+6, y+2, 2, 3, C_HAIR);   /* hair side L */
    fill(x+16, y+2, 2, 2, C_HAIR);  /* hair side R */
    /* Part / highlight */
    fill(x+9, y+1, 2, 2, C_HAIR_HI);

    /* --- Eyebrows (thick) --- */
    fill(x+8, y+5, 4, 1, C_HAIR);
    fill(x+13, y+5, 4, 1, C_HAIR);

    /* --- Eyes --- */
    fill(x+9, y+6, 2, 2, C_EYE);
    fill(x+14, y+6, 2, 2, C_EYE);
    /* Eye whites */
    fill(x+9, y+6, 1, 1, C_WHITE);
    fill(x+14, y+6, 1, 1, C_WHITE);

    /* --- Mouth --- */
    fill(x+10, y+9, 4, 1, C_MOUTH);
}

/* ------------------------------------------------------------------ */
/*  Erase player (draw road over it)                                   */
/* ------------------------------------------------------------------ */
static void erase_player(int x, int y) {
    fill(x, y, PLAYER_W, PLAYER_H, C_ROAD);
}

/* ------------------------------------------------------------------ */
/*  Car sprite  (top-down, 22 x 36)                                    */
/* ------------------------------------------------------------------ */
#define CAR_W 22
#define CAR_H 36

static void draw_car(int x, int y, eadk_color_t body) {
    /* --- Main body --- */
    fill(x+2, y+3, CAR_W-4, CAR_H-6, body);
    fill(x, y+6, CAR_W, CAR_H-12, body);

    /* --- Body shading (darker edges) --- */
    eadk_color_t shade = RGB(
        ((body>>11)&0x1F)*6,  /* darken red */
        ((body>>5)&0x3F)*3,   /* darken green */
        (body&0x1F)*6         /* darken blue */
    );
    fill(x, y+6, 2, CAR_H-12, shade);
    fill(x+CAR_W-2, y+6, 2, CAR_H-12, shade);

    /* --- Windshield (front - bottom, since car faces us) --- */
    fill(x+4, y+CAR_H-12, CAR_W-8, 7, C_WINDOW);
    fill(x+5, y+CAR_H-11, CAR_W-10, 5, RGB(160,220,250)); /* highlight */

    /* --- Rear window --- */
    fill(x+5, y+5, CAR_W-10, 5, C_WINDOW);

    /* --- Headlights --- */
    fill(x+2, y+CAR_H-4, 5, 3, C_HEADLIGHT);
    fill(x+CAR_W-7, y+CAR_H-4, 5, 3, C_HEADLIGHT);

    /* --- Taillights --- */
    fill(x+2, y+1, 4, 3, C_TAILLIGHT);
    fill(x+CAR_W-6, y+1, 4, 3, C_TAILLIGHT);

    /* --- Side mirrors --- */
    fill(x-1, y+CAR_H-14, 3, 3, body);
    fill(x+CAR_W-2, y+CAR_H-14, 3, 3, body);

    /* --- Roof line --- */
    fill(x+3, y+10, CAR_W-6, 1, shade);
}

static void erase_car(int x, int y) {
    /* Erase slightly wider to cover mirrors */
    fill(x-1, y, CAR_W+2, CAR_H, C_ROAD);
}

/* ------------------------------------------------------------------ */
/*  Crash screen character (standing, front view, ~24 x 56)            */
/* ------------------------------------------------------------------ */
static void draw_crash_guy(int x, int y) {
    /* --- Head --- */
    fill(x+5, y, 14, 14, C_SKIN);
    fill(x+6, y+1, 12, 12, C_SKIN);
    fill(x+5, y+3, 1, 8, C_SKIN_SH);
    fill(x+18, y+3, 1, 8, C_SKIN_SH);

    /* --- Hair --- */
    fill(x+5, y-2, 14, 5, C_HAIR);
    fill(x+4, y, 2, 4, C_HAIR);
    fill(x+18, y, 2, 3, C_HAIR);
    fill(x+8, y-2, 3, 2, C_HAIR_HI);

    /* --- Eyebrows --- */
    fill(x+7, y+4, 4, 2, C_HAIR);
    fill(x+13, y+4, 4, 2, C_HAIR);

    /* --- Eyes --- */
    fill(x+8, y+6, 3, 2, C_EYE);
    fill(x+14, y+6, 3, 2, C_EYE);
    fill(x+8, y+6, 1, 1, C_WHITE);
    fill(x+14, y+6, 1, 1, C_WHITE);

    /* --- Sad mouth --- */
    fill(x+9, y+10, 6, 1, C_MOUTH);
    fill(x+8, y+9, 2, 1, C_MOUTH);
    fill(x+14, y+9, 2, 1, C_MOUTH);

    /* --- Neck --- */
    fill(x+10, y+13, 4, 3, C_SKIN);

    /* --- Shirt collar --- */
    fill(x+7, y+15, 10, 3, C_SHIRT);

    /* --- Suit jacket --- */
    fill(x+4, y+17, 16, 22, C_SUIT);
    fill(x+5, y+18, 3, 18, C_SUIT_HI);
    fill(x+16, y+18, 3, 18, C_SUIT_HI);

    /* --- Tie --- */
    fill(x+11, y+17, 2, 18, C_TIE);
    fill(x+11, y+18, 2, 3, C_TIE_HI);

    /* --- Arms --- */
    fill(x+1, y+18, 4, 18, C_SUIT);
    fill(x+19, y+18, 4, 18, C_SUIT);
    fill(x+2, y+19, 2, 10, C_SUIT_HI);
    fill(x+20, y+19, 2, 10, C_SUIT_HI);

    /* --- Hands --- */
    fill(x+1, y+35, 4, 4, C_SKIN);
    fill(x+19, y+35, 4, 4, C_SKIN);

    /* --- Trousers --- */
    fill(x+5, y+39, 6, 16, C_SUIT);
    fill(x+13, y+39, 6, 16, C_SUIT);
    fill(x+6, y+40, 3, 12, C_SUIT_HI);
    fill(x+14, y+40, 3, 12, C_SUIT_HI);

    /* --- Shoes --- */
    fill(x+4, y+54, 7, 4, C_SHOE);
    fill(x+13, y+54, 7, 4, C_SHOE);
    fill(x+5, y+54, 3, 2, RGB(70,50,30)); /* shoe highlight */
    fill(x+14, y+54, 3, 2, RGB(70,50,30));
}

#endif /* SPRITES_H */
