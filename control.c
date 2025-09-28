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

    vector square[4] = { // ����һ���������ԭ��
        {-1,-1},{-1,1},{1,1},{1,-1}
    };
    // ����ԭ���ڱ�����ʱ���˳��˳ʱ�����ʱ�뵥�̣���������������ʱ�ᷢ�������߼�����, �μ�����init_polygonal��make_lines
    object car = init(square, 1000, (vector){0, 0}, 2, true); // ��ʼ����������������ں����б�����

    vector board_init[4] = { // ����һ�����ӵ�ԭ��
        {-1000, -1}, {-1000, 1}, {1000, 1}, {1000, -1},
    };
    object board = init(board_init, 1000, (vector){0, -100}, 1, false); // ��ʼ�����
    object *obj_list[] = { &car, &board };

    // �����ǻ�������
    const double fps = 60; // ֡��
    const double target_delta_t = 1 / fps; // Ŀ��֡ʱ�䣬������������ģ��
    double frame_delta_time; // ʵ��֡ʱ�䣬��ѭ���ڱ�����
    const unsigned long time_per_frame = 1000000 / fps; // ÿ֡΢����
    double accumulator = 0; // ������ʱ��
    unsigned long start_time = get_time(); // ���ѭ����ʼ��ʱ��
    int len = sizeof(obj_list) / sizeof(object*);
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
        // printf("���ѭ�����");

        int index = 0;
        line** line_list = NULL;

        for (int i = 0; i < len; i++) { // �����ж���γ�Ա������
            int line_len = obj_list[i]->point_count * sizeof(line*); // ��ǰԪ�صĶ���������Ϊ�����߶���
            object *obj = obj_list[i]; // ��Ҫ������Ķ���
            line* lines = make_lines(*obj); // �������������߶��б�
            line** temp = realloc(line_list, (index + 1) * sizeof(line*)); // ��̬�����ڴ沢���õ����߶������ݴ�
            if (temp == NULL) { // �������ʧ��
                free(line_list);
                exit(EXIT_FAILURE);
            }
            line_list = temp; // �����������ʱ��ά�б�д��
            line_list[index] = lines;
            index++; // ����
            
            /*��Ҫ��ʼдС�ڱ�̬�㷨�ˣ�����������벻Ҫ���ң��Ҵ��Ҳ������*/
            /*��ʼ*/
        };

        for (int i = 0; i < len; i++) { // ���ÿ��������Ƿ�active����active�Ĳ������˶�״̬
            // printf("�������ѭ��ѭ��");
            object *obj = obj_list[i];
            // printf("����ѭ�����");
            bool is_cross = false;
            if (obj->active == true) {
                int active_count = obj->point_count;
                // printf("�����ж����");
                while (accumulator >= target_delta_t) { // ����״̬
                // printf("��������ѭ��");
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
                    if (j == i) continue;            // ֻ�����Լ�
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
                };  // ���Ǹ�ʲô�����ţ�

                if (frame_count % (int)fps == 0) {
                    printf("���ٶȣ�(%.2f,%.2f) | λ�ã�(%.2f,%.2f) | �ٶȣ�(%.2f,%.2f)\n", car.a.x, car.a.y, car.position.x, car.position.y, car.v.x, car.v.y);
                };
                
                if (is_cross) { // �����ײΪ��
                    printf("pong!");
                    crush(&car);
                    printf("���ٶȣ�(%.2f,%.2f) | λ�ã�(%.2f,%.2f) | �ٶȣ�(%.2f,%.2f)\n", car.a.x, car.a.y, car.position.x, car.position.y, car.v.x, car.v.y);
                };

                if (get_time() - start_time > 1000000 * 20) { // �����������ģ��20s
                    running_status = false;
                };
            }
        }
    }

    printf("ģ�����\n");
    system("pause");
    return 0;
}