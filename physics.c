#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

typedef struct {
    double x;
    double y; 
} vector; // 用一个结构体定义，可以是向量或坐标
typedef struct {
    vector start;
    vector end;
} line; // 线段，由首尾两个坐标定义，运算时被解为一个表达式，它是组成实体的基本单位
typedef struct {
    vector *point; // 顶点，定义时务必确保可以首尾相连，务必确保顺序，否则会出现严重问题，参见函数init_polygonal和make_lines
    int point_count; // 顶点的数量
    vector position; // 位置，等同于质心
    vector prev_position; // 上一帧的位置，用于verlet积分
    float mass; // 质量
    vector v; // 线速度
    vector F; // 合力
    vector a; // 加速度
    float ang_v; // 角速度
    float torque; // 扭矩
    float inertia; // 转动惯量
    bool active;
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
double mut_vv(vector a, vector b) { // 点乘
    float muted = a.x * b.x + a.y * b.y;
    return muted;
}
vector divide_vv(vector a, vector b) { // 除法虽然大概不会用到但还是定义一下，不对这个运算真的有意义吗
    vector dived = {
        .x = a.x / b.y,
        .y = a.y / b.y,
    };
    return dived;
}
float cross_product(vector a, vector b) { // 叉乘，有关角动量，有关相交检测
    float crossed = a.x * b.y - a.y * b.x;
    return crossed;
}
vector vector_n(vector l) { // 取法向量，方法是将向量旋转90°
    vector n = {.x=l.x, .y=-l.y};
    return n;
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

const vector G = {.x=0,.y=9.8};

// 所有原型在被定义时务必顺序（顺时针或逆时针单程），否则在向量化时会发生严重逻辑问题, 参见函数init_polygonal和make_lines

vector* init_polygonal/*初始化多边形*/(vector *points/*多边形原型*/,int count/*顶点数量，在init函数中被计算*/, float times /*倍率*/) {
    vector* new_points = malloc(sizeof(vector) * count);
    for (int i = 0; i < count; ++i) {
        new_points[i] = mut_vd(points[i], times);
    };
    return new_points;
}

object init(vector *polygonal, float mass, vector position,int times, bool active) {
    object car = {0};
    int count = sizeof(square)/sizeof(vector);
    car.point = init_polygonal(square, count, times);
    car.point_count = count;
    car.mass = mass;
    car.position = position;
    car.prev_position = car.position;
    car.F.x = 0;
    car.F.y = -9.8 * car.mass; // 始终给它一个重力，但这样过于简化，我害怕后续复杂状态下可能丢失
    car.a.y = -9.8;
    car.active = active;
    /*
    car = (object) {.mass = 1000,.position = {.x = 0,.y = 0},.v = {.x = 0,.y = 0,},.F = {.x = 200,.y = 300,}};
    没用的东西，这是没有线条和点，只有质心时的初始化
    */

    double dt = 1.0 / 60.0; // 假设时间步长
    car.prev_position.x = car.position.x - car.v.x * dt + 0.5 * car.a.x * dt * dt;
    car.prev_position.y = car.position.y - car.v.y * dt + 0.5 * car.a.y * dt * dt;
    car.prev_position.x = car.position.x - car.v.x * dt + 0.5 * car.a.x * dt * dt;
    car.prev_position.y = car.position.y - car.v.y * dt + 0.5 * car.a.y * dt * dt;
    // 最后三行是AI写的，因为prev_position应当表示上一次更新的位置，于是进行一次逆运算，否则将引入误差

    return car;

    return car;
}

void newton(object *obj) { // 牛顿第二定律，但只是将合力化为加速度
    vector F = obj->F;
    vector a = { // 加速度
        .x = F.x / obj->mass,
        .y = F.y / obj->mass,
    };
    obj->a = a;
}

void update(vector old_v, object *obj, double delta_t) { // 更新运动状态
    vector old_position = obj->position;

    /*
    // 从AI大仙那里求到的verlet积分，因为我发现原本的计算方式会积累不小的误差
    obj->position.x = 2 * obj->position.x - obj->prev_position.x + obj->a.x * delta_t * delta_t;
    obj->position.y = 2 * obj->position.y - obj->prev_position.y + obj->a.y * delta_t * delta_t;
    但是已经没用了，还得靠自己
    */
    obj->position.x += ((old_v.x + obj->v.x) / 2) * delta_t;
    obj->position.y += ((old_v.y + obj->v.y) / 2) * delta_t;

    obj->prev_position = old_position; // 这次计算的当前位置是下一次计算的上一位置
}

line* make_lines(object obj) { // 将物体的点向量化，并返回一个line数组
    int count = obj.point_count;
    line* lines = malloc(count * sizeof(line));
    for (int i = 0; i < count; i++) { // 这个循环将多边形首尾相连遍历
        if (i == count-1) {
            lines[i].start = add_vv(obj.point[i], obj.position);
            lines[i].end = add_vv(obj.point[0], obj.position);
        } else {
        lines[i].start = add_vv(obj.point[i], obj.position);
        lines[i].end = add_vv(obj.point[i+1], obj.position);
        }
    }
    return lines;
}

int sign_lp(vector a, vector b, vector c) { // 检测点c与向量ab的相对位置，左右或重合
    vector ab = sub_vv(b, a);
    vector ac = sub_vv(c, a);
    float cp = cross_product(ab, ac); // 叉乘，判断点c在向量ab的相对位置
    if (cp > 0) return 1; // 左侧
    else if (cp < 0) return -1; // 右侧
    else return 0; // 重合
}

bool cross(line a, line b) { // 判断两线段是否相交，返回布尔值
    vector as = a.start;
    vector ae = a.end;
    vector bs = b.start;
    vector be = b.end;

    int sign_asebs = sign_lp(as, ae, bs); // b的起点相对于向量a的位置
    int sign_asebe = sign_lp(as, ae ,be); // b的终点呢
    int sign_bseas = sign_lp(bs, be, as); // a的起点相对于向量b的位置
    int sign_bseae = sign_lp(bs, be, ae); // a的终点呢

    // 如果两个向量的起点与重点相互都是不同侧，则可以认定两个向量相互跨越，即相交
    if (sign_asebs * sign_asebe < 0 && sign_bseas * sign_bseae < 0) return true;
    return false;
}

void crush(object *car) { // 简单的碰撞反弹，只是将速度反转，将来实现动量定理之后一定要改
    vector temp_v = car->v;
    car->v.x = -temp_v.x;
    car->v.y = -temp_v.y;
}

// 要再往前走就必须得有图形了
// 这个模块暂时搁置，先做显示
/*
int main(void) {
    printf("%f,%f\n", car.v.x, car.v.y);
    init(&car);
    printf("%f,%f\n", car.v.x, car.v.y);
    newton(&car, 2);
    printf("v:%f,%f\n", car.v.x, car.v.y);
    gravity(&car, 2);
    printf("v:%f,%f\n", car.v.x, car.v.y);
    line* lines = make_lines(car);
    int count = car.point_count;
    for (int i = 0; i < count; i++) {
        printf("lines%lf,%lf,%lf,%lf\n", 
            lines[i].start.x,
            lines[i].start.y,
            lines[i].end.x,
            lines[i].end.y
        );
    }
    free(lines);
    system("pause");
    return 0;
}
*/