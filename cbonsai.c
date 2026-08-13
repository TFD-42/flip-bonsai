// Flip Bonsai — Flipper Zero port of cbonsai (https://gitlab.com/jallbrit/cbonsai)
//
// Derivative work: setDeltas() and the recursive growth logic in
// grow_branch() are a direct port of cbonsai's setDeltas() and branch()
// (Copyright (C) jallbrit and contributors, GPL-3.0-or-later). Adapted from
// a terminal character grid to the Flipper's 128x64 monochrome pixel
// canvas: wood steps are drawn as short connected line segments and leaves
// as single dots/discs instead of glyphs, and recursion/storage are bounded
// for the device's fixed stack and RAM. This file is modified from, and
// distributed under, the same GPL-3.0-or-later license as the original —
// see LICENSE.
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version. This program is distributed WITHOUT ANY
// WARRANTY; see the GNU General Public License for details.

#include <furi.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define BASE_HEIGHT 4
#define LIFE_START 32
#define MULTIPLIER 5
#define MAX_STROKES 2200
#define MAX_DEPTH 160

typedef enum {
    Trunk,
    ShootLeft,
    ShootRight,
    Dying,
    Dead,
} BranchType;

typedef struct {
    uint8_t x0, y0, x1, y1;
    uint8_t type; // BranchType, for render style
    bool bold;
} Stroke;

typedef enum {
    ModeMenu,
    ModeTree,
    ModeHelp,
} AppMode;

typedef enum {
    MenuNew,
    MenuHelp,
    MenuQuit,
    MenuCount,
} MenuItem;

typedef struct {
    AppMode mode;
    int menu_selected;

    Stroke strokes[MAX_STROKES];
    int stroke_count;
    int branches;
    int shoots;
    uint32_t shoot_counter;

    FuriMutex* mutex;
} AppState;

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

static uint32_t rnd(uint32_t mod) {
    if(mod == 0) return 0;
    return furi_hal_random_get() % mod;
}

// port of cbonsai's setDeltas()
static void set_deltas(BranchType type, int life, int age, int multiplier, int* out_dx, int* out_dy) {
    int dx = 0, dy = 0;
    uint32_t dice;

    switch(type) {
    case Trunk:
        if(age <= 2 || life < 4) {
            dy = 0;
            dx = (int)rnd(3) - 1;
        } else if(age < multiplier * 3) {
            int step = (int)(multiplier * 0.5f);
            if(step < 1) step = 1;
            dy = (age % step == 0) ? -1 : 0;

            dice = rnd(10);
            if(dice == 0) dx = -2;
            else if(dice <= 3) dx = -1;
            else if(dice <= 5) dx = 0;
            else if(dice <= 8) dx = 1;
            else dx = 2;
        } else {
            dice = rnd(10);
            dy = (dice > 2) ? -1 : 0;
            dx = (int)rnd(3) - 1;
        }
        break;

    case ShootLeft:
        dice = rnd(10);
        if(dice <= 1) dy = -1;
        else if(dice <= 7) dy = 0;
        else dy = 1;

        dice = rnd(10);
        if(dice <= 1) dx = -2;
        else if(dice <= 5) dx = -1;
        else if(dice <= 8) dx = 0;
        else dx = 1;
        break;

    case ShootRight:
        dice = rnd(10);
        if(dice <= 1) dy = -1;
        else if(dice <= 7) dy = 0;
        else dy = 1;

        dice = rnd(10);
        if(dice <= 1) dx = 2;
        else if(dice <= 5) dx = 1;
        else if(dice <= 8) dx = 0;
        else dx = -1;
        break;

    case Dying:
        dice = rnd(10);
        if(dice <= 1) dy = -1;
        else if(dice <= 8) dy = 0;
        else dy = 1;

        dice = rnd(15);
        if(dice == 0) dx = -3;
        else if(dice <= 2) dx = -2;
        else if(dice <= 5) dx = -1;
        else if(dice <= 8) dx = 0;
        else if(dice <= 11) dx = 1;
        else if(dice <= 13) dx = 2;
        else dx = 3;
        break;

    case Dead:
        dice = rnd(10);
        if(dice <= 2) dy = -1;
        else if(dice <= 6) dy = 0;
        else dy = 1;
        dx = (int)rnd(3) - 1;
        break;
    }

    *out_dx = dx;
    *out_dy = dy;
}

