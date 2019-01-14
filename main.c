// [main.c]
// this template is provided for the 2D shooter game.

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>

// If defined, logs will be shown on console and written to file.
// If commented out, logs will not be shown nor be saved.
#define LOG_ENABLED

/* Constants. */

// Frame rate (frame per second)
const int FPS = 60;
// Display (screen) width.
const int SCREEN_W = 800;
// Display (screen) height.
const int SCREEN_H = 600;
// At most 4 audios can be played at a time.
const int RESERVE_SAMPLES = 4;
// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_START = 2;
enum {
	SCENE_MENU = 1,
	SCENE_START = 2,
    SCENE_SETTINGS = 3,
    SCENE_GAMEOVER = 4,
    SCENE_DEAD = 5,
    SCENE_BOSS = 6
};

/* Input states */

// The active scene id.
int active_scene;
// Keyboard state, whether the key is down or not.
bool key_state[ALLEGRO_KEY_MAX];
// Mouse state, whether the key is down or not.
// 1 is for left, 2 is for right, 3 is for middle.
bool *mouse_state;
// Mouse position.
int mouse_x, mouse_y;
// TODO: More variables to store input states such as joysticks, ...

/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;

/* Shared resources*/

ALLEGRO_FONT* font_cerena_48;
ALLEGRO_FONT* font_pirulen_24;
ALLEGRO_FONT* font_quantico_20;
ALLEGRO_FONT* font_quantico_28;
ALLEGRO_FONT* font_paint_48;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

/* Setting Scene resources */
ALLEGRO_BITMAP* img_controls_move;
ALLEGRO_BITMAP* img_controls_shoot;
ALLEGRO_BITMAP* img_icon_p51;
ALLEGRO_BITMAP* img_icon_camo;
ALLEGRO_BITMAP* img_icon_jet;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* img_plane_p51;
ALLEGRO_BITMAP* img_plane_camo;
ALLEGRO_BITMAP* img_plane_jet;
ALLEGRO_BITMAP* start_img_enemy;
ALLEGRO_BITMAP* img_boss;
ALLEGRO_BITMAP* img_bullet;
ALLEGRO_BITMAP* img_life;
ALLEGRO_BITMAP* img_plane_explode;
ALLEGRO_BITMAP* img_boss_bullet;

ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;
ALLEGRO_SAMPLE* enemy_explode;
ALLEGRO_SAMPLE_ID enemy_explode_id;
ALLEGRO_SAMPLE* bullet_fired;
ALLEGRO_SAMPLE_ID bullet_fired_id;
ALLEGRO_SAMPLE* plane_hit;
ALLEGRO_SAMPLE_ID plane_hit_id;
ALLEGRO_SAMPLE* plane_crash;
ALLEGRO_SAMPLE_ID plane_crash_id;
ALLEGRO_SAMPLE* boss_bgm;
ALLEGRO_SAMPLE_ID boss_bgm_id;
ALLEGRO_SAMPLE* level_up;
ALLEGRO_SAMPLE_ID level_up_id;

/* Game over scene */
ALLEGRO_BITMAP* victory_background;
ALLEGRO_SAMPLE* game_over_bgm;
ALLEGRO_SAMPLE_ID game_over_bgm_id;
ALLEGRO_SAMPLE* victory_bgm;
ALLEGRO_SAMPLE_ID victory_bgm_id;

typedef struct {
	// The center coordinate of the image.
	float x, y;
	// The width and height of the object.
	float w, h;
	// The velocity in x, y axes.
	float vx, vy;
	// Should we draw this object on the screen.
	bool hidden;
	// The pointer to the object’s image.
	ALLEGRO_BITMAP* img;
} MovableObject;
void draw_movable_object(MovableObject obj);
#define MAX_ENEMY 10
#define MAX_BULLET 20
MovableObject plane;
MovableObject boss;
MovableObject enemies[MAX_ENEMY];
MovableObject bullets[MAX_BULLET];
MovableObject boss_bullet1[MAX_BULLET];
MovableObject boss_bullet2[MAX_BULLET];

/* Declare own variables */
const float MAX_COOLDOWN = 0.2f;
double last_shoot_timestamp, last_enemy_release_timestamp, last_plane_hit_timestamp, last_boss_timestamp;
int enemies_count;
int health, lives, score, level;
int plane_num;
int boss_dir, boss_hit, boss_shoot;

/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);
// [HACKATHON 3-2]
// TODO: Declare a function.
// Determines whether the point (px, py) is in rect (x, y, w, h).
// Uncomment the code below.
bool pnt_in_rect(int px, int py, int x, int y, int w, int h);
bool in_back_btn(int px, int py, int x, int y, int w, int h);
bool check_hit_bullet(MovableObject enemies, MovableObject bullets);
bool check_hit_bullet_boss(MovableObject boss, MovableObject bullets);
bool check_hit_plane(MovableObject enemies, MovableObject plane);
bool check_hit_plane_boss(MovableObject boss, MovableObject plane);
void show_enemy(void);

/* Event callbacks. */
void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

/* Declare function prototypes for debugging. */

// Display error message and exit the program, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// If the program crashes unexpectedly, you can inspect "log.txt" for
// further information.
void game_abort(const char* format, ...);
// Log events for later debugging, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// You can inspect "log.txt" for logs in the last run.
void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
	// Set random seed for better random outcome.
	srand(time(NULL));
	allegro5_init();
	game_log("Allegro5 initialized");
	game_log("Game begin");
	// Initialize game variables.
	game_init();
	game_log("Game initialized");
	// Draw the first frame.
	game_draw();
	game_log("Game start event loop");
	// This call blocks until the game is finished.
	game_start_event_loop();
	game_log("Game end");
	game_destroy();
	return 0;
}

