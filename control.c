#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "physics.c"

#define FPS 60.0 // 帧数
#define delta_t (1.0 / FPS) // 每次循环时间
#define Max_Frame_Skip 2 // 最高跳帧，每次循环最多可以在在这个帧数对应的时间中不进行检测
// 物理运算需要时间，如果在运算完成前下一个循环已经到来，则必会出现逻辑卡死

// 或许应该传入object类的对象
bool simulate_running_status = true; // 开关

void sleep(unsigned long m_seconds) { // 睡眠函数，传入毫秒数
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

unsigned long get_time() {}