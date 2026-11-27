#ifndef PONG_NET_H
#define PONG_NET_H

#include <stdbool.h>

#define NET_PORT 5000

// Geometria usada pelo jogo e pela validação dos pacotes.
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define PADDLE_HEIGHT (SCREEN_HEIGHT / 6)
#define BALL_RADIUS 8

typedef struct {
    int player1Y, player2Y;
    int ballX, ballY;
    int score1, score2;
    bool gameOver;
} NetState;

bool NetOpen(bool host, const char *hostIp);
bool NetReceiveInput(double now, bool *up, bool *down);
void NetSendInput(bool up, bool down);
bool NetReceiveState(double now, NetState *state);
void NetSendState(NetState state);
void NetClose(void);

#endif