void allegro5_init(void) {
	if (!al_init())
		game_abort("failed to initialize allegro");

	// Initialize add-ons.
	if (!al_init_primitives_addon())
		game_abort("failed to initialize primitives add-on");
	if (!al_init_font_addon())
		game_abort("failed to initialize font add-on");
	if (!al_init_ttf_addon())
		game_abort("failed to initialize ttf add-on");
	if (!al_init_image_addon())
		game_abort("failed to initialize image add-on");
	if (!al_install_audio())
		game_abort("failed to initialize audio add-on");
	if (!al_init_acodec_addon())
		game_abort("failed to initialize audio codec add-on");
	if (!al_reserve_samples(RESERVE_SAMPLES))
		game_abort("failed to reserve samples");
	if (!al_install_keyboard())
		game_abort("failed to install keyboard");
	if (!al_install_mouse())
		game_abort("failed to install mouse");
	// TODO: Initialize other addons such as video, ...

	// Setup game display.
	game_display = al_create_display(SCREEN_W, SCREEN_H);
	if (!game_display)
		game_abort("failed to create display");
	al_set_window_title(game_display, "I2P(I)_2018_Yang Final Project <student_id>");

	// Setup update timer.
	game_update_timer = al_create_timer(1.0f / FPS);
	if (!game_update_timer)
		game_abort("failed to create timer");

	// Setup event queue.
	game_event_queue = al_create_event_queue();
	if (!game_event_queue)
		game_abort("failed to create event queue");

	// Malloc mouse buttons state according to button counts.
	const unsigned m_buttons = al_get_mouse_num_buttons();
	game_log("There are total %u supported mouse buttons", m_buttons);
	// mouse_state[0] will not be used.
	mouse_state = malloc((m_buttons + 1) * sizeof(bool));
	memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));

	// Register display, timer, keyboard, mouse events to the event queue.
	al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
	al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
	al_register_event_source(game_event_queue, al_get_keyboard_event_source());
	al_register_event_source(game_event_queue, al_get_mouse_event_source());
	// TODO: Register other event sources such as timer, video, ...

	// Start the timer to update and draw the game.
	al_start_timer(game_update_timer);
}

void game_init(void) {
	/* Shared resources*/
	font_cerena_48 = al_load_font("cerena.ttf", 48, 0);
	if (!font_cerena_48)
		game_abort("failed to load font: cerena.ttf with size 50");

	font_pirulen_24 = al_load_font("pirulen.ttf", 24, 0);
	if (!font_pirulen_24)
		game_abort("failed to load font: pirulen.ttf with size 24");

    font_quantico_20 = al_load_font("Quantico.ttf", 20, 0);
    if (!font_quantico_20)
        game_abort("failed to load font: Quantico.ttf with size 20");

    font_quantico_28 = al_load_font("Quantico.ttf", 28, 0);
    if (!font_quantico_28)
        game_abort("failed to load font: Quantico.ttf with size 28");

    font_paint_48 = al_load_font("paint.ttf", 48, 0);
    if (!font_paint_48)
        game_abort("failed to load font: paint.ttf with size 48");

	/* Menu Scene resources*/
	main_img_background = load_bitmap_resized("main-bg.jpg", SCREEN_W, SCREEN_H);

	main_bgm = al_load_sample("main_menu.ogg");
	if (!main_bgm)
		game_abort("failed to load audio: main_menu.ogg");

	img_settings = al_load_bitmap("settings.png");
	if (!img_settings)
		game_abort("failed to load image: settings.png");
	img_settings2 = al_load_bitmap("settings2.png");
	if (!img_settings2)
		game_abort("failed to load image: settings2.png");

    /* Settings Scene resources */
    img_controls_move = al_load_bitmap("controls_move.png");
    if (!img_controls_move)
        game_abort("failed to load image: controls_move.png");

    img_controls_shoot = al_load_bitmap("controls_shoot.png");
    if (!img_controls_shoot)
        game_abort("failed to load image: controls_shoot.png");

    img_icon_p51 = al_load_bitmap("p51_icon.png");
    if (!img_icon_p51)
        game_abort("failed to load image: p51_icon.png");

    img_icon_camo = al_load_bitmap("camo_icon.png");
    if (!img_icon_camo)
        game_abort("failed to load image: camo_icon.png");

    img_icon_jet = al_load_bitmap("jet_icon.png");
    if (!img_icon_jet)
        game_abort("failed to load image: jet_icon.png");

	/* Start Scene resources*/
	start_img_background = load_bitmap_resized("start-bg.jpg", SCREEN_W, SCREEN_H);

	img_plane_p51 = al_load_bitmap("p51.png");
	if (!img_plane_p51)
		game_abort("failed to load image: p51.png");

    img_plane_camo = al_load_bitmap("camo.png");
    if (!img_plane_camo)
        game_abort("failed to load bitmap: camo.png");

    img_plane_jet = al_load_bitmap("jet.png");
    if (!img_plane_jet)
        game_abort("failed to load bitmap: jet.png");

	start_img_enemy = al_load_bitmap("smallfighter0006.png");
	if (!start_img_enemy)
		game_abort("failed to load image: smallfighter0006.png");

    img_boss = al_load_bitmap("boss.png");
    if (!img_boss)
        game_abort("failed to load image: boss.png");

    img_bullet = al_load_bitmap("bullet-metal.png");
	if (!img_bullet)
		game_abort("failed to load image: bullet-metal.png");

    img_life = al_load_bitmap("lives.png");
    if (!img_life)
        game_abort("failed to load image: lives.png");

    img_plane_explode = al_load_bitmap("explode.png");
    if (!img_plane_explode)
        game_abort("failed to load image: explode.png");

    img_boss_bullet = al_load_bitmap("boss_bullet.png");
    if (!img_boss_bullet)
        game_abort("failed to load image: boss_bullet.png");

    //sound
	start_bgm = al_load_sample("scene_start.ogg");
	if (!start_bgm)
		game_abort("failed to load audio: scene_start.ogg");

    bullet_fired = al_load_sample("bullet_fired.ogg");
    if (!bullet_fired)
        game_abort("failed to load audio: bullet_fired.ogg");

    enemy_explode = al_load_sample("enemy_explode.ogg");
    if (!enemy_explode)
        game_abort("failed to load audio: enemy_explode.ogg");

    plane_hit = al_load_sample("plane_hit.ogg");
    if (!plane_hit)
        game_abort("failed to load audio: plane_hit.ogg");

    plane_crash = al_load_sample("plane_crash.ogg");
    if (!plane_crash)
        game_abort("failed to load audio: plane_crash.ogg");

    boss_bgm = al_load_sample("scene_boss.ogg");
    if (!boss_bgm)
        game_abort("failed to load audio: scene_boss.ogg");

    level_up = al_load_sample("level_up.ogg");
    if (!level_up)
        game_abort("failed to load audio: level_up.ogg");

    /* Game over scene resource */
    game_over_bgm = al_load_sample("game_over.ogg");
    if (!game_over_bgm)
        game_abort("failed to load audio: game_over.ogg");

    victory_bgm = al_load_sample("scene_victory.ogg");
    if (!victory_bgm)
        game_abort("failed to load audio: scene_victory.ogg");

    victory_background = load_bitmap_resized("victory.png", SCREEN_W, SCREEN_H);
    if (!victory_background)
        game_abort("failed to load image: victory.png");

	// Change to first scene.
	game_change_scene(SCENE_MENU);
}

