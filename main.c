#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define INITIAL_ENEMY_CAPACITY 10
#define INITIAL_LASER_CAPACITY 10
#define BASE_SPAWN_INTERVAL 1.0f
#define MINIMUM_SPAWN_INTERVAL 0.1f
// game state engine
typedef enum { GAME_RUNNING, GAME_OVER } GameState;

// player defining struct
typedef struct {
  Vector2 player_pos;
  Rectangle player_frame_rec;
} Player;

// enemy defining struct
typedef struct {
  Vector2 enemy_pos;
  Rectangle enemy_frame_rec;
} Enemy;

typedef struct {
  Vector2 laser_pos;
  Rectangle laser_frame_rec;
} Laser;

bool restart_game(Player *player, int *enemy_count, Enemy *enemies,
                  int *score_calc, float *spawn_interval) {
  player->player_pos.x = 300.0f;
  player->player_pos.y = 600.0f;
  *enemy_count = 0;
  *score_calc = 0;
  *spawn_interval = BASE_SPAWN_INTERVAL;

  // Optionally reset the enemies' positions (if needed for your logic)
  for (int i = 0; i < *enemy_count; i++) {
    enemies[i].enemy_pos.x = 0.0f;
    enemies[i].enemy_pos.y = 0.0f;
  }

  return true;
}

int main() {
  const int width = 680;
  const int height = 840;
  int score_calc = 0;
  GameState gameState = GAME_RUNNING;
  InitWindow(width, height, "I am a scam game");

  // Load sprites
  Texture2D spaceship = LoadTexture("./sprites/player.png");
  Texture2D enemy_sprite = LoadTexture("./sprites/red.png");
  Texture2D laser_sprite = LoadTexture("./sprites/laser.png");

  // Player initialization
  Player player = {
      .player_pos = {300.0f, 600.0f},
      .player_frame_rec = {0.0f, 0.0f, spaceship.width, spaceship.height}};

  // Enemy array
  Enemy *enemies = malloc(INITIAL_ENEMY_CAPACITY * sizeof(Enemy));
  int enemy_count = 0;
  int enemy_capacity = INITIAL_ENEMY_CAPACITY;

  // Laser array
  Laser *lasers = malloc(INITIAL_LASER_CAPACITY * sizeof(Laser));
  int laser_count = 0;
  int laser_capacity = INITIAL_LASER_CAPACITY;

  // Timer for spawning enemies
  float spawn_timer = 0.0f;
  float spawn_interval = BASE_SPAWN_INTERVAL;

  // Set target FPS
  SetTargetFPS(120);
  while (!WindowShouldClose()) {

    // Update player frame rectangle position to match player movement
    player.player_frame_rec.x = player.player_pos.x;
    player.player_frame_rec.y = player.player_pos.y;

    bool isDead = false;
    for (int i = 0; i < enemy_count; i++) {
      if (CheckCollisionRecs(
              (Rectangle){player.player_pos.x, player.player_pos.y,
                          player.player_frame_rec.width,
                          player.player_frame_rec.height},
              (Rectangle){enemies[i].enemy_pos.x, enemies[i].enemy_pos.y,
                          enemies[i].enemy_frame_rec.width,
                          enemies[i].enemy_frame_rec.height})) {
        isDead = true;
        break;
      }
    }

    // Update game state based on collision
    if (isDead) {
      gameState = GAME_OVER; // Change game state to GAME_OVER
      if (gameState == GAME_OVER && IsKeyPressed(KEY_R)) {
        restart_game(&player, &enemy_count, enemies, &score_calc,
                     &spawn_interval);
        gameState = GAME_RUNNING;
      }
    }
    if (gameState == GAME_RUNNING) {

      // Update spawn timer
      spawn_timer += GetFrameTime();
      if (spawn_timer >= spawn_interval) {
        // Reset timer
        spawn_timer = 0.0f;

        // Check if we need to resize the enemies array
        if (enemy_count >= enemy_capacity) {
          enemy_capacity *= 2; // double the capacity
          enemies = realloc(enemies, enemy_capacity * sizeof(Enemy));
        }

        // Spawn new enemy
        enemies[enemy_count].enemy_pos = (Vector2){
            (float)(GetRandomValue(0, width - enemy_sprite.width)), 0.0f};
        enemies[enemy_count].enemy_frame_rec =
            (Rectangle){0.0f, 0.0f, enemy_sprite.width, enemy_sprite.height};
        enemy_count++;
      }

      // Move enemies downwards and remove off-screen enemies
      for (int i = 0; i < enemy_count;) {
        enemies[i].enemy_pos.y += 2; // Move enemy down

        if (enemies[i].enemy_pos.y > height) {
          score_calc++;
          enemies[i] = enemies[enemy_count - 1];
          enemy_count--;
        } else {
          i++;
        }
      }

      // Handle keyboard inputs
      if (IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT)) {
        if (player.player_pos.x > 0) {
          player.player_pos.x -= 2;
        }
      }
      if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
        if (player.player_pos.x <= width - player.player_frame_rec.width) {
          player.player_pos.x += 2;
        }
      }

      // Handle laser firing
      if (IsKeyPressed(KEY_SPACE) && laser_count < laser_capacity) {
        if (laser_count >= laser_capacity) {
          laser_capacity *= 2;
          lasers = realloc(lasers, laser_capacity * sizeof(Laser));
        }

        lasers[laser_count].laser_pos =
            (Vector2){player.player_pos.x + player.player_frame_rec.width / 2 -
                          (float)laser_sprite.width / 2,
                      player.player_pos.y};
        lasers[laser_count].laser_frame_rec =
            (Rectangle){0.0f, 0.0f, laser_sprite.width, laser_sprite.height};
        laser_count++;
      }

      // Update laser positions
      for (int i = 0; i < laser_count;) {
        lasers[i].laser_pos.y -= 5;

        // Check if laser is off-screen
        if (lasers[i].laser_pos.y < 0) {
          lasers[i] = lasers[laser_count - 1];
          laser_count--;
        } else {
          i++;
        }
      }
      if (score_calc / 5 > 0) {
        spawn_interval = BASE_SPAWN_INTERVAL - ((float)score_calc / 2) * 0.1f;
        if (spawn_interval < MINIMUM_SPAWN_INTERVAL) {
          spawn_interval = MINIMUM_SPAWN_INTERVAL;
        }
      }
    }
    char score[20];
    sprintf(score, "%d", score_calc);
    // Detect collision

    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(0, 0);
    // check if player is dead
    if (gameState == GAME_OVER) {
      DrawText("You are dead", 300, 500, 36, RED);
      DrawText(score, 500, 0.0, 36, RED);
    } else {

      // Draw Score
      DrawText(score, 600, 0, 16, RED);

      // Draw enemies
      for (int i = 0; i < enemy_count; i++) {
        DrawTextureRec(enemy_sprite, enemies[i].enemy_frame_rec,
                       enemies[i].enemy_pos, RED);
      }

      // Draw lasers
      for (int i = 0; i < laser_count; i++) {
        DrawTextureRec(laser_sprite, lasers[i].laser_frame_rec,
                       lasers[i].laser_pos, WHITE);
      }

      // Draw player
      DrawTextureRec(spaceship, player.player_frame_rec, player.player_pos,
                     BLUE);
    }
    EndDrawing();
  }

  // Free memory
  free(enemies);
  free(lasers);
  UnloadTexture(spaceship);
  UnloadTexture(enemy_sprite);
  UnloadTexture(laser_sprite);

  CloseWindow();
  return 0;
}
