#include "raylib.h"
#include "physics.c"
#include <stdio.h>

// 图形界面配置
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define WINDOW_TITLE "Physics Simulation with Raylib"

// 缩放因子，用于将物理坐标转换为屏幕坐标
#define SCALE_FACTOR 50.0f
#define ORIGIN_X (SCREEN_WIDTH / 2)
#define ORIGIN_Y (SCREEN_HEIGHT / 2)

// 将物理坐标转换为屏幕坐标
Vector2 physics_to_screen(vector phys_pos) {
    return (Vector2){
        ORIGIN_X + phys_pos.x * SCALE_FACTOR,
        ORIGIN_Y - phys_pos.y * SCALE_FACTOR // Y轴翻转，因为屏幕坐标原点在左上角
    };
}

void init_graphics(int width, int height, const char* title) {
    InitWindow(width, height, title);
    SetTargetFPS(60);
}

void run_graphics_loop(object* obj) {
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;
    
    // 主循环
    while (!WindowShouldClose()) {
        // 物理更新
        float delta_time = GetFrameTime();
        newton(obj, delta_time);
        gravity(obj, delta_time);
        
        // 更新位置
        obj->position.x += obj->v.x * delta_time;
        obj->position.y += obj->v.y * delta_time;
        
        // 渲染
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        BeginMode2D(camera);
        
        // 绘制坐标轴
        DrawLine(0, ORIGIN_Y, SCREEN_WIDTH, ORIGIN_Y, GRAY);
        DrawLine(ORIGIN_X, SCREEN_HEIGHT, ORIGIN_X, 0, GRAY);
        
        // 绘制物体
        if (obj->point_count > 0) {
            // 计算物体在屏幕上的位置
            Vector2 screen_points[obj->point_count];
            for (int i = 0; i < obj->point_count; i++) {
                vector world_point = {
                    obj->position.x + obj->point[i].x,
                    obj->position.y + obj->point[i].y
                };
                screen_points[i] = physics_to_screen(world_point);
            }
            
            // 绘制多边形
            for (int i = 0; i < obj->point_count; i++) {
                int next_i = (i + 1) % obj->point_count;
                DrawLineV(screen_points[i], screen_points[next_i], BLUE);
            }
            
            // 绘制质心
            Vector2 center = physics_to_screen(obj->position);
            DrawCircleV(center, 5, RED);
            
            // 绘制速度向量
            Vector2 velocity_end = physics_to_screen((vector){
                obj->position.x + obj->v.x * 0.2,
                obj->position.y + obj->v.y * 0.2
            });
            DrawLineV(center, velocity_end, RED);
        }
        
        EndMode2D();
        
        // 显示信息
        DrawText(TextFormat("Position: (%.2f, %.2f)", obj->position.x, obj->position.y), 10, 10, 20, BLACK);
        DrawText(TextFormat("Velocity: (%.2f, %.2f)", obj->v.x, obj->v.y), 10, 40, 20, BLACK);
        DrawText(TextFormat("Force: (%.2f, %.2f)", obj->F.x, obj->F.y), 10, 70, 20, BLACK);
        
        EndDrawing();
    }
}

void cleanup_graphics() {
    CloseWindow();
}

int main() {
    object obj;
    init(&obj);

    init_graphics(SCREEN_HEIGHT, SCREEN_HEIGHT, WINDOW_TITLE); 

    run_graphics_loop(&obj);

    cleanup_graphics();

    free(obj.point);

    return 0;
}