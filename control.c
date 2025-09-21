#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "physics.c"

#define FPS 60.0 // ֡��
#define delta_t (1.0 / FPS) // ÿ��ѭ��ʱ��

// ����Ӧ�ô���object��Ķ���
bool running_status = true; // ����

void sleep_micro(unsigned long m_seconds) { // ˯�ߺ��������������
    LARGE_INTEGER freq, start, end; // ����������ͱ�������i32��һ��LongLong(��i64)��Ա�Ľṹ�壬����win����ʷ����������ȸʳ����
    QueryPerformanceFrequency(&freq); // ����ȷʱ�ӵ�Ƶ�ʸ�ֵ���ṹ���ڵ�i64��Ա
    QueryPerformanceCounter(&start); // ��ȡ��ȷʱ�ӵĵ�ǰ����
    // ע�⣬��׼ʱ�ӵı�����һ����Ƶ�ۼӣ����ķ���ֵ��һ������������Ŀǰ������ѭ������������Ҫ����Ƶ�ʲ��ܵõ�����

    double delta = 0; // ��������ʱ������(��)
    double target_seconds = m_seconds / 1000000.0; // ����������Ҫ˯������(����ת��)

    while (delta < target_seconds) {
        QueryPerformanceCounter(&end); // ��ȡ��ȷʱ�ӵ�ǰ����
        delta = (double) (end.QuadPart - start.QuadPart) / freq.QuadPart; // ��þ���������
    }
}

unsigned long get_time() {  // ��ȡ��ǰʱ�䣬�����߼�����һ��������ȫһ��
    LARGE_INTEGER freq, time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    return (unsigned long)((double)time.QuadPart * 1000000.0 / freq.QuadPart);
}

int main() {

    init(&car);
    // �����ǻ�������
    const double fps = 60; // ֡��
    const double target_delta_t = 1 / fps; // Ŀ��֡ʱ�䣬������������ģ��
    double frame_delta_time; // ʵ��֡ʱ�䣬��ѭ���ڱ�����
    const unsigned long time_per_frame = 1000000 / fps; // ÿ֡΢����
    double accumulator = 0; // ������ʱ��
    unsigned long start_time = get_time(); // ���ѭ����ʼ��ʱ��
    // double frame_start_time = 0; // ֡��ʼʱ��

    // ������ѭ���еı���
    unsigned long frame_start = 0; // ��ǰѭ������ʼʱ��
    int frame_count = 0; // ʱ�������
    unsigned long frame_end_time, frame_start_time;
    frame_start_time = get_time();


    while (running_status) { // ����ѭ������
        frame_start = get_time();

        frame_end_time = get_time();
        frame_delta_time = (double)(frame_end_time - frame_start_time) / 1000000;
        frame_start_time = frame_end_time;

        if (frame_delta_time > 0.25) {
            frame_delta_time = 0.25;
        }

        accumulator += frame_delta_time; // �ۼ�ʱ�������

        while (accumulator >= target_delta_t) { // ����״̬
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
            printf("���ٶȣ�(%.2f,%.2f) | λ�ã�(%.2f,%.2f) | �ٶȣ�(%.2f,%.2f)\n", car.a.x, car.a.y, car.position.x, car.position.y, car.v.x, car.v.y);
        }

        if (get_time() - start_time > 1000000 * 10) {
            running_status = false;
        }
    };

    printf("ģ�����\n");
    system("pause");
    return 0;
}