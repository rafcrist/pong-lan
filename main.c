#include "raylib.h"

int main(void)
{
    int     player1Y = 175;
    int     player2Y = 175;

    int     score1 = 0;
    int     score2 = 0;
    
    int     ballX = 400;
    int     ballY = 225;
    int     ballSpeedX = 4;
    int     ballSpeedY = 3;
    bool    gameOver = false;

    InitWindow(800, 450, "Pong LAN - SENAI");
    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        if (!gameOver)
        {
            if (IsKeyDown(KEY_W) && player1Y > 0)
                player1Y -= 5;
            else if (IsKeyDown(KEY_S) && player1Y < 350)
                player1Y += 5;

            if (IsKeyDown(KEY_UP) && player2Y > 0)
                player2Y -= 5;
            else if (IsKeyDown(KEY_DOWN) && player2Y < 350)
                player2Y += 5;

            ballX += ballSpeedX;
            ballY += ballSpeedY;
            if (ballY >= 442)
            {
                ballY = 442;
                ballSpeedY = -ballSpeedY;
            }
            else if (ballY <= 8)
            {
                ballY = 8;
                ballSpeedY = -ballSpeedY;
            }

            Vector2 ballPosition = {ballX, ballY};
            Rectangle paddle1 = {30, player1Y, 15, 100};
            Rectangle paddle2 = {760, player2Y, 15, 100};

            if (CheckCollisionCircleRec(ballPosition, 8, paddle1) && ballSpeedX < 0)
            {
                ballX = 53;

                int paddleCenterY = player1Y + 100 / 2;
                int difference = ballY - paddleCenterY;
                ballSpeedX = -ballSpeedX;
                ballSpeedY = difference / 10;
            }
            if (CheckCollisionCircleRec(ballPosition, 8, paddle2) && ballSpeedX > 0)
            {
                ballX = 752;

                int paddleCenterY = player2Y + 100 / 2;
                int difference = ballY - paddleCenterY;
                ballSpeedX = -ballSpeedX;
                ballSpeedY = difference / 10;
            }
            
            if (ballX >= 808)
            {
                ballX = 400;
                ballY = 225;
                player1Y = 175;
                player2Y = 175;

                score1 += 1;
                ballSpeedY = 3;
                ballSpeedX = -4;
            }
            else if (ballX <= -8)
            {
                ballX = 400;
                ballY = 225;
                player1Y = 175;
                player2Y = 175;

                score2 += 1;
                ballSpeedY = 3;  
                ballSpeedX = 4;    
            }

            if (score1 >= 5 || score2 >= 5)
            {
                gameOver = true;
            }
        }
        else
        {
            if(IsKeyPressed(KEY_R))
            {
                gameOver = false;

                score1 = 0;
                score2 = 0;

                ballX = 400;
                ballY = 225;

                player1Y = 175;
                player2Y = 175;
                ballSpeedX = 4;
                ballSpeedY = 3;
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);
        if (!gameOver)
        {
            for (int y = 0; y < 450; y += 25)
                DrawRectangle(397, y, 6, 15, GRAY);
            DrawText(TextFormat("P1: %d", score1), 230, 20, 25, WHITE);
            DrawText(TextFormat("P2: %d", score2), 490, 20, 25, WHITE);
            DrawCircle(ballX, ballY, 8, WHITE);
            DrawRectangle(30, player1Y, 15, 100, WHITE);
            DrawRectangle(760, player2Y, 15, 100, WHITE);
        }
        else
        {
            int length;

            if (score1 >= 5)
            {
                length = MeasureText("JOGADOR 1 VENCEU!", 30);
                DrawText("JOGADOR 1 VENCEU!",
                    (800 - length) / 2, 175, 30, GREEN);
            }
            else if (score2 >= 5)
            {
                length = MeasureText("JOGADOR 2 VENCEU!", 30);
                DrawText("JOGADOR 2 VENCEU!",
                    (800 - length) / 2, 175, 30, GREEN);
            }

            length = MeasureText("Pressione R para reiniciar", 20);
            DrawText("Pressione R para reiniciar",
                (800 - length) / 2, 240, 20, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return (0);
}
