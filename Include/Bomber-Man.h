// #include SDL_MAIN_HANDLED
// #include <SDL.h>

//_t indica un type per altri compiler
typedef struct vec2
{
    int x;
    int y;
} vec2_t;
// questa struct definisce un alias e crea una struttura

#pragma region PLAYER

typedef struct player
{
    vec2_t position;
    unsigned int lifes_number;
    unsigned int bombs_number;
    unsigned int score_number;
    unsigned int value_speed;
} player_t;

#pragma endregion

#pragma region BOMB

typedef struct bomb
{
    vec2_t position;
    unsigned int value_range;
    unsigned int value_cooldown;
} bomb_t;

#pragma endregion

#pragma region CELL
enum Cell_Type
{
    GROUND = 0,
    WALL,
    UNDESTROYABLE_WALL,
    TELEPORT,
    CELL_TYPE_LAST
};

enum Bonus_Type
{
    NONE = 0,
    SPEED,
    BOMB_x_2,
    BOMB_x_3,
    BONUS_TYPE_LAST
};

typedef struct cell
{
    enum Cell_Type type;
    enum Bonus_Type bonus;
} cell_t;

#pragma endregion

#pragma region ENEMY
enum Enemy_Type
{
    BAT,
    BALLON,
    ENEMY_TYPE_LAST
};

typedef struct Enemy
{
    vec2_t position;
    enum Enemy_Type type;
} enemy_t;

#pragma endregion

#pragma region GAME MODE

typedef struct Game_Mode
{
    unsigned int timer;
} game_Mode_t;

#pragma endregion