void game_start_event_loop(void) {
	bool done = false;
	ALLEGRO_EVENT event;
	int redraws = 0;
	while (!done) {
		al_wait_for_event(game_event_queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			// Event for clicking the window close button.
			game_log("Window close button clicked");
			done = true;
		} else if (event.type == ALLEGRO_EVENT_TIMER) {
			// Event for redrawing the display.
			if (event.timer.source == game_update_timer)
				// The redraw timer has ticked.
				redraws++;
		} else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			// Event for keyboard key down.
			game_log("Key with keycode %d down", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = true;
			on_key_down(event.keyboard.keycode);
		} else if (event.type == ALLEGRO_EVENT_KEY_UP) {
			// Event for keyboard key up.
			game_log("Key with keycode %d up", event.keyboard.keycode);
			key_state[event.keyboard.keycode] = false;
		} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			// Event for mouse key down.
			game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = true;
			on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
		} else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			// Event for mouse key up.
			game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
			mouse_state[event.mouse.button] = false;
		} else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			if (event.mouse.dx != 0 || event.mouse.dy != 0) {
				// Event for mouse move.
				game_log("Mouse move to (%d, %d)", event.mouse.x, event.mouse.y);
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
			} else if (event.mouse.dz != 0) {
				// Event for mouse scroll.
				game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
			}
		}
		// TODO: Process more events and call callbacks by adding more
		// entries inside Scene.

		// Redraw
		if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
			if (redraws > 1)
				game_log("%d frame(s) dropped", redraws - 1);
			// Update and draw the next frame.
			game_update();
			game_draw();
			redraws = 0;
		}
	}
}

