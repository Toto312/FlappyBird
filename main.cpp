#include <iostream>
#include <vector>
#include <cmath>

#include <raylib.h>
#include <raymath.h>

static Texture2D numberTex;
static const Vector2 numberSize = {
   12, 18 
};

static Texture2D bgTex;
static const Vector2 bgSize = {
    144, 256
};

#define DAY_BG_REC ((Rectangle){0,0,bgSize.x,bgSize.y});
#define NIGHT_BG_REC ((Rectangle){bgSize.x,0,bgSize.x,bgSize.y});

static Texture2D birdTex;
static const Vector2 birdSize = {
    17, 12
};

static const int birdAnimFPS = 3;

#define YELLOW_BIRD_1 ((Rectangle){0,0,birdSize.x,birdSize.y})
#define YELLOW_BIRD_2 ((Rectangle){birdSize.x,0,birdSize.x,birdSize.y})
#define YELLOW_BIRD_3 ((Rectangle){2*birdSize.x,0,birdSize.x,birdSize.y})
#define BLUE_BIRD_1 ((Rectangle){3*birdSize.x,0,birdSize.x,birdSize.y})
#define BLUE_BIRD_2 ((Rectangle){4*birdSize.x,0,birdSize.x,birdSize.y})
#define BLUE_BIRD_3 ((Rectangle){5*birdSize.x,0,birdSize.x,birdSize.y})
#define RED_BIRD_1 ((Rectangle){0,birdSize.y,birdSize.x,birdSize.y})
#define RED_BIRD_2 ((Rectangle){birdSize.x,birdSize.y,birdSize.x,birdSize.y})
#define RED_BIRD_3 ((Rectangle){2*birdSize.x,birdSize.y,birdSize.x,birdSize.y})

static Texture2D pipeTex;
static const Vector2 pipeSize = {
    26, 256 
};

#define RED_PIPE_DOWN ((Rectangle){pipeSize.x,0,pipeSize.x,pipeSize.y})
#define RED_PIPE_UP ((Rectangle){0,0,pipeSize.x,pipeSize.y})
#define GREEN_PIPE_DOWN ((Rectangle){2*pipeSize.x,0,pipeSize.x,pipeSize.y})
#define GREEN_PIPE_UP ((Rectangle){3*pipeSize.x,0,pipeSize.x,pipeSize.y})

static const Vector2 RESOLUTION = bgSize;
static float scale = 4.f; 

static float OFFSET_START_PIPE = 1000.f;
static float DISTANCE_BETWEEN_PIPES = 125.f * scale;
static float DISTANCE_FROM_PIPES = 100.f;
static const int TOTAL_PIPES = 100;

enum class EntityType {
    Bird,
    Pipe
};

struct Entity {
    EntityType type;
    Texture2D texture;
    Rectangle subTex;
    Rectangle rec;
    Vector2 position;
    float rotation;

    void UpdateRecPos() {
        rec.x = position.x - rec.width / 2.f;
        rec.y = position.y - rec.height / 2.f;
    }

    virtual void Update(float dt) {}

    Rectangle GetCol() {
        Rectangle coll = rec;
        coll.x -= (rec.width)/2.f;
        coll.y -= (rec.height)/2.f;
        return coll;
    }
};

#define PLAYER_SPEED 200.f 

struct Player : public Entity {
    float vel = 0.f;
    float timeClick = 1000.f;
    float maxTimeClick = 0.25f;

    Player() {
        type = EntityType::Bird;
        texture = birdTex;
        subTex = YELLOW_BIRD_1;
        position = {0, 0};
        rec = Rectangle{0,GetRenderHeight() / 2.f, birdSize.x*scale, birdSize.y*scale};
        rotation = 0.f;
    }

    void Update(float dt) override {
        rec.x += PLAYER_SPEED * dt;

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(0)) {
            timeClick = 0;
        }

        if (timeClick <= maxTimeClick) {
            float d = 1-timeClick/maxTimeClick;
            timeClick += dt;
            vel = -d*0.5*d * 1800 * dt;
            rotation = -45 * (d);

            if (d<=0.333) subTex = YELLOW_BIRD_2;
            else if (d<=0.666) subTex = YELLOW_BIRD_3;
            else subTex = YELLOW_BIRD_1; 
        } else {
            vel += 25*dt;
            if (vel > 17) vel = 17;
            float d = vel / 17;
            rotation = 45 * d;
        }

        rec.y += vel;
    }
};

struct Pipe : public Entity {
    bool wasPassed = false;

    Pipe(Rectangle subTex, Vector2 position) {
        this->subTex = subTex;
        this->position = position;
        type = EntityType::Pipe;
        texture = pipeTex;
        rotation = 0.f;
        rec = {0,0,pipeSize.x*scale,pipeSize.y*scale};

        UpdateRecPos();
    }
};

