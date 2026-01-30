#include <iostream>
#include <vector>
#include <cmath>

#include <raylib.h>
#include <raymath.h>

static Texture2D startTex;

static Texture2D buttonTex;
static Vector2 buttonSize = {
    40, 14
};

#define OK_BUTTON ((Rectangle){0,0,buttonSize.x,buttonSize.y})

static Texture2D dieMenuTex;

static Texture2D numberTex;
static const Vector2 numberSize = {
   12, 18 
};

static Texture2D numberMediumTex;
static const Vector2 numberMediumSize = {
   9, 12 
};

static Texture2D numberSmallTex;
static const Vector2 numberSmallSize = {
   6, 7 
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

int score = 0;
int best = 0;
bool playerDead = false;
bool paused = true;

Sound jump;
Sound dead;
Sound newScore;

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

    virtual Rectangle GetCol() {
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
    bool super = false;

    Player() {
        type = EntityType::Bird;
        texture = birdTex;
        subTex = YELLOW_BIRD_1;
        position = {0, 0};
        rec = Rectangle{0,GetRenderHeight() / 2.f, birdSize.x*scale, birdSize.y*scale};
        rotation = 0.f;
    }

    void Update(float dt) override {
        if (score < 100) rec.x += PLAYER_SPEED * dt;
        else rec.x += PLAYER_SPEED * 1.8f * dt;

        if (super) rec.x += PLAYER_SPEED * 30.f * dt;

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsMouseButtonPressed(0)) {
            timeClick = 0;
            PlaySound(jump);
        }

        if (IsKeyPressed(KEY_P)) {
            super = !super;
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

    Rectangle GetCol() override {
        Rectangle col = rec;
        col.width *= 0.5;
        col.height *= 0.75;
        col.x -= (col.width)/2.f;
        col.y -= (col.height)/2.f;
        return col;
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

static float OFFSET_START_PIPE = 500.f;
static float DISTANCE_BETWEEN_PIPES = 125.f * scale;
static float DISTANCE_FROM_PIPES = 100.f;
static const int TOTAL_PIPES = 200;

void StartWorld() {
    entities.push_back(new Player());

    for (int i = 0; i < TOTAL_PIPES; i++) {
        Vector2 position = Vector2 {
            i * DISTANCE_BETWEEN_PIPES + OFFSET_START_PIPE,
            (float)GetRandomValue(DISTANCE_FROM_PIPES,GetRenderHeight()-DISTANCE_FROM_PIPES)
        };

        std::cout << i << ", " << position.x << "\n";

        float distance = DISTANCE_FROM_PIPES;
        if (i >= 100) {
            distance *= 0.8f;
        }

        Vector2 upPos = position, downPos = position;
        upPos.y += distance+(GREEN_PIPE_UP.height*scale);
        downPos.y -= distance;

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

int lastScore = 0;

void UpdateWorld(float dt) {
    if (paused) {
        return;
    }

    for (Entity* entity : entities) {
        entity->Update(dt);
        
        float d = entity->rec.x-entities[0]->rec.x;
        if (d<0) d = -d;

        if (d < GetRenderWidth() && d > 0) {
            Rectangle player = entities[0]->GetCol(), curr = entity->GetCol();
            if (CheckCollisionRecs(player,curr) && !((Player*)entities[0])->super) {
                playerDead = true;
            }
        }
    }

    Rectangle col = entities[0]->GetCol();
    if (col.y + col.height < 0 ||
        col.y - col.height*2 > GetRenderHeight() && !((Player*)entities[0])->super) {
        playerDead = true;
    }

    float positionX = entities[0]->GetCol().x + entities[0]->GetCol().width * 2;

    score = (positionX - OFFSET_START_PIPE) / DISTANCE_BETWEEN_PIPES + 1;
    if (lastScore != score && score != 0) {
        lastScore = score;
        PlaySound(newScore);
    }

    if (playerDead) {
        PlaySound(dead);
    }

    if (playerDead && best < score) {
        best = score;
    }
}

#include <array>
#include <charconv>

enum {
    FONT_SIZE_BIG,
    FONT_SIZE_MEDIUM,
    FONT_SIZE_SMALL
};

typedef int FontSize;

Vector2 GetFontSize(FontSize size) {
    Vector2 numSize;
    if (size == FONT_SIZE_BIG) {
        numSize = numberSize;
    } else if (size == FONT_SIZE_MEDIUM) {
        numSize = numberMediumSize;
    } else {
        numSize = numberSmallSize;
    }
    return numSize;
}

void DrawNumber(int number, float x, float y, FontSize size) {
    const char* str = std::to_string(number).c_str();
    Vector2 pos = (Vector2){x,y};

    while((*str!='\0')) {
        Texture2D tex;
        Vector2 numSize;
        if (size == FONT_SIZE_BIG) {
            tex = numberTex;
            numSize = numberSize;
        } else if (size == FONT_SIZE_MEDIUM) {
            tex = numberMediumTex;
            numSize = numberMediumSize;
        } else {
            tex = numberSmallTex;
            numSize = numberSmallSize;
        }

        Rectangle rec = {
            numSize.x*(*str-'0'), 0,
            numSize.x, numSize.y
        };  

        Rectangle dst = {
            pos.x, pos.y,
            numSize.x*scale, numSize.y*scale
        };

        DrawTexturePro(tex, rec, dst, Vector2Zero(), 0.f, WHITE);

        if (number != FONT_SIZE_BIG) pos.x += numSize.x*scale*0.8;
        else pos.x += numSize.x*scale;

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
        /*Rectangle col = entity->GetCol();
        DrawRectangleRec(col,RED);*/
    }
}

int main() {
    Vector2 nowRes = Vector2Scale(RESOLUTION, scale);
    InitWindow(static_cast<int>(nowRes.x), static_cast<int>(nowRes.y), "Flappy Bird");
    SetTargetFPS(60);

    InitAudioDevice();

    bgTex = LoadTexture("bg.png");
    birdTex = LoadTexture("bird.png");
    pipeTex = LoadTexture("pipe.png");
    numberTex = LoadTexture("numbers.png");
    dieMenuTex = LoadTexture("die-menu.png");
    numberMediumTex = LoadTexture("numbers-medium.png");
    numberSmallTex = LoadTexture("numbers-small.png");
    buttonTex = LoadTexture("button.png");
    startTex = LoadTexture("start.png");

    jump = LoadSound("jump.wav");
    dead = LoadSound("dead.wav");
    newScore = LoadSound("coin.wav");

    Camera2D camera = Camera2D {
        .target = Vector2{GetRenderWidth()/2.f,0.f},
        .rotation = 0.f,
        .zoom = 1.f,
    };
        
    StartWorld();

    std::string scoreText = "0";

    bool did100Screen = false;
    bool did1stMessage = false;
    float firstMessageTime = 0;
    const float FIRST_MESSAGE_WAIT = 1.5;
    bool did2ndMessage = false;
    float secondMessageTime = 0;
    const float SECOND_MESSAGE_WAIT = 0.5;

    float wonTimer = 0;
    bool won = false;
    const float WON_WAIT = 5;

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        Vector2 dieMenuPos = (Vector2) {
            GetRenderWidth() / 2.f - (dieMenuTex.width * scale) / 2.f,
            GetRenderHeight() / 2.f - (dieMenuTex.height * scale) / 2.f,
        };
        Rectangle OKButton = {
            dieMenuPos.x+67*scale,
            dieMenuPos.y+25*scale,
            buttonSize.x*scale,
            buttonSize.y*scale
        };

        if (paused && !playerDead) {
            if(GetKeyPressed() != 0 || IsMouseButtonPressed(0)) paused = false; 
        }

        camera.target.x = entities[0]->rec.x - GetRenderWidth()/2.f;
        if (!paused && !playerDead && !won) {
            if (score != 100 || did100Screen)
                UpdateWorld(dt);
        }

        if (playerDead) {
            if (CheckCollisionPointRec(GetMousePosition(),OKButton) && IsMouseButtonPressed(0)) {
                playerDead = false;
                paused = true;
                EndWorld();
                StartWorld();
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawWorld();
        EndMode2D();

        Font font = GetFontDefault();

        if (!playerDead && !paused) {
            scoreText = std::to_string(score);
            int width = MeasureText(scoreText.c_str(),80);
            
            DrawNumber(score,GetRenderWidth()/2.f-width/2.f,10,FONT_SIZE_BIG);

            if (score == 100 && !did100Screen) {
                paused = true;
            }

        } if (!playerDead && paused) {
            if (score == 100 && !did100Screen) {
                DrawRectangle(0,0,GetRenderWidth(),GetRenderHeight(),BLACK);

                if (!did1stMessage) {
                    firstMessageTime += dt;
                    Vector2 size = MeasureTextEx(font, "YOU WIN!", 30, 1);
                    DrawTextEx(font, "YOU WIN!", Vector2{GetRenderWidth() / 2.f - size.x / 2.f, GetRenderHeight() / 2.f - size.y / 2.f}, 30, 1, WHITE);
                }

                did1stMessage = firstMessageTime >= FIRST_MESSAGE_WAIT;

                if (did1stMessage && !did2ndMessage) {
                    secondMessageTime += dt;
                    Vector2 size = MeasureTextEx(font, "(for now)", 20, 1);
                    DrawTextEx(font, "(for now)", Vector2{GetRenderWidth() / 2.f - size.x / 2.f, GetRenderHeight() / 2.f - size.y / 2.f}, 20, 1, WHITE);
                }

                did2ndMessage = secondMessageTime >= SECOND_MESSAGE_WAIT;
                
                if (did1stMessage && did2ndMessage) {
                    paused = false;
                    did100Screen = true;
                }
            }  else {
                Color bg = (Color){0,0,0,126};
                DrawRectangle(0,0,GetRenderWidth(),GetRenderHeight(),bg);

                Vector2 startPos = {
                    GetRenderWidth() / 2.f - (startTex.width * scale) / 2.f,
                    GetRenderHeight() * 0.5725 - (startTex.height * scale) / 2.f
                };

                DrawTextureEx(startTex, startPos, 0.f, scale, WHITE);
            }

        } if (playerDead) {
            Color bg = (Color){0,0,0,126};
            DrawRectangle(0,0,GetRenderWidth(),GetRenderHeight(),bg);

            DrawTextureEx(dieMenuTex, dieMenuPos, 0.f, scale, WHITE);

            int digScore = 0;
            if (score < 10) digScore = 1;
            else if (score < 100) digScore = 2;
            else digScore = 3;
            float x = (dieMenuPos.x + 37 * scale) - (digScore * GetFontSize(FONT_SIZE_MEDIUM).x * scale) / 2.f; 
            DrawNumber(score,x,dieMenuPos.y+19*scale,FONT_SIZE_MEDIUM);

            int digBest = 0;
            if (best < 10) digBest = 1;
            else if (best < 100) digBest = 2;
            else digBest = 3;
            float xBest = (dieMenuPos.x + 37 * scale) - (digBest * GetFontSize(FONT_SIZE_MEDIUM).x * scale) / 2.f; 
            DrawNumber(best,xBest,dieMenuPos.y+41*scale,FONT_SIZE_MEDIUM);

            DrawTexturePro(buttonTex, OK_BUTTON, OKButton, Vector2Zero(), 0.f, WHITE);
        }

        if (score == 200 && wonTimer < WON_WAIT) {
            DrawRectangle(0,0,GetRenderWidth(),GetRenderHeight(),BLACK);
            if (wonTimer < WON_WAIT) {
                wonTimer += dt;
                paused = true;
                won = true;
                Vector2 size = MeasureTextEx(font, "YOU WON! (for real this time)", 30, 1);
                DrawTextEx(font, "YOU WON! (for real this time)", Vector2{GetRenderWidth() / 2.f - size.x / 2.f, GetRenderHeight() / 2.f - size.y / 2.f}, 30, 1, WHITE);
            }

            if (wonTimer >= WON_WAIT){
                std::puts("yea");
                playerDead = false;
                paused = true;
                won = false;
                best = TOTAL_PIPES; 
                EndWorld();
                StartWorld();
            }
        }

        EndDrawing();
    }

    EndWorld();

    UnloadTexture(startTex);
    UnloadTexture(buttonTex);
    UnloadTexture(numberSmallTex);
    UnloadTexture(numberMediumTex);
    UnloadTexture(dieMenuTex);
    UnloadTexture(numberTex);
    UnloadTexture(pipeTex);
    UnloadTexture(birdTex);
    UnloadTexture(bgTex);

    UnloadSound(dead);
    UnloadSound(jump);
    UnloadSound(newScore);

    CloseAudioDevice();
    CloseWindow();
}