void game_update(void) {
	if (active_scene == SCENE_START || active_scene == SCENE_BOSS) {
		plane.vx = plane.vy = 0;
		if (key_state[ALLEGRO_KEY_UP] || key_state[ALLEGRO_KEY_W])
			plane.vy -= 1;
		if (key_state[ALLEGRO_KEY_DOWN] || key_state[ALLEGRO_KEY_S])
			plane.vy += 1;
		if (key_state[ALLEGRO_KEY_LEFT] || key_state[ALLEGRO_KEY_A])
			plane.vx -= 1;
		if (key_state[ALLEGRO_KEY_RIGHT] || key_state[ALLEGRO_KEY_D])
			plane.vx += 1;
		// 0.71 is (1/sqrt(2)).
		plane.y += plane.vy * 4 * (plane.vx ? 0.71f : 1);
		plane.x += plane.vx * 4 * (plane.vy ? 0.71f : 1);

		// Limit the plane's collision box inside the frame.
		if ((plane.x - (plane.w/2)) < 0)
			plane.x = plane.w / 2;
		else if ((plane.x + (plane.w/2)) > SCREEN_W)
			plane.x = SCREEN_W - plane.w / 2;
		if ((plane.y - (plane.h/2)) < 0)
			plane.y = plane.h / 2;
		else if ((plane.y + (plane.h/2)) > SCREEN_H-100)
			plane.y = SCREEN_H-100 - plane.h / 2;

		// Update bullet coordinates.
		int i;
		for (i = 0; i < MAX_BULLET; i++) {
			if (bullets[i].hidden = false)
				continue;
			bullets[i].x += bullets[i].vx;
			bullets[i].y += bullets[i].vy;
			if (bullets[i].y < 0)
				bullets[i].hidden = true;
		}


		// TODO: Shoot if key is down and cool-down is over.
		double now = al_get_time();
		if (key_state[ALLEGRO_KEY_SPACE] && now - last_shoot_timestamp >= MAX_COOLDOWN) {
            al_play_sample(bullet_fired, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &bullet_fired_id);
			for (i = 0; i < MAX_BULLET; i++) {
				if (bullets[i].hidden)
					break;
			}
			if (i == MAX_BULLET)
				return;
			last_shoot_timestamp = now;
			bullets[i].hidden = false;
			bullets[i].x = plane.x;
			bullets[i].y = plane.y - plane.h/2;
		}

		//boss shoot
		int j;
		if (!boss.hidden && now - last_boss_timestamp >= MAX_COOLDOWN && boss_shoot <= 10) {
            for (i = 0; i < MAX_BULLET; i++) {
                if (boss_bullet1[i].hidden)
                    break;
            }
            for (j = 0; j < MAX_BULLET; j++) {
                if (boss_bullet2[j].hidden)
                    break;
            }
            if (i == MAX_BULLET || j == MAX_BULLET)
                return;
            last_boss_timestamp = now;
            boss_bullet1[i].hidden = false;
            boss_bullet1[i].x = boss.x - 50;
            boss_bullet1[i].y = boss.y + boss.y/2;
            boss_bullet2[i].hidden = false;
            boss_bullet2[i].x = boss.x + 50;
            boss_bullet2[i].y = boss.y + boss.y/2;
            boss_shoot++;
		}

		// update boss bullet coordinates
		for (i = 0; i < MAX_BULLET; i++) {
            if (!boss_bullet1[i].hidden) {
                boss_bullet1[i].x += boss_bullet1[i].vx;
                boss_bullet1[i].y += boss_bullet1[i].vy;
                if (boss_bullet1[i].y > SCREEN_H - 100) {
                    boss_bullet1[i].hidden = true;
                    boss_shoot--;
                }
            }
		}
		for (i = 0; i < MAX_BULLET; i++) {
            if (!boss_bullet2[i].hidden) {
                boss_bullet2[i].x += boss_bullet2[i].vx;
                boss_bullet2[i].y += boss_bullet2[i].vy;
                if (boss_bullet2[i].y > SCREEN_H - 100) {
                    boss_bullet2[i].hidden = true;
                }
            }
		}

		//update enemies coordinates
        for (j = 0; j < MAX_ENEMY; j++) {
            if (!enemies[j].hidden) {
                enemies[j].x += enemies[j].vx;
                enemies[j].y += enemies[j].vy;
                if (enemies[j].y > SCREEN_H-100 && !enemies[j].hidden) {
                    health = health - 10;
                    al_play_sample(plane_hit, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_hit_id);
                    enemies[j].hidden = true;
                    if (enemies_count > 0)
                        enemies_count--;
                }
            }
        }

        //update boss coordinates
        if (!boss.hidden) {
            if (boss_dir == 1) {
                boss.vx = 1;
                boss.vy = 0;
            } else if (boss_dir == 2) {
                boss.vx = -1;
                boss.vy = 0;
            }
            boss.x += boss.vx;
            boss.y += boss.vy;
            if (boss.x + boss.w/2 > SCREEN_W) {
                boss_dir = 2;
            } else if (boss.x - boss.w/2 < 0) {
                boss_dir = 1;
            }
        }

        //show new enemy
        switch (level) {
            case 1:
                if (enemies_count < 10 && now - last_enemy_release_timestamp >= 2) {
                    show_enemy();
                    last_enemy_release_timestamp = now;
                }
                break;
            case 2:
                if (enemies_count < 10 && now - last_enemy_release_timestamp >= 1) {
                    show_enemy();
                    last_enemy_release_timestamp = now;
                }
                break;
            case 3:
                if (enemies_count < 1) {
                    game_change_scene(SCENE_BOSS);
                    boss.hidden = false;
                    enemies_count++;
                }
        }

        //hit boss
        for (j = 0; j < MAX_BULLET; j++) {
            if (!boss.hidden && !bullets[j].hidden) {
                if (check_hit_bullet_boss(boss, bullets[j])) {
                    al_play_sample(enemy_explode, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &enemy_explode_id);
                    boss_hit++;
                    bullets[j].hidden = true;
                    bullets[j].y = 0;
                    score += 10;
                    if (boss_hit > 100) {
                        for (i = 0; i < MAX_BULLET; i++) {
                            bullets[i].hidden = true;
                            bullets[i].y = 0;
                        }
                        al_play_sample(plane_crash, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_crash_id);
                        game_change_scene(SCENE_GAMEOVER);
                    }
                }
            }
        }

        //hit enemy
        int k, l;
        for (k = 0; k < MAX_ENEMY; k++) {
            for (l = 0; l < MAX_BULLET; l++) {
                if (!enemies[k].hidden && !bullets[l].hidden) {
                    //hit enemy
                    if (check_hit_bullet(enemies[k], bullets[l])) {
                        al_play_sample(enemy_explode, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &enemy_explode_id);
                        enemies[k].hidden = true;
                        enemies[k].y = SCREEN_H;
                        enemies_count--;
                        bullets[l].hidden = true;
                        bullets[l].y = 0;
                        score += 10;
                        if (level == 1 && score >= 200) {
                            al_play_sample(level_up, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &level_up_id);
                            level = 2;
                        } else if (level == 2 && score >= 600) {
                            al_play_sample(level_up, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &level_up_id);
                            level = 3;
                        }
                    }
                }
            }
        }

        //die
        if (health <= 0) {
            al_play_sample(plane_crash, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_crash_id);
            lives--;
            game_draw();
            if (lives <= 0) {
                for (i = 0; i < MAX_BULLET; i++) {
                    bullets[i].hidden = true;
                    bullets[i].y = 0;
                }
                game_change_scene(SCENE_GAMEOVER);
            } else {
                game_change_scene(SCENE_DEAD);
                game_draw();
                health = 100;
            }
        }
        //get hit by enemy
        if (now - last_plane_hit_timestamp >= 1) {
            for (k = 0; k < MAX_ENEMY; k++) {
                if (!enemies[k].hidden && check_hit_plane(enemies[k], plane) && health > 0) {
                    al_play_sample(plane_hit, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_hit_id);
                    health = health - 10;
                    game_draw();
                    last_plane_hit_timestamp = now;
                }
            }
        }

        //get hit by boss bullet
        if (now - last_plane_hit_timestamp >= 1) {
            for (k = 0; k < MAX_BULLET; k++) {
                if (!boss_bullet1[k].hidden && check_hit_plane(boss_bullet1[k], plane) && health > 0) {
                    al_play_sample(plane_hit, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_hit_id);
                    health = health - 10;
                    boss_bullet1[k].hidden = true;
                    boss_bullet1[k].y = 0;
                    game_draw();
                    last_plane_hit_timestamp = now;
                }
            }
            for (k = 0; k < MAX_BULLET; k++) {
                if (!boss_bullet2[k].hidden && check_hit_plane(boss_bullet2[k], plane) && health > 0) {
                    al_play_sample(plane_hit, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_hit_id);
                    health = health - 10;
                    boss_bullet2[k].hidden = true;
                    boss_bullet2[k].y = 0;
                    game_draw();
                    last_plane_hit_timestamp = now;
                }
            }
        }

        //get hit by boss
        if (now - last_plane_hit_timestamp >= 1) {
            if (!boss.hidden && check_hit_plane_boss(boss, plane) && health > 0) {
                al_play_sample(plane_hit, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, &plane_hit_id);
                health = health - 20;
                game_draw();
                last_plane_hit_timestamp = now;
            }
        }
	}
}

