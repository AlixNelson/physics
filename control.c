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

    vector square[4] = { // 这是一个正方体的原型
        {-1,-1},{-1,1},{1,1},{1,-1}
    };
    // 所有原型在被定义时务必顺序（顺时针或逆时针单程），否则在向量化时会发生严重逻辑问题, 参见函数init_polygonal和make_lines
    object car = init(square, 1000, (vector){0, 0}, 2, true); // 初始化车车，这个对象在核心中被定义

    vector board_init[4] = { // 定义一个板子的原型
        {-1000, -1}, {-1000, 1}, {1000, 1}, {1000, -1},
    };
    object board = init(board_init, 1000, (vector){0, -100}, 1, false); // 初始化板板
    object *obj_list[] = { &car, &board };

    // 下面是环境变量
    const double fps = 60; // 帧数
    const double target_delta_t = 1 / fps; // 目标帧时间，用来进行物理模拟
    double frame_delta_time; // 实际帧时间，在循环内被计量
    const unsigned long time_per_frame = 1000000 / fps; // 每帧微秒数
    double accumulator = 0; // 计量总时间
    unsigned long start_time = get_time(); // 这个循环开始的时间
    int len = sizeof(obj_list) / sizeof(object*);
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
        // printf("外层循环完成");

        int index = 0;
        line** line_list = NULL;

        for (int i = 0; i < len; i++) { // 将所有多边形成员向量化
            int line_len = obj_list[i]->point_count * sizeof(line*); // 当前元素的顶点数，即为它的线段数
            object *obj = obj_list[i]; // 需要被处理的对象
            line* lines = make_lines(*obj); // 生成这个对象的线段列表
            line** temp = realloc(line_list, (index + 1) * sizeof(line*)); // 动态分配内存并将得到的线段数组暂存
            if (temp == NULL) { // 如果分配失败
                free(line_list);
                exit(EXIT_FAILURE);
            }
            line_list = temp; // 将被分配的临时二维列表写入
            line_list[index] = lines;
            index++; // 计数
            
            /*我要开始写小众变态算法了，如果看不懂请不要问我，我大概也看不懂*/
            /*开始*/
        };

        for (int i = 0; i < len; i++) { // 检测每个多边形是否active，不active的不更新运动状态
            // printf("进入二层循环循环");
            object *obj = obj_list[i];
            // printf("二层循环完成");
            bool is_cross = false;
            if (obj->active == true) {
                int active_count = obj->point_count;
                // printf("三层判断完成");
                while (accumulator >= target_delta_t) { // 更新状态
                // printf("进入三层循环");
                newton(obj);
            
                vector old_v = obj->v;
                // printf("%.16f, %.16f", old_v.x, old_v.y);
                car.v.x += car.a.x * target_delta_t;
                car.v.y += car.a.y * target_delta_t;
                
                update(old_v, obj, target_delta_t);

                if (line_list != NULL) {
                    if (i >= 0) {
                        if (line_list[i] != NULL) {free(line_list[i]);}
                        line_list[i] = make_lines(*obj);
                    }
                }

                accumulator -= target_delta_t;
                frame_count += 1;
                
                for (int j = 0; j < len; j++) {
                    if (j == i) continue;            // 只跳过自己
                    line *lines = line_list[j];
                    int count = obj_list[j]->point_count;
                    for (int q = 0; q < count; q++) {
                        for (int p = 0; p < active_count; p++) {
                            if (cross(lines[q], line_list[i][p])) {
                                is_cross = true;
                                break;
                            }
                        }
                        if (is_cross) break;
                    }
                    if (is_cross) break;
                }
            }

                unsigned long elapsed = get_time() - frame_start;
                if (elapsed < time_per_frame) {
                    sleep_micro(time_per_frame - elapsed);
                };  // 这是干什么的来着？

                if (frame_count % (int)fps == 0) {
                    printf("加速度：(%.2f,%.2f) | 位置：(%.2f,%.2f) | 速度：(%.2f,%.2f)\n", car.a.x, car.a.y, car.position.x, car.position.y, car.v.x, car.v.y);
                };
                
                if (is_cross) { // 如果碰撞为真
                    printf("pong!");
                    crush(&car);
                    printf("加速度：(%.2f,%.2f) | 位置：(%.2f,%.2f) | 速度：(%.2f,%.2f)\n", car.a.x, car.a.y, car.position.x, car.position.y, car.v.x, car.v.y);
                };

                if (get_time() - start_time > 1000000 * 20) { // 测试用输出，模拟20s
                    running_status = false;
                };
            }
        }
    }

    printf("模拟结束\n");
    system("pause");
    return 0;
}