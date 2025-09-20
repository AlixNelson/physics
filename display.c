#include <raylib.h>


int main() {
	//初始化窗口
	InitWindow(800, 600,"窗口标题");
	SetTargetFPS(60);
	
	
	
	//开启程序的主循环
	while (!WindowShouldClose()) {
		
		
		
		//绘制帧，
		BeginDrawing();//开始绘制
		ClearBackground(WHITE);//重新绘制一些背景。
		
		
		
		EndDrawing();//结束绘制
	}
	
	
	//关闭窗口
	CloseWindow();
	return 0;
}