void game_draw(void) {
	if (active_scene == SCENE_MENU) {
		al_draw_bitmap(main_img_background, 0, 0, 0);
		al_draw_text(font_cerena_48, al_map_rgb(255, 255, 255), SCREEN_W / 2, 50, ALLEGRO_ALIGN_CENTER, "Space Shooter");
		al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "Press enter key to start");
		// [HACKATHON 3-5]
		// TODO: Draw settings images.
		if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W - 48, 10, 38, 38))
			al_draw_bitmap(img_settings, SCREEN_W - 48, 10, 0);
		else
			al_draw_bitmap(img_settings2, SCREEN_W - 48, 10, 0);
	} else if (active_scene == SCENE_START || active_scene == SCENE_BOSS) {
		int i;
		al_draw_bitmap(start_img_background, 0, 0, 0);
		//dashboard
		al_draw_filled_rectangle(0, SCREEN_H-100, SCREEN_W, SCREEN_H, al_map_rgb(59, 68, 75));
		al_draw_rectangle(2, SCREEN_H-98, SCREEN_W-1, SCREEN_H-1, al_map_rgb(166, 123, 91), 3);
		al_draw_text(font_quantico_20, al_map_rgb(255, 255, 255), 10, SCREEN_H-90, 0, "Health");
		al_draw_text(font_quantico_20, al_map_rgb(255, 255, 255), 10, SCREEN_H-50, 0, "Lives");
		if (health > 0)
            al_draw_filled_rectangle(83, SCREEN_H-90, 85+health*2, SCREEN_H-65, al_map_rgb(179, 0, 0));
        for (i = 0; i < lives; i++) {
            al_draw_bitmap(img_life, 83+i*40, SCREEN_H-45, 0);
        }
        al_draw_textf(font_quantico_20, al_map_rgb(255, 255, 255), SCREEN_W-20, SCREEN_H-90, ALLEGRO_ALIGN_RIGHT, "%d", score);
        al_draw_textf(font_quantico_20, al_map_rgb(255, 255, 255), SCREEN_W-20, SCREEN_H-50, ALLEGRO_ALIGN_RIGHT, "Level %d", level);

        //Movable objects
		for (i = 0; i < MAX_BULLET; i++) {
			draw_movable_object(bullets[i]);
			draw_movable_object(boss_bullet1[i]);
			draw_movable_object(boss_bullet2[i]);
		}
		draw_movable_object(plane);
		draw_movable_object(boss);
		for (i = 0; i < MAX_ENEMY; i++)
			draw_movable_object(enemies[i]);
	} else if (active_scene == SCENE_SETTINGS) {
		al_draw_bitmap(main_img_background, 0, 0, 0);
		//back button
		al_draw_filled_rectangle(5, 5, 80, 40, al_map_rgb(59, 68, 75));
		al_draw_rectangle(7, 7, 79, 39, al_map_rgb(166, 123, 91), 3);
		al_draw_text(font_quantico_20, al_map_rgb(255, 255, 255), 20, 10, 0, "Back");
		//change plane
		al_draw_text(font_quantico_28, al_map_rgb(255, 255, 255), 20, 150, 0, "CUSTOMIZE:");
		al_draw_text(font_quantico_28, al_map_rgb(255, 255, 255), 450, 150, 0, "(left/right to change)");
		switch (plane_num) {
            case 0:
                al_draw_bitmap(img_icon_p51, 250, 125, 0);
                al_draw_text(font_quantico_20, al_map_rgb(255, 255, 255), 300, 230, ALLEGRO_ALIGN_CENTER, "P-51");
                break;
            case 1:
                al_draw_bitmap(img_icon_camo, 250, 125, 0);
                al_draw_text(font_quantico_20, al_map_rgb(255, 255, 255), 300, 230, ALLEGRO_ALIGN_CENTER, "CAMO");
                break;
            case 2:
                al_draw_bitmap(img_icon_jet, 250, 125, 0);
                al_draw_text(font_quantico_20, al_map_rgb(255, 255, 255), 300, 230, ALLEGRO_ALIGN_CENTER, "JET");
                break;
		}
		//controls
		al_draw_text(font_quantico_28, al_map_rgb(255, 255, 255), 20, 400, 0, "CONTROLS:");
		al_draw_text(font_quantico_28, al_map_rgb(255, 255, 255), 200, 400, 0, "Move around");
		al_draw_text(font_quantico_28, al_map_rgb(255, 255, 255), 200, 500, 0, "Shoot!!!");
		al_draw_bitmap(img_controls_move, 450, 350, 0);
		al_draw_bitmap(img_controls_shoot, 450, 500, 0);
	} else if (active_scene == SCENE_GAMEOVER) {
	    if (boss_hit > 100) {
            al_draw_bitmap(victory_background, 0, 0, 0);
            al_draw_text(font_paint_48, al_map_rgb(255, 255, 255), SCREEN_W/2, 150, ALLEGRO_ALIGN_CENTER, "YOU WIN!");
            al_draw_textf(font_quantico_28, al_map_rgb(255, 255, 255), SCREEN_W/2, 250, ALLEGRO_ALIGN_CENTER, "YOUR SCORE: %d", score);
            al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W/2, 450, ALLEGRO_ALIGN_CENTER, "press enter to return");
	    } else {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font_paint_48, al_map_rgb(255, 255, 255), SCREEN_W/2, 150, ALLEGRO_ALIGN_CENTER, "GAME OVER");
            al_draw_textf(font_quantico_28, al_map_rgb(255, 255, 255), SCREEN_W/2, 250, ALLEGRO_ALIGN_CENTER, "YOUR SCORE: %d", score);
            al_draw_textf(font_quantico_28, al_map_rgb(255, 255, 255), SCREEN_W/2, 300, ALLEGRO_ALIGN_CENTER, "MAX LEVEL: %d", level);
            al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W/2, 450, ALLEGRO_ALIGN_CENTER, "press enter to return");
	    }
	} else if (active_scene == SCENE_DEAD) {
        al_draw_text(font_quantico_28, al_map_rgb(255, 255, 255), SCREEN_W/2, 150, ALLEGRO_ALIGN_CENTER, "YOU CRASHED!");
        al_draw_textf(font_quantico_28, al_map_rgb(255, 255, 255), SCREEN_W/2, 200, ALLEGRO_ALIGN_CENTER, "REMAINING LIVES: %d", lives);
        al_draw_text(font_pirulen_24, al_map_rgb(166, 123, 91), SCREEN_W/2, 300, ALLEGRO_ALIGN_CENTER, "Press Enter to continue");
        al_draw_bitmap(img_plane_explode, plane.x-plane.w/2, plane.y-plane.h/2, 0);
	}
	al_flip_display();
}

