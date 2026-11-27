#include "raylib.h"
#include "net.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    bool host = argc == 2 && strcmp(argv[1], "host") == 0;
    bool client = argc == 3 && strcmp(argv[1], "join") == 0;

    if (argc != 1 && !host && !client)
    {
        fprintf(stderr, "Uso: pong.exe [host | join IP_DO_ANFITRIAO]\n");
        return 1;
    }
    if ((host || client) && !NetOpen(host, client ? argv[2] : NULL))
    {
        if (host)
            fprintf(stderr, "Nao foi possivel abrir a porta UDP %d.\n", NET_PORT);
        else
            fprintf(stderr, "Nao foi possivel iniciar a rede. Confira o IPv4 do anfitriao.\n");
        return 1;
    }

    const int centerX = SCREEN_WIDTH / 2;
    const int centerY = SCREEN_HEIGHT / 2;
    const int paddleWidth = 20;
    const int paddleSpeed = 8;
    const int player1X = SCREEN_WIDTH / 32;
    const int player2X = SCREEN_WIDTH - player1X - paddleWidth;
    const int maxPaddleY = SCREEN_HEIGHT - PADDLE_HEIGHT;
    const int centerPaddleY = maxPaddleY / 2;
    const int paddleThird = PADDLE_HEIGHT / 3;

    int     player1Y = centerPaddleY;
    int     player2Y = centerPaddleY;

    int     score1 = 0;
    int     score2 = 0;
    
    const float initialBallSpeed = 6.0f;
    const float maxBallSpeed = 14.0f;
    // As proporcoes 4/5 e 3/5 preservam a velocidade total na diagonal.
    const float diagonalX = 0.8f;
    const float diagonalY = 0.6f;
    float   ballX = centerX;
    float   ballY = centerY;
    float   ballSpeed = initialBallSpeed;
    float   ballSpeedX = ballSpeed * diagonalX;
    float   ballSpeedY = ballSpeed * diagonalY;
    bool    gameOver = false;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong LAN - SENAI");
    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        bool connected = true;
        bool player2Up = IsKeyDown(KEY_UP);
        bool player2Down = IsKeyDown(KEY_DOWN);

        if (host)
            connected = NetReceiveInput(GetTime(), &player2Up, &player2Down);

        if (client)
        {
            NetState state = {player1Y, player2Y, (int)ballX, (int)ballY, score1, score2, gameOver};
            NetSendInput(player2Up, player2Down);
            connected = NetReceiveState(GetTime(), &state);
            if (connected)
            {
                player1Y = state.player1Y;
                player2Y = state.player2Y;
                ballX = state.ballX;
                ballY = state.ballY;
                score1 = state.score1;
                score2 = state.score2;
                gameOver = state.gameOver;
            }
        }
        else if (connected && !gameOver)
        {
            if (IsKeyDown(KEY_W) && player1Y > 0)
                player1Y -= paddleSpeed;
            else if (IsKeyDown(KEY_S) && player1Y < maxPaddleY)
                player1Y += paddleSpeed;

            if (player2Up && player2Y > 0)
                player2Y -= paddleSpeed;
            else if (player2Down && player2Y < maxPaddleY)
                player2Y += paddleSpeed;

            if (player1Y < 0)
                player1Y = 0;
            else if (player1Y > maxPaddleY)
                player1Y = maxPaddleY;

            if (player2Y < 0)
                player2Y = 0;
            else if (player2Y > maxPaddleY)
                player2Y = maxPaddleY;

            ballX += ballSpeedX;
            ballY += ballSpeedY;
            if (ballY >= SCREEN_HEIGHT - BALL_RADIUS)
            {
                ballY = SCREEN_HEIGHT - BALL_RADIUS;
                ballSpeedY = -ballSpeedY;
            }
            else if (ballY <= BALL_RADIUS)
            {
                ballY = BALL_RADIUS;
                ballSpeedY = -ballSpeedY;
            }

            Vector2 ballPosition = {ballX, ballY};
            Rectangle paddle1 = {player1X, player1Y, paddleWidth, PADDLE_HEIGHT};
            Rectangle paddle2 = {player2X, player2Y, paddleWidth, PADDLE_HEIGHT};
            bool movingLeft = ballSpeedX < 0.0f;
            Rectangle targetPaddle = movingLeft ? paddle1 : paddle2;

            if (CheckCollisionCircleRec(ballPosition, BALL_RADIUS, targetPaddle))
            {
                ballSpeed *= 1.05f;
                if (ballSpeed > maxBallSpeed)
                    ballSpeed = maxBallSpeed;

                float hitY = ballY - targetPaddle.y;
                ballSpeedY = 0.0f;
                if (hitY < paddleThird)
                    ballSpeedY = -ballSpeed * diagonalY;
                else if (hitY >= 2 * paddleThird)
                    ballSpeedY = ballSpeed * diagonalY;

                ballSpeedX = ballSpeed;
                if (ballSpeedY != 0.0f)
                    ballSpeedX *= diagonalX;

                if (movingLeft)
                    ballX = player1X + paddleWidth + BALL_RADIUS;
                else
                {
                    ballX = player2X - BALL_RADIUS;
                    ballSpeedX = -ballSpeedX;
                }
            }

            bool exitedRight = ballX >= SCREEN_WIDTH + BALL_RADIUS;
            if (exitedRight || ballX <= -BALL_RADIUS)
            {
                if (exitedRight)
                    score1 += 1;
                else
                    score2 += 1;

                ballX = centerX;
                ballY = centerY;
                player1Y = centerPaddleY;
                player2Y = centerPaddleY;
                ballSpeed = initialBallSpeed;
                ballSpeedX = ballSpeed * diagonalX;
                if (exitedRight)
                    ballSpeedX = -ballSpeedX;
                ballSpeedY = ballSpeed * diagonalY;
            }

            if (score1 >= 5 || score2 >= 5)
            {
                gameOver = true;
            }
        }
        else if (connected)
        {
            if (IsKeyPressed(KEY_R))
            {
                gameOver = false;

                score1 = 0;
                score2 = 0;

                ballX = centerX;
                ballY = centerY;
                player1Y = centerPaddleY;
                player2Y = centerPaddleY;
                ballSpeed = initialBallSpeed;
                ballSpeedX = ballSpeed * diagonalX;
                ballSpeedY = ballSpeed * diagonalY;
            }
        }

        if (host && connected)
        {
            NetState state = {player1Y, player2Y, (int)ballX, (int)ballY, score1, score2, gameOver};
            NetSendState(state);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if (!connected)
        {
            const char *message = host ? TextFormat("Aguardando jogador 2 na porta %d...", NET_PORT)
                                       : "Conectando ao anfitriao...";
            DrawText(message, (SCREEN_WIDTH - MeasureText(message, 20)) / 2,
                     centerY - 10, 20, WHITE);
        }
        else if (!gameOver)
        {
            for (int y = 0; y < SCREEN_HEIGHT; y += 25)
                DrawRectangle(centerX - 3, y, 6, 15, GRAY);

            const int scoreFont = 32;
            const int scoreY = SCREEN_HEIGHT / 20;
            const char *scoreText1 = TextFormat("P1: %d", score1);
            const char *scoreText2 = TextFormat("P2: %d", score2);
            DrawText(scoreText1, SCREEN_WIDTH / 3 - MeasureText(scoreText1, scoreFont) / 2,
                     scoreY, scoreFont, WHITE);
            DrawText(scoreText2, SCREEN_WIDTH * 2 / 3 - MeasureText(scoreText2, scoreFont) / 2,
                     scoreY, scoreFont, WHITE);

            DrawCircle((int)ballX, (int)ballY, BALL_RADIUS, WHITE);
            DrawRectangle(player1X, player1Y, paddleWidth, PADDLE_HEIGHT, WHITE);
            DrawRectangle(player2X, player2Y, paddleWidth, PADDLE_HEIGHT, WHITE);
        }
        else
        {
            const char *winner = score1 >= 5 ? "JOGADOR 1 VENCEU!" : "JOGADOR 2 VENCEU!";
            const char *restart = client ? "Aguardando o anfitriao reiniciar"
                                         : "Pressione R para reiniciar";
            DrawText(winner, (SCREEN_WIDTH - MeasureText(winner, 30)) / 2,
                     centerY - 45, 30, GREEN);
            DrawText(restart, (SCREEN_WIDTH - MeasureText(restart, 20)) / 2,
                     centerY + 20, 20, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    if (host || client)
        NetClose();
    return (0);
}