static void add_stroke(AppState* s, int x0, int y0, int x1, int y1, BranchType renderType, bool bold) {
    if(s->stroke_count >= MAX_STROKES) return;
    Stroke* st = &s->strokes[s->stroke_count++];
    st->x0 = (uint8_t)x0;
    st->y0 = (uint8_t)y0;
    st->x1 = (uint8_t)x1;
    st->y1 = (uint8_t)y1;
    st->type = (uint8_t)renderType;
    st->bold = bold;
}

// port of cbonsai's branch(), recording strokes instead of drawing to ncurses live
static void grow_branch(AppState* s, int y, int x, BranchType type, int life, int depth) {
    if(depth > MAX_DEPTH) return;
    if(s->stroke_count >= MAX_STROKES) return;

    s->branches++;
    int dx = 0, dy = 0, age = 0;
    int shootCooldown = MULTIPLIER;

    while(life > 0) {
        life--;
        age = LIFE_START - life;

        set_deltas(type, life, age, MULTIPLIER, &dx, &dy);

        int maxY = SCREEN_H - BASE_HEIGHT;
        if(dy > 0 && y > (maxY - 2)) dy--;

        if(life < 3) {
            grow_branch(s, y, x, Dead, life, depth + 1);
        } else if(type == Trunk && life < (MULTIPLIER + 2)) {
            grow_branch(s, y, x, Dying, life, depth + 1);
        } else if((type == ShootLeft || type == ShootRight) && life < (MULTIPLIER + 2)) {
            grow_branch(s, y, x, Dying, life, depth + 1);
        } else if(type == Trunk && ((rnd(3) == 0) || (life % MULTIPLIER == 0))) {
            if(rnd(8) == 0 && life > 7) {
                shootCooldown = MULTIPLIER * 2;
                grow_branch(s, y, x, Trunk, life + (int)rnd(5) - 2, depth + 1);
            } else if(shootCooldown <= 0) {
                shootCooldown = MULTIPLIER * 2;
                int shootLife = life + MULTIPLIER;
                s->shoots++;
                s->shoot_counter++;
                grow_branch(s, y, x, (BranchType)((s->shoot_counter % 2) + 1), shootLife, depth + 1);
            }
        }
        shootCooldown--;

        int px = x, py = y;
        x += dx;
        y += dy;
        if(x < 0) x = 0;
        if(x > SCREEN_W - 1) x = SCREEN_W - 1;
        if(y < 0) y = 0;
        if(y > SCREEN_H - 1 - BASE_HEIGHT) y = SCREEN_H - 1 - BASE_HEIGHT;

        BranchType renderType = (life < 4) ? Dying : type;
        bool bold;
        switch(renderType) {
        case Trunk:
        case ShootLeft:
        case ShootRight:
            bold = (rnd(2) == 0);
            break;
        case Dying:
            bold = (rnd(10) == 0);
            break;
        default: // Dead
            bold = (rnd(3) == 0);
            break;
        }

        add_stroke(s, px, py, x, y, renderType, bold);

        if(s->stroke_count >= MAX_STROKES) return;
    }
}

static void grow_tree(AppState* s) {
    s->stroke_count = 0;
    s->branches = 0;
    s->shoots = 0;
    s->shoot_counter = rnd(0xFFFF);

    int start_x = SCREEN_W / 2;
    int start_y = SCREEN_H - 1 - BASE_HEIGHT;
    grow_branch(s, start_y, start_x, Trunk, LIFE_START, 0);
}