void game_destroy(void) {
	// Destroy everything you have created.
	// Free the memories allocated by malloc or allegro functions.
	// Destroy shared resources.
	al_destroy_font(font_cerena_48);
	al_destroy_font(font_pirulen_24);
	al_destroy_font(font_quantico_20);
	al_destroy_font(font_quantico_28);
	al_destroy_font(font_paint_48);

	/* Menu Scene resources*/
	al_destroy_bitmap(main_img_background);
	al_destroy_sample(main_bgm);
	// [HACKATHON 3-6]
	// TODO: Destroy the 2 settings images.
	// Uncomment and fill in the code below.
	al_destroy_bitmap(img_settings);
	al_destroy_bitmap(img_settings2);

	/* Setting Scene resources */
	al_destroy_bitmap(img_controls_move);
	al_destroy_bitmap(img_controls_shoot);
	al_destroy_bitmap(img_icon_p51);
	al_destroy_bitmap(img_icon_camo);
	al_destroy_bitmap(img_icon_jet);

	/* Start Scene resources*/
	al_destroy_bitmap(start_img_background);
	al_destroy_bitmap(img_plane_p51);
	al_destroy_bitmap(img_plane_camo);
	al_destroy_bitmap(img_plane_jet);
	al_destroy_bitmap(start_img_enemy);
	al_destroy_bitmap(img_boss);
    al_destroy_bitmap(img_life);
	al_destroy_bitmap(img_bullet);
	al_destroy_bitmap(img_plane_explode);
	al_destroy_bitmap(img_boss_bullet);

	al_destroy_sample(start_bgm);
	al_destroy_sample(bullet_fired);
	al_destroy_sample(plane_hit);
	al_destroy_sample(plane_crash);
	al_destroy_sample(enemy_explode);

	/* Game over scene Resources */
	al_destroy_sample(game_over_bgm);

	al_destroy_timer(game_update_timer);
	al_destroy_event_queue(game_event_queue);
	al_destroy_display(game_display);
	free(mouse_state);
}

