#include "Bomber-Man.h"

#define SDL_MAIN_HANDLED
#include "include_SDL/SDL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#pragma comment(lib, "ws2_32.lib") // Winsock Library
#include <stdio.h>

static void Bomberman_GameModeInit(game_Mode_t *game_mode)
{
    game_mode->timer = 60;
}

static void Bomberman_MapInit()
{
}

static void Bomberman_PlayerInit(player_t *player)
{
    player->position.x = 0;
    player->position.y = 0;
    player->lifes_number = 1;
    player->bombs_number = 1;
    player->score_number = 0;
    player->value_speed = 1;
}

static int Bomberman_GraficInit(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    *window = SDL_CreateWindow("SDL is active!", 100, 100, 512, 512, 0);
    if (!*window)
    {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer)
    {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return -1;
    }

    int width;
    int height;
    int channels;
    unsigned char *pixels = stbi_load("./Assets/basn6a08.png", &width, &height, &channels, 4);
    if (!pixels)
    {
        SDL_Log("Unable to open image");
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return -1;
    }

    SDL_Log("Image width: %d height: %d channels: %d", width, height, channels);

    *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!*texture)
    {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        free(pixels);
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return -1;
    }

    SDL_UpdateTexture(*texture, NULL, pixels, width * 4);
    SDL_SetTextureAlphaMod(*texture, 255);
    SDL_SetTextureBlendMode(*texture, SDL_BLENDMODE_BLEND);
    free(pixels);
    return 0;
}

int Bomberman_InitSocket()
{
#ifdef _WIN32
    // this part is only required on Windows: it initializes the Winsock2 dll
    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data))
    {
        printf("unable to initialize winsock2 \n");
        return -1;
    }
#endif
    int socket_value = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_value < 0)
    {
        printf("unable to initialize the UDP socket \n");
        return -1;
    }

    struct sockaddr_in sin;
    inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr); // this will create a big endian 32 bit address
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999); // converts 9999 to big endian
    if (bind(socket_value, (struct sockaddr *)&sin, sizeof(sin)))
    {
        printf("unable to bind the UDP socket \n");
        return -1;
    }
    return socket_value;
}

int *Bomberman_ExtractCoordinates(char *input_str)
{
    int *pos = malloc(sizeof(int));

    char *next_token;
    pos[0] = atoi(strtok_s(input_str, " ", &next_token));
    pos[1] = atoi(strtok_s(NULL, " ", &next_token));

    return pos;
}

void Bomberman_GetSocketInformation(int socket_value, player_t *player)
{
    char buffer[4096];
    struct sockaddr_in sender_in;
    int sender_in_size = sizeof(sender_in);
    int len = recvfrom(socket_value, buffer, 4096, 0, (struct sockaddr *)&sender_in, &sender_in_size);
    if (len > 0)
    {
        char addr_as_string[64];
        inet_ntop(AF_INET, &sender_in.sin_addr, addr_as_string, 64);

        printf("received %d bytes from %s:%d\n", len, addr_as_string, ntohs(sender_in.sin_port));

        int *pos = Bomberman_ExtractCoordinates(buffer);

        player->position.x = pos[0];
        player->position.y = pos[1];
        free(pos);
    }
}

int Bomberman_SetNb(int socket_value)
{
#ifdef _WIN32
    unsigned long nb_mode = 1;
    return ioctlsocket(socket_value, FIONBIO, &nb_mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0)
        return flags;
    flags |= O_NONBLOCK;
    return fcntl(s, F_SETFL, flags);
#endif
}

int main(int argc, char **argv)
{
    int socket = Bomberman_InitSocket();
    game_Mode_t game_mode;
    cell_t map[64 * 64];
    player_t player;
    int running = 1;
    float time_delta = 0;
    Uint32 time_prev = 0;
    Uint32 time_curr = 0;
    Uint32 time_frequency = SDL_GetPerformanceFrequency();
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    Bomberman_SetNb(socket);

    Bomberman_GameModeInit(&game_mode);

    Bomberman_MapInit();

    Bomberman_PlayerInit(&player);

    if (Bomberman_GraficInit(&window, &renderer, &texture))
        return -1;

    while (running)
    {
        Bomberman_GetSocketInformation(socket, &player);
        SDL_Event event;
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT)
                running = 0;

        time_prev = time_curr;
        time_curr = SDL_GetPerformanceCounter();
        time_delta = (time_curr - time_prev) / (float)time_frequency;

        SDL_PumpEvents();
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        player.position.x += keys[SDL_SCANCODE_RIGHT];
        player.position.x -= keys[SDL_SCANCODE_LEFT];
        player.position.y += keys[SDL_SCANCODE_DOWN];
        player.position.y -= keys[SDL_SCANCODE_UP];

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect target_rect = {player.position.x, player.position.y, 32, 32};
        SDL_RenderCopy(renderer, texture, NULL, &target_rect);
        SDL_RenderPresent(renderer);
    }

    return 0;
}