static void draw_pot(Canvas* canvas) {
    int cy = SCREEN_H - BASE_HEIGHT;
    canvas_draw_line(canvas, SCREEN_W / 2 - 16, cy, SCREEN_W / 2 + 16, cy);
    canvas_draw_line(canvas, SCREEN_W / 2 - 14, cy + 1, SCREEN_W / 2 + 14, cy + 1);
    canvas_draw_line(canvas, SCREEN_W / 2 - 14, cy + 1, SCREEN_W / 2 - 17, cy + 3);
    canvas_draw_line(canvas, SCREEN_W / 2 + 14, cy + 1, SCREEN_W / 2 + 17, cy + 3);
    canvas_draw_line(canvas, SCREEN_W / 2 - 17, cy + 3, SCREEN_W / 2 + 17, cy + 3);
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* s = ctx;
    furi_mutex_acquire(s->mutex, FuriWaitForever);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(s->mode == ModeMenu) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 28, 10, "= BONSAI =");
        canvas_draw_str(canvas, 5, 20, "ported from cbonsai");

        const char* items[] = {"New tree", "Help", "Quit"};
        for(int i = 0; i < MenuCount; i++) {
            int y = 34 + i * 10;
            if(i == s->menu_selected) {
                canvas_draw_str(canvas, 5, y, ">");
            }
            canvas_draw_str(canvas, 16, y, items[i]);
        }
    } else if(s->mode == ModeHelp) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 8, "HELP");
        canvas_draw_str(canvas, 2, 18, "Same growth algorithm as");
        canvas_draw_str(canvas, 2, 26, "cbonsai: trunk/shoot/dying/");
        canvas_draw_str(canvas, 2, 34, "dead branches, recursive.");
        canvas_draw_str(canvas, 2, 44, "OK: grow a new random tree");
        canvas_draw_str(canvas, 2, 52, "BACK: return to menu");
    } else { // ModeTree
        for(int i = 0; i < s->stroke_count; i++) {
            Stroke* st = &s->strokes[i];
            BranchType t = (BranchType)st->type;
            if(t == Trunk || t == ShootLeft || t == ShootRight) {
                canvas_draw_line(canvas, st->x0, st->y0, st->x1, st->y1);
                if(st->bold) canvas_draw_line(canvas, st->x0 + 1, st->y0, st->x1 + 1, st->y1);
            } else if(t == Dying) {
                if(st->bold) canvas_draw_disc(canvas, st->x1, st->y1, 1);
                else canvas_draw_dot(canvas, st->x1, st->y1);
            } else { // Dead
                canvas_draw_dot(canvas, st->x1, st->y1);
            }
        }

        draw_pot(canvas);

        canvas_set_font(canvas, FontSecondary);
        char buf[24];
        snprintf(buf, sizeof(buf), "branches:%d", s->branches);
        canvas_draw_str(canvas, 2, 8, buf);
    }

    furi_mutex_release(s->mutex);
}

static void input_callback(InputEvent* input, void* ctx) {
    FuriMessageQueue* queue = ctx;
    AppEvent event = {.type = EventTypeInput, .input = *input};
    furi_message_queue_put(queue, &event, FuriWaitForever);
}

int32_t cbonsai_app(void* p) {
    UNUSED(p);

    AppState* s = malloc(sizeof(AppState));
    memset(s, 0, sizeof(AppState));
    s->mode = ModeMenu;
    s->menu_selected = 0;
    s->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(AppEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, s);
    view_port_input_callback_set(view_port, input_callback, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    bool running = true;
    AppEvent event;
    while(running) {
        if(furi_message_queue_get(queue, &event, FuriWaitForever) != FuriStatusOk) continue;

        furi_mutex_acquire(s->mutex, FuriWaitForever);
        InputEvent* in = &event.input;

        if(in->type == InputTypeLong && in->key == InputKeyBack) {
            running = false;
        } else if(s->mode == ModeMenu) {
            if((in->type == InputTypeShort || in->type == InputTypeRepeat) && in->key == InputKeyUp) {
                s->menu_selected = (s->menu_selected - 1 + MenuCount) % MenuCount;
            }
            if((in->type == InputTypeShort || in->type == InputTypeRepeat) && in->key == InputKeyDown) {
                s->menu_selected = (s->menu_selected + 1) % MenuCount;
            }
            if(in->type == InputTypeShort && in->key == InputKeyOk) {
                if(s->menu_selected == MenuNew) {
                    grow_tree(s);
                    s->mode = ModeTree;
                } else if(s->menu_selected == MenuHelp) {
                    s->mode = ModeHelp;
                } else if(s->menu_selected == MenuQuit) {
                    running = false;
                }
            }
        } else if(s->mode == ModeHelp) {
            if(in->type == InputTypeShort && in->key == InputKeyBack) {
                s->mode = ModeMenu;
            }
        } else { // ModeTree
            if(in->type == InputTypeShort && in->key == InputKeyOk) {
                grow_tree(s);
            }
            if(in->type == InputTypeShort && in->key == InputKeyBack) {
                s->mode = ModeMenu;
            }
        }

        furi_mutex_release(s->mutex);
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(queue);
    furi_mutex_free(s->mutex);
    free(s);

    return 0;
}