void game_change_scene(int next_scene) {
	game_log("Change scene from %d to %d", active_scene, next_scene);
	// TODO: Destroy resources initialized when creating scene.
	if (active_scene == SCENE_MENU && next_scene != SCENE_SETTINGS) {
        boss_hit = 0;
		al_stop_sample(&main_bgm_id);
		game_log("stop audio (bgm)");
	} else if (active_scene == SCENE_START && next_scene != SCENE_DEAD) {
		al_stop_sample(&start_bgm_id);
		game_log("stop audio (bgm)");
	} else if (active_scene == SCENE_GAMEOVER) {
	    if (boss_hit > 100) {
            al_stop_sample(&victory_bgm_id);
	    } else {
            al_stop_sample(&game_over_bgm_id);
	    }
	} else if (active_scene == SCENE_BOSS && next_scene != SCENE_DEAD) {
        al_stop_sample(&boss_bgm_id);
        game_log("stop audio (bgm)");
	}

	if (active_scene != SCENE_DEAD && next_scene == SCENE_START) {
        al_play_sample(start_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_id);
        health = 100;
		lives = 3;
		score = 0;
		level = 1;
	} else if (active_scene != SCENE_SETTINGS && next_scene == SCENE_MENU) {
        al_play_sample(main_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id);
	} else if (active_scene != SCENE_DEAD && next_scene == SCENE_BOSS) {
        al_play_sample(boss_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &boss_bgm_id);
	}

	active_scene = next_scene;
	// TODO: Allocate resources before entering scene.
	if (active_scene == SCENE_START || active_scene == SCENE_BOSS) {
		int i;
		switch (plane_num) {
            case 0:
                plane.img = img_plane_p51;
                break;
            case 1:
                plane.img = img_plane_camo;
                break;
            case 2:
                plane.img = img_plane_jet;
                break;
		}
		plane.x = 400;
		plane.y = SCREEN_H-150;
		plane.w = plane.h = 51;
		//boss
		boss.img = img_boss;
        boss.w = 250;
        boss.h = 188;
        boss.x = SCREEN_W/2;
        boss.y = 100;
        boss_dir = 1;
        boss.hidden = true;
        boss_shoot = 0;
		//enemies
		enemies_count = 0;
		for (i = 0; i < MAX_ENEMY; i++) {
			enemies[i].w = 28;
			enemies[i].h = 68;
			enemies[i].img = start_img_enemy;
			enemies[i].vx = 0;
			enemies[i].vy = 1;
			enemies[i].hidden = true;
		}
		// [HACKATHON 2-5-2]
		// TODO: Initialize bullets.

		for (i = 0; i < MAX_BULLET; i++) {
			bullets[i].w = al_get_bitmap_width(img_bullet);
			bullets[i].h = al_get_bitmap_height(img_bullet);
			bullets[i].img = img_bullet;
			bullets[i].vx = 0;
			bullets[i].vy = -3;
			bullets[i].hidden = true;
			//boss
			boss_bullet1[i].w = al_get_bitmap_width(img_boss_bullet);
			boss_bullet1[i].h = al_get_bitmap_height(img_boss_bullet);
			boss_bullet1[i].img = img_boss_bullet;
			boss_bullet1[i].vx = 0;
			boss_bullet1[i].vy = 2;
			boss_bullet1[i].hidden = true;
			//
			boss_bullet2[i].w = al_get_bitmap_width(img_boss_bullet);
			boss_bullet2[i].h = al_get_bitmap_height(img_boss_bullet);
			boss_bullet2[i].img = img_boss_bullet;
			boss_bullet2[i].vx = 0;
			boss_bullet2[i].vy = 2;
			boss_bullet2[i].hidden = true;
		}
	} else if (active_scene == SCENE_GAMEOVER) {
	    if (boss_hit > 100) {
            al_play_sample(victory_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &victory_bgm_id);
	    } else {
            al_play_sample(game_over_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &game_over_bgm_id);
	    }
	}
}

void on_key_down(int keycode) {
	if (active_scene == SCENE_MENU) {
		if (keycode == ALLEGRO_KEY_ENTER)
			game_change_scene(SCENE_START);
	} else if (active_scene == SCENE_DEAD) {
        if (keycode == ALLEGRO_KEY_ENTER) {
            if (level == 3) {
                game_change_scene(SCENE_BOSS);
            } else {
                game_change_scene(SCENE_START);
            }
        }
	} else if (active_scene == SCENE_GAMEOVER) {
        if (keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_MENU);
	} else if (active_scene == SCENE_SETTINGS) {
        if (keycode == ALLEGRO_KEY_ESCAPE) {
            game_change_scene(SCENE_MENU);
        } else if (keycode == ALLEGRO_KEY_LEFT) {
            if (plane_num == 0) {
                plane_num = 3;
            }
            plane_num--;
        } else if (keycode == ALLEGRO_KEY_RIGHT) {
            if (plane_num == 2) {
                plane_num = -1;
            }
            plane_num++;
        }

	}
}