std::vector<Entity*> entities;

void StartWorld() {
    entities.push_back(new Player());

    for (int i = 0; i < 40; i++) {
        Vector2 position = Vector2 {
            i * DISTANCE_BETWEEN_PIPES + OFFSET_START_PIPE,
            (float)GetRandomValue(DISTANCE_FROM_PIPES,GetRenderHeight()-DISTANCE_FROM_PIPES)
        };
        Vector2 upPos = position, downPos = position;
        upPos.y += DISTANCE_FROM_PIPES+(GREEN_PIPE_UP.height*scale);
        downPos.y -= DISTANCE_FROM_PIPES;

        Entity* upPipe = new Pipe(GREEN_PIPE_UP, upPos);
        Entity* downPipe = new Pipe(GREEN_PIPE_DOWN, downPos);

        entities.push_back(upPipe);
        entities.push_back(downPipe);
    }
}

void EndWorld() {
    for (auto* entity : entities) {
        delete entity;
    }
    entities.clear();
}

int score = 0;

void UpdateWorld(float dt) {
    float playerDead = false;

    for (Entity* entity : entities) {
        entity->Update(dt);
        
        float d = entity->rec.x-entities[0]->rec.x;
        if (d<0) d = -d;

        if (d < GetRenderWidth() && d > 0) {
            Rectangle player = entities[0]->GetCol(), curr = entity->GetCol();
            if (CheckCollisionRecs(player,curr)) {
                playerDead = true;
            }
        }
    }

    if (playerDead) {
        EndWorld();
        StartWorld();
        return;
    }

    float positionX = entities[0]->GetCol().x + entities[0]->GetCol().width - DISTANCE_BETWEEN_PIPES;
    score = (positionX) / DISTANCE_BETWEEN_PIPES;
    if (score < 0) score = 0;
}

#include <array>
#include <charconv>

void DrawNumber(int number, float x, float y) {
    const char* str = std::to_string(number).c_str();
    Vector2 pos = (Vector2){x,y};

    while((*str!='\0')) {
        Rectangle rec = {
            numberSize.x*(*str-'0'), 0,
            numberSize.x, numberSize.y
        };  

        Rectangle dst = {
            pos.x, pos.y,
            numberSize.x*scale, numberSize.y*scale
        };

        DrawTexturePro(numberTex, rec, dst, Vector2Zero(), 0.f, WHITE);
        pos.x += numberSize.x;

        str++;
    }
} 

void DrawWorld() {
    Vector2 playerPos = Vector2{entities[0]->rec.x,entities[0]->rec.y};

    Rectangle bgRec = {0, 0, bgSize.x, bgSize.y};
    Rectangle bgDst = {-std::fmod(playerPos.x,(float)GetRenderWidth())+playerPos.x-GetRenderWidth()/2.f, 0, bgSize.x*scale, bgSize.y*scale};

    DrawTexturePro(bgTex, bgRec, bgDst, Vector2Zero(), 0.f, WHITE);
    bgDst.x += bgDst.width;
    DrawTexturePro(bgTex, bgRec, bgDst, Vector2Zero(), 0.f, WHITE);

    for (auto* entity : entities) {
        DrawTexturePro(entity->texture, entity->subTex, entity->rec, (Vector2){entity->rec.width / 2.f, entity->rec.height / 2.f}, entity->rotation, WHITE);
        Rectangle col = entity->GetCol();
        //DrawRectangleRec(col,RED);
    }
}

int main() {
    Vector2 nowRes = Vector2Scale(RESOLUTION, scale);
    InitWindow(static_cast<int>(nowRes.x), static_cast<int>(nowRes.y), "Flappy Bird");
    SetTargetFPS(60);

    bgTex = LoadTexture("bg.png");
    birdTex = LoadTexture("bird.png");
    pipeTex = LoadTexture("pipe.png");
    numberTex = LoadTexture("numbers.png");

    Camera2D camera = Camera2D {
        .target = Vector2{GetRenderWidth()/2.f,0.f},
        .rotation = 0.f,
        .zoom = 1.f,
    };
        
    StartWorld();

    std::string scoreText = "0";

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        UpdateWorld(dt);
        camera.target.x = entities[0]->rec.x - GetRenderWidth()/2.f;

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawWorld();
        EndMode2D();

        Font font = GetFontDefault();

        scoreText = std::to_string(score);
        int width = MeasureText(scoreText.c_str(),80);
        
        //DrawText(scoreText.c_str(),GetRenderWidth()/2.f-width/2.f,0,80,WHITE);
        DrawNumber(score,GetRenderWidth()/2.f-width/2.f,10);

        EndDrawing();
    }

    EndWorld();

    UnloadTexture(numberTex);
    UnloadTexture(pipeTex);
    UnloadTexture(birdTex);
    UnloadTexture(bgTex);

    CloseWindow();
}
