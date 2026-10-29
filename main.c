#include "raylib.h"

int main(void)
{
    int player1Y = 175;
    int player2Y = 175;

    int score1 = 0;
    int score2 = 0;
    
    int ballX = 400;
    int ballY = 225;
    int ballSpeedX = 4;
    int ballSpeedY = 3;

    InitWindow(800, 450, "Pong LAN - SENAI");
    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        Rectangle paddle1 = {30, player1Y, 15, 100};
        Rectangle paddle2 = {760, player2Y, 15, 100};
        Vector2 ballPosition = {ballX, ballY};

        if (IsKeyDown(KEY_W) && player1Y > 0)
            player1Y -= 5;
        else if (IsKeyDown(KEY_S) && player1Y < 350)
            player1Y += 5;

        if (IsKeyDown(KEY_UP) && player2Y > 0)
            player2Y -= 5;
        else if (IsKeyDown(KEY_DOWN) && player2Y < 350)
            player2Y += 5;

        if (CheckCollisionCircleRec(ballPosition, 8, paddle1) && ballSpeedX < 0)
            ballSpeedX = -ballSpeedX;
        if (CheckCollisionCircleRec(ballPosition, 8, paddle2) && ballSpeedX > 0)
            ballSpeedX = -ballSpeedX;

        ballX += ballSpeedX;
        if (ballX >= 792)
        {
            ballX = 400;
            ballY = 225;
            player1Y = 175;
            player2Y = 175;

            score1 += 1;
        }
        else if (ballX <= 8)
        {
            ballX = 400;
            ballY = 225;
            player1Y = 175;
            player2Y = 175;

            score2 += 1;        
        }

        ballY += ballSpeedY;
        if (ballY >= 442)
            ballSpeedY = -ballSpeedY;
        else if (ballY <= 8)
            ballSpeedY = -ballSpeedY;
        

        BeginDrawing();
        ClearBackground(BLACK);
        
        DrawText(TextFormat("P1: %d", score1), 230, 20, 25, WHITE);
        DrawText(TextFormat("P2: %d", score2), 490, 20, 25, WHITE);
        DrawCircle(ballX, ballY, 8, WHITE);
        DrawRectangle(30, player1Y, 15, 100, WHITE);
        DrawRectangle(760, player2Y, 15, 100, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return (0);
}