void on_mouse_down(int btn, int x, int y) {
	// [HACKATHON 3-8]
	// TODO: When settings clicked, switch to settings scene.
	// Uncomment and fill in the code below.
	if (active_scene == SCENE_MENU) {
		if (btn == 1) {
			if (pnt_in_rect(x, y, SCREEN_W - 48, 10, 38, 38) == false)
				game_change_scene(SCENE_SETTINGS);
		}
	} else if (active_scene == SCENE_SETTINGS) {
        if (btn == 1) {
            if(in_back_btn(x, y, 5, 5, 80, 40)) {
                game_change_scene(SCENE_MENU);
            }
        }
	}
}

void draw_movable_object(MovableObject obj) {
	if (obj.hidden)
		return;
	al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}

ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
	ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
	if (!loaded_bmp)
		game_abort("failed to load image: %s", filename);
	ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
	ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();

	if (!resized_bmp)
		game_abort("failed to create bitmap when creating resized image: %s", filename);
	al_set_target_bitmap(resized_bmp);
	al_draw_scaled_bitmap(loaded_bmp, 0, 0,
		al_get_bitmap_width(loaded_bmp),
		al_get_bitmap_height(loaded_bmp),
		0, 0, w, h, 0);
	al_set_target_bitmap(prev_target);
	al_destroy_bitmap(loaded_bmp);

	game_log("resized image: %s", filename);

	return resized_bmp;
}

// [HACKATHON 3-3]
// TODO: Define bool pnt_in_rect(int px, int py, int x, int y, int w, int h)
// Uncomment and fill in the code below.
bool pnt_in_rect(int px, int py, int x, int y, int w, int h) {
    if ((px >= x) && (px <= (x + w))
        && (py >= y) && (py <= (y + h)))
	return false;
}

bool in_back_btn(int px, int py, int x, int y, int w, int h) {
    if (px >= x && px <= w && py >= y && py <= h) {
        return true;
    }
    else {
        return false;
    }
}

bool check_hit_bullet(MovableObject enemies, MovableObject bullets) {
    int e_down = enemies.y + enemies.h/2;
    int b_up = bullets.y - bullets.h/2;
    int b_left = bullets.x - bullets.w/2;
    int b_right = bullets.x + bullets.w/2;
    if ((b_up < e_down) && ((b_left < enemies.x) && (b_right > enemies.x))) {
        return true;
    }
    return false;
}

bool check_hit_bullet_boss(MovableObject boss, MovableObject bullets) {
    int e_up = boss.y - boss.h/2;
    int e_down = boss.y + boss.h/2;
    int e_left = boss.x - boss.w/2;
    int e_right = boss.x + boss.w/2;
    int b_up = bullets.y - bullets.h/2;
    int b_left = bullets.x - bullets.w/2;
    int b_right = bullets.x + bullets.w/2;
    if ((e_down >b_up) && (e_right > b_left) && (e_left < b_right)) {
        return true;
    }
    return false;
}

bool check_hit_plane_boss(MovableObject boss, MovableObject plane) {
    int e_up = boss.y - boss.h/2;
    int e_down = boss.y + boss.h/2;
    int e_left = boss.x - boss.w/2;
    int e_right = boss.x + boss.w/2;
    int p_up = plane.y - plane.h/2;
    int p_down = plane.y + plane.h/2;
    int p_left = plane.x - plane.w/2;
    int p_right = plane.x + plane.w/2;
    if ((e_down > p_up) && (e_up < p_down) && ((p_left < e_right && p_right > e_left))) {
        return true;
    }
    return false;
}

bool check_hit_plane(MovableObject enemies, MovableObject plane) {
    int e_up = enemies.y - enemies.h/2;
    int e_down = enemies.y + enemies.h/2;
    int e_left = enemies.x - enemies.w/2;
    int e_right = enemies.x + enemies.w/2;
    int p_up = plane.y - plane.h/2;
    int p_down = plane.y + plane.h/2;
    int p_left = plane.x - plane.w/2;
    int p_right = plane.x + plane.w/2;
    if ((e_down > p_up) && (e_up < p_down) && ((p_left < e_right && p_right > e_left))) {
        return true;
    }
    return false;
}

void show_enemy() {
    int j;
    for (j = 0; j < MAX_ENEMY; j++) {
        if (enemies[j].hidden)
            break;
    }
    if (j == MAX_ENEMY)
        return;
    enemies_count++;
    enemies[j].hidden = false;
    enemies[j].x = enemies[j].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[j].w);
    enemies[j].y = 80;
}


// +=================================================================+
// | Code below is for debugging purpose, it's fine to remove it.    |
// | Deleting the code below and removing all calls to the functions |
// | doesn't affect the game.                                        |
// +=================================================================+

void game_abort(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
	fprintf(stderr, "error occured, exiting after 2 secs");
	// Wait 2 secs before exiting.
	al_rest(2);
	// Force exit program.
	exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
	va_list arg;
	va_start(arg, format);
	game_vlog(format, arg);
	va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
	static bool clear_file = true;
	vprintf(format, arg);
	printf("\n");
	// Write log to file for later debugging.
	FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
	if (pFile) {
		vfprintf(pFile, format, arg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	clear_file = false;
#endif
}
