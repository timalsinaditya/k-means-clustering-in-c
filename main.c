#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MIN_X -20.0
#define MIN_Y -20.0
#define MAX_X 20.0
#define MAX_Y 20.0

Vector2 project_sample_to_screen(Vector2 sample)
{
    float x = (sample.x - MIN_X ) / ( MAX_X - MIN_X); 
    float y = (sample.y - MIN_Y ) / ( MAX_Y - MIN_Y);
    return CLITERAL(Vector2) { .x = GetScreenWidth()*x, .y =GetScreenHeight()- GetScreenHeight()* y};
}

typedef struct {
    Vector2 *items;
    size_t count;
    size_t capacity;
} Samples;

float rand_float(void) {
    return (float)rand()/RAND_MAX;
}

void generate_cluster(Vector2 center, float radius, size_t count, Samples *samples){
    float count_instant = count+ samples->count;

    samples->items = (Vector2 *)realloc(samples->items,sizeof(Vector2) * (count_instant));

    for(size_t i = samples->count; i < count_instant; ++i){
        float angle = rand_float()*2*PI;
        float mag = rand_float();
        Vector2 sample = {
            .x = center.x + cosf(angle)*mag*radius,
            .y = center.y + sinf(angle)*mag*radius
        };
       samples->items[i]=sample;
     }

    samples->count = count_instant;;
}

int main(){
    srand(time(0));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "kmeans");
    Samples set= {0};

    generate_cluster(CLITERAL(Vector2){0,0}, 15 , 100, &set);
    generate_cluster(CLITERAL(Vector2){MAX_X*0.5f,MAX_Y*0.5f},5, 100, &set);
    generate_cluster(CLITERAL(Vector2){MIN_X*0.5f,MAX_Y*0.5f},5, 100, &set);

    while(!WindowShouldClose()) {

        if(IsKeyPressed(KEY_R)){
            set.count=0;
            generate_cluster(CLITERAL(Vector2){0,0}, 10 , 100, &set);
            generate_cluster(CLITERAL(Vector2){MAX_X*0.5f,MAX_Y*0.5f},5, 100, &set);
            generate_cluster(CLITERAL(Vector2){MIN_X*0.5f,MAX_Y*0.5f},5, 100, &set);
        }

        BeginDrawing();
        ClearBackground(GetColor(0x181818AA));

        for(size_t i = 0; i< set.count; ++i){
            Vector2 it = set.items[i];
            DrawCircleV(project_sample_to_screen(it), 4.0f, RED);
        }

        EndDrawing();
    }
    free(set.items);
    CloseWindow();
    return 0;
}
