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
    float mass; // 质量
    vector v; // 线速度
    vector F; // 合力
    vector a; // 加速度
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
float cross_product(vector a, vector b) { // 叉乘，有关角动量，有关相交检测
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
const vector G = {.x=0,.y=9.8};

vector square[4] = { // 这是一个正方体的原型
    {-1,-1},{-1,1},{1,1},{1,-1}
};
// 所有原型在被定义时务必顺序（顺时针或逆时针单程），否则在向量化时会发生严重逻辑问题, 参见函数init_polygonal和make_lines

vector* init_polygonal/*初始化多边形*/(vector *points/*顶点数组*/,int count/*顶点数量，在init函数中被计算*/, float times /*倍率*/) {
    vector* new_points = malloc(sizeof(vector) * count);
    for (int i = 0; i < count; ++i) {
        new_points[i] = mut_vd(points[i], times);
    };
    return new_points;
}

void init(object *car) {
    int count = sizeof(square)/sizeof(vector);
    car->point = init_polygonal(square, count, 2);
    car->point_count = count;
    car->mass = 1000;
    car->position.x = 2;
    car->position.y = 2;
    car->F.x = 0;
    car->F.y = 9.8 * car->mass;
    car->a.y = 9.8;
    /*
    car = (object) {.mass = 1000,.position = {.x = 0,.y = 0},.v = {.x = 0,.y = 0,},.F = {.x = 200,.y = 300,}};
    没用的东西，这是没有线条和点，只有质心时的初始化
    */
}

void newton(object *obj) { // 牛顿第二定律，但只是将合力化为加速度
    vector F = obj->F;
    vector a = { // 加速度
        .x = F.x / obj->mass,
        .y = F.y / obj->mass,
    };
    obj->a = a;
}

line* make_lines(object obj) { // 将物体的点向量化，并返回一个line数组
    int count = obj.point_count;
    line* lines = malloc(count * sizeof(line));
    for (int i = 0; i < count; i++) { // 这个循环将多边形首尾相连遍历
        if (i == count-1) {
            lines[i].start = obj.point[i];
            lines[i].end = obj.point[0];
        } else {
        lines[i].start = obj.point[i];
        lines[i].end = obj.point[i+1];
        }
    }
    return lines;
}

/*
void gravity(object *obj, float t) { // 重力，本来想直接用牛顿第二定律实现，但好像单独定义一个反而更加省事
    vector accelerate = mut_vd(G, t);
    obj->v = sub_vv(obj->v, accelerate);
}
果然还是大一统比较好，把a作为成员加入了object，并在控制循环中计算状态更新
所以这个函数已经没用了
*/

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
    if (sign_asebe * sign_asebe < 0 && sign_bseas * sign_bseae < 0) { 
        return true;
    } 
    return false;
}

bool crush() {
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