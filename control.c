#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "physics.c"

#define FPS 60.0 // 帧数
#define delta_t (1.0 / FPS) // 每次循环时间

// 或许应该传入object类的对象
bool running_status = true; // 开关

void sleep_micro(unsigned long m_seconds) { // 睡眠函数，传入毫秒数
    LARGE_INTEGER freq, start, end; // 这个数据类型本质是有i32和一个LongLong(即i64)成员的结构体，又是win的历史包袱，但是雀食好用
    QueryPerformanceFrequency(&freq); // 将精确时钟的频率赋值给结构体内的i64成员
    QueryPerformanceCounter(&start); // 获取精确时钟的当前计数
    // 注意，精准时钟的本质是一个高频累加，它的返回值是一个整数，是它目前经过的循环数，所以需要除以频率才能得到秒数

    double delta = 0; // 用来计量时间变更量(秒)
    double target_seconds = m_seconds / 1000000.0; // 这是我们需要睡的秒数(毫秒转秒)

    while (delta < target_seconds) {
        QueryPerformanceCounter(&end); // 获取精确时钟当前计数
        delta = (double) (end.QuadPart - start.QuadPart) / freq.QuadPart; // 获得经过的秒数
    }
}

unsigned long get_time() {  // 获取当前时间，基本逻辑和上一个函数完全一致
    LARGE_INTEGER freq, time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    return (unsigned long)((double)time.QuadPart * 1000000.0 / freq.QuadPart);
}

int main() {

    init(&car);
    // 下面是环境变量
    const double fps = 60; // 帧数
    const double target_delta_t = 1 / fps; // 目标帧时间，用来进行物理模拟
    double frame_delta_time; // 实际帧时间，在循环内被计量
    const unsigned long time_per_frame = 1000000 / fps; // 每帧微秒数
    double accumulator = 0; // 计量总时间
    unsigned long start_time = get_time(); // 这个循环开始的时间
    // double frame_start_time = 0; // 帧起始时间

    // 下面是循环中的变量
    unsigned long frame_start = 0; // 当前循环的起始时间
    int frame_count = 0; // 时间计数器
    unsigned long frame_end_time, frame_start_time;
    frame_start_time = get_time();


    while (running_status) { // 控制循环主体
        frame_start = get_time();

        frame_end_time = get_time();
        frame_delta_time = (double)(frame_end_time - frame_start_time) / 1000000;
        frame_start_time = frame_end_time;

        if (frame_delta_time > 0.25) {
            frame_delta_time = 0.25;
        }

        accumulator += frame_delta_time; // 累计时间计数器

        while (accumulator >= target_delta_t) { // 更新状态
            newton(&car); 
            /*

            vector delta_position = mut_vd(car.v, target_delta_t);
            car.position = add_vv(car.position, delta_position);
            */
            car.v.x += car.a.x * target_delta_t;
            car.v.y += car.a.y * target_delta_t;
            
            update(&car, target_delta_t);

            accumulator -= target_delta_t;
            frame_count += 1;
        };

        unsigned long elapsed = get_time() - frame_start;
        if (elapsed < time_per_frame) {
            sleep_micro(time_per_frame - elapsed);
        };

        if (frame_count % (int)fps == 0) {
            printf("加速度：(%.2f,%.2f) | 位置：(%.2f,%.2f) | 速度：(%.2f,%.2f)\n", car.a.x, car.a.y, car.position.x, car.position.y, car.v.x, car.v.y);
        }

        if (get_time() - start_time > 1000000 * 10) {
            running_status = false;
        }
    };

    printf("模拟结束\n");
    system("pause");
    return 0;
}