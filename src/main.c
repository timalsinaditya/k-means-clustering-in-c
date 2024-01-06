#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <raymath.h>

#define MIN_X -20.0
#define MIN_Y -20.0
#define MAX_X 20.0
#define MAX_Y 20.0

#define SAMPLE_RADIUS 4.0f
#define MEAN_RADIUS (2*SAMPLE_RADIUS)
#define K 4

typedef struct {
    Vector2 *items;
    size_t count;
    size_t capacity;
} Samples;

static Samples set = {0};
static Samples cluster[K] = {0};
static Vector2 means[K] = {0};
Vector2 centroid = {0};
static Color raylib_colors[] = {YELLOW, PINK, WHITE, RED, BLUE, BROWN };
static size_t colors_count = sizeof(raylib_colors) / sizeof(raylib_colors[0]);

static inline float rand_float(void) {
    return (float)rand()/RAND_MAX;
}

static inline float lerpf(float random, float min, float max)
{
    return(random*(max-min)+min);
}

Vector2 project_sample_to_screen(Vector2 sample)
{
    float x = (sample.x - MIN_X ) / ( MAX_X - MIN_X); 
    float y = (sample.y - MIN_Y ) / ( MAX_Y - MIN_Y);
    return CLITERAL(Vector2) { .x = GetScreenWidth()*x, .y =GetScreenHeight()- GetScreenHeight()* y};
}

static void generate_set(Vector2 center, float radius, size_t count, Samples *samples){
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

void generate_state(void)
{
    set.count =0;
    generate_set(CLITERAL(Vector2){0,0}, 15 , 200, &set);
    generate_set(CLITERAL(Vector2){MAX_X*0.5f,MAX_Y*0.5f},10, 200, &set);
    generate_set(CLITERAL(Vector2){MIN_X*0.5f,MAX_Y*0.5f},5, 200, &set);

    for(size_t i = 0; i<K; ++i)
    {
        means[i].x = lerpf(rand_float(), MIN_X, MAX_X);
        means[i].y = lerpf(rand_float(), MIN_Y, MAX_Y);
    }
}

void recluster_state(void)
{ 
    for(size_t i = 0; i<K; ++i)
    {
        cluster[i].count = 0;
    }
    for(size_t i = 0; i < set.count; ++i)
    {
        Vector2 p = set.items[i];
        int k = -1;
        float s = FLT_MAX;
        for(size_t j = 0; j < K; ++j)
        {
            Vector2 m = means[j];
            float sm = Vector2LengthSqr(Vector2Subtract(p,m));
            if(sm < s)
            {
                k = j;
                s = sm;
            }
        }

        cluster[k].count += 1;
        cluster[k].items = (Vector2 *)realloc(cluster[k].items,sizeof(Vector2)*cluster[k].count);
        cluster[k].items[cluster[k].count-1] = p;
    }

}

void update_means()
{
    for(size_t i = 0; i<K; ++i){
        if(cluster[i].count>0){
            means[i] = Vector2Zero();
            for(size_t j =0; j<cluster[i].count; ++j){
                means[i]=Vector2Add(means[i], cluster[i].items[j]);
            }
            means[i].x /= cluster[i].count;
            means[i].y /= cluster[i].count;
        } else {
        means[i].x = lerpf(rand_float(), MIN_X, MAX_X);
        means[i].y = lerpf(rand_float(), MIN_Y, MAX_Y);
        }
    }
}

void calculate_centroid()
{
      Vector2 temp = {0};
      for(size_t i = 0; i<K; ++i)
       {
           temp.x += means[i].x;
           temp.y += means[i].y;
       }
      centroid.x = temp.x / K;
      centroid.y = temp.y / K;
}

int main(){
    srand(time(0));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "kmeans");
    
    generate_state();
    recluster_state();
    calculate_centroid();

    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_R)){
            generate_state();
            recluster_state();
            calculate_centroid();
        }

        assert(K <= colors_count);
        
        if(IsKeyPressed(KEY_SPACE)) {
            update_means();
            recluster_state();
            calculate_centroid();
        }

        BeginDrawing();
        ClearBackground(GetColor(0x181818AA));
        /*
        for(size_t i = 0; i< set.count; ++i){
            Vector2 it = set.items[i];
            DrawCircleV(project_sample_to_screen(it), SAMPLE_RADIUS, RED);
        }
        */

        for(size_t i = 0; i< K; ++i){
            Color color = raylib_colors[i%colors_count];
            Vector2 it = means[i];
            DrawCircleV(project_sample_to_screen(it), MEAN_RADIUS, color);
            
            for(size_t j =0; j<cluster[i].count; ++j){
                Vector2 it = cluster[i].items[j];
                DrawCircleV(project_sample_to_screen(it), SAMPLE_RADIUS, color);
                DrawCircleV(project_sample_to_screen(centroid), 10, RED);
            }
        }

        EndDrawing();
    }
    free(set.items);
    CloseWindow();
    return 0;
}
