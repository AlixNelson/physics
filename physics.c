#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define G 9.8

typedef struct {
    double x;
    double y; 
} vector; // 用一个结构体定义，可以是向量或坐标
typedef struct {
    vector start;
    vector end;
} line; // 线段，由首尾两个坐标定义，运算时被解为一个表达式，它是组成实体的基本单位
typedef struct {
    vector *point; // 顶点
    int point_count; // 顶点的数量
    vector position; // 重心
    float mass; // 质量
    vector v; // 线速度
    vector F; // 合力
    float ang_v; // 角速度
    float torque; // 扭矩
    float inertia; // 转动惯量
    // 我不会角动量定理，但先定义再说
} object; // 实体，由许多顶点定义，运算时两个顶点被解为一个线段，由线段表达式是否相交来判断碰撞

// 定义向量对向量运算
vector add_vv(vector a, vector b) { // 加法
    vector added = {
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
    return added;
}
vector sub_vv(vector a, vector b) { // 减法
    vector subed = {
        .x = a.x - b.x,
        .y = a.y - b.y,
    };
    return subed;
}
float mut_vv(vector a, vector b) { // 点乘 它的物理意义决定它大概不会超出float范围，大概...
    float muted = a.x * b.x + a.y * b.y;
    return muted;
}
vector divide_vv(vector a, vector b) { // 除法虽然大概不会用到但还是定义一下
    vector dived = {
        .x = a.x / b.y,
        .y = a.y / b.y,
    };
    return dived;
}
float cross_product(vector a, vector b) { // 叉乘，有关角动量
    float crossed = a.x * b.y - a.y * b.x;
    return crossed;
}

// 定义向量对数值的运算
vector add_vd(vector a, double b) { // 向量与double加法
    vector added = {
        .x = a.x + b,
        .y = a.y + b,
    };
    return added;
}
vector mut_vd(vector a, double b) { // 向量与double乘法
    vector muted = {
        .x = a.x * b,
        .y = a.y * b,
    };
    return muted;
}
vector divide_vd(vector a, double b) {
    vector divided = {
        .x = a.x / b,
        .y = a.y / b,
    };
    return divided;
}
// 其他暂时不定义，估计不会用到

object car = {0};

vector square[4] = { // 这是一个正方体的原型
    {-1,-1},{-1,1},{1,1},{1,-1}
};

vector* init_polygonal/*初始化多边形*/(vector *points/*顶点数组*/, int count, float times /*倍率*/) {
    vector* new_points = malloc(sizeof(vector) * count);
    for (int i = 0; i < count; ++i) {
        new_points[i] = mut_vd(points[i], times);
    };
}

void init(object *car) {
    car->point = init_polygonal(square, 4, 2);
    car->point_count = 4;
    car->mass = 1000;
    car->position.x = 0;
    car->position.y = 0;
    car->F.x = 200;
    car->F.y = 300;
    /*
    car = (object) {.mass = 1000,.position = {.x = 0,.y = 0},.v = {.x = 0,.y = 0,},.F = {.x = 200,.y = 300,}};
    没用的东西，这是没有线条时的初始化
    */
}

void newton(object *c, float t) {  // 牛顿第二定律
    vector F = c->F;
    vector a = { // 加速度
        .x = F.x / c->mass,
        .y = F.y / c->mass,
    };
    vector accelerate = mut_vd(a, t); // 速度变量
    c->v = add_vv(c->v, accelerate);
}

void vectors(object obj) { 
/*
将多边形的顶点向量化 可是真的需要这么做吗？
或许可以直接根据点的坐标判断？
这两个路线我都没有认真考虑过
*/
}


int main(void) {
    printf("%f,%f\n", car.v.x, car.v.y);
    init(&car);
    printf("%f,%f\n", car.v.x, car.v.y);
    newton(&car, 2);
    printf("%f,%f\n", car.v.x, car.v.y);
    system("pause");
    return 0;
}