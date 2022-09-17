#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <Windows.h>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include "BMP.h"

//#define DELAY 1
#define KEY_SPACE 32
#define KEY_ESC 27
#define KEY_ENTER 13

#define SPEED 60.f
#define PATH "img.bmp"

float distance = -100.f;

int WIDTH = 120;
int HEIGHT = 39;

int winWidth = 1900, winHeight = 980;

int rightOffset = 250;

float gridSize;

static int frame;
int FPS = 0;
long int elapsedTime = 0;

bool pause = false, lastPause = true;

std::vector<std::vector<bool>> field(WIDTH + 1, std::vector<bool>(HEIGHT + 1));
std::vector<std::vector<bool>> newField(WIDTH + 1, std::vector<bool>(HEIGHT + 1));

std::string name = PATH;
std::string finishCode;

std::chrono::high_resolution_clock::time_point timer_start, timer_end, step_time;

float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float crdX(float coord) {
	return map(coord, 0, winWidth, -1, 1);
}

float crdY(float coord) {
	return map(coord, 0, winHeight, -1, 1);
}

void glPoint(float x, float y) {
	glVertex3f(crdX(x), -crdY(y), -1);
}

void changeSize(int w, int h) {
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	/*
	// предотвращение деления на ноль
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;
	// используем матрицу проекции
	glMatrixMode(GL_PROJECTION);
	// обнуляем матрицу
	glLoadIdentity();
	// установить параметры вьюпорта
	glViewport(0, 0, w, h);
	// установить корректную перспективу
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);
	// вернуться к матрице проекции
	glMatrixMode(GL_MODELVIEW);
	*/
}

void processNormalKeys(unsigned char key, int x, int y) {
	//if (key == 13)
		//pause = !pause;
	if (key == KEY_SPACE) {
		pause = !pause;
	}
}

void drawString(float x, float y, float z, const char* string) {
	glRasterPos3f(crdX(x), crdY(y), z);
	for (const char* c = string; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);  // Updates the position
	}
}

void drawString(float x, float y, float z, char* string) {
	glRasterPos3f(crdX(x), crdY(y), z);
	for (char* c = string; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);  // Updates the position
	}
}

void drawString(float x, float y, float z, std::string string) {
	glRasterPos3f(crdX(x), crdY(y), z);
	for (int c = 0; string[c] != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[c]);  // Updates the position
	}
}

void setup() {
	try {
		BMP BMPfield(name);
		std::cout << "Importing " + name + ".\n";
		//if (BMPfield.get_heigt() != HEIGHT || BMPfield.get_width() != WIDTH) throw std::runtime_error("Error! Invalid size of BMP image.");
		WIDTH = BMPfield.get_width();
		HEIGHT = BMPfield.get_heigt();

		std::cout << "Image sizes{ width: " << WIDTH << "; height: " << HEIGHT << " }\n";

		int lineSize = BMPfield.get_bytes_per_raw();
		int fileSize = lineSize * HEIGHT;

		std::cout << "line size: " << lineSize << "; file size: " << fileSize << "\n";

		std::ifstream img{ name, std::ios_base::binary };

		if (!img.good()) throw std::runtime_error("Error! Couldn't open " + name + " in binary mode.");

		img.seekg(BMPfield.file_header.offset_data); // 0x3e (62) bytes, start of pixels info
		//img.seekg(0x3e); // 0x3e (62) bytes, start of pixels info

		std::vector<unsigned char> imgData(fileSize);

		img.read((char*)&imgData[0], fileSize);
		std::cout << "Writen " << fileSize << " bytes to imgData.\n";

		img.close();
		std::cout << "Import is completed.\n";

		field = std::vector<std::vector<bool>>(WIDTH + 1, std::vector<bool>(HEIGHT + 1));
		newField = std::vector<std::vector<bool>>(WIDTH + 1, std::vector<bool>(HEIGHT + 1));

		//*
		for (int y = HEIGHT - 1; y >= 0; y--) {
			for (int x = 0; x < WIDTH; x++) {
				int pos = y * lineSize + x / 8; // currently readable byte
				int bit = 1 << (7 - x % 8);
				//std::cout << "Import " << name << "  size: " << imgData.size() << '\t' << "  byte n:  " << pos << '\n';
				//std::cout << imgData.size() << '\t' << pos << '\t' << (int)imgData[pos] << '\n'; //1695, 3rd iter - fail
				//
				int pixel = !((imgData[pos] & bit) > 0); // 1 - black, 0 - white
				field[x][HEIGHT - y - 1] = pixel;
			}
		}
		//*/
	} catch (std::runtime_error err) {
		std::cout << err.what() << '\n';
		glutDestroyWindow(1);
	}

	timer_start = timer_end = step_time = std::chrono::high_resolution_clock::now();
}

void drawField() {
	gridSize = min((float)(winWidth - rightOffset) / (float)WIDTH, (float)winHeight / (float)HEIGHT);

	for (int y = 0; y < HEIGHT; y++) {
		glBegin(GL_LINE_STRIP);
		glPoint(0, y * gridSize);
		glPoint(WIDTH * gridSize, y * gridSize);
		glEnd();

		for (int x = 0; x < WIDTH; x++) {
			glBegin(GL_LINE_STRIP);
			glPoint(x * gridSize, 0);
			glPoint(x * gridSize, HEIGHT * gridSize);
			glEnd();

			if (field[x][y]) {
				glColor3f(0.5, 0.5, 0.5);
				glBegin(GL_QUADS);
				glPoint(((float)x + 0.1) * gridSize, ((float)y + 0.1) * gridSize);
				glPoint(((float)x + 0.9) * gridSize, ((float)y + 0.1) * gridSize);
				glPoint(((float)x + 0.9) * gridSize, ((float)y + 0.9) * gridSize);
				glPoint(((float)x + 0.1) * gridSize, ((float)y + 0.9) * gridSize);
				glEnd();	
			}
			glColor3f(0.15, 0.15, 0.15);
		}
	}
	glBegin(GL_LINE_STRIP);
	glPoint(WIDTH * gridSize, 0);
	glPoint(WIDTH * gridSize, HEIGHT * gridSize);
	glPoint(0, HEIGHT * gridSize);
	glPoint(WIDTH * gridSize, HEIGHT * gridSize);
	glEnd();

}
// game step
int fieldAnalyse() {
	int alive = 0;
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int neighbors = 0; // number of neighbors
			if (field[(WIDTH + x - 1) % WIDTH][(HEIGHT + y - 1) % HEIGHT] == 1) neighbors++; //  (0, 0)
			if (field[(WIDTH + x - 1) % WIDTH][y] == 1) neighbors++;						 //  (0, 1)
			if (field[(WIDTH + x - 1) % WIDTH][(y + 1) % HEIGHT] == 1) neighbors++;			 //  (0, 2)
			if (field[x][(HEIGHT + y - 1) % HEIGHT] == 1) neighbors++;						 //  (1, 0)
			if (field[x][(y + 1) % HEIGHT] == 1) neighbors++;								 //  (1, 2)
			if (field[(x + 1) % WIDTH][(HEIGHT + y - 1) % HEIGHT] == 1) neighbors++;		 //  (2, 0)
			if (field[(x + 1) % WIDTH][y] == 1) neighbors++;								 //  (2, 1)
			if (field[(x + 1) % WIDTH][(y + 1) % HEIGHT] == 1) neighbors++;					 //  (2, 2)

			if (field[x][y] == 0 && neighbors == 3) { // newborn
				newField[x][y] = 1;
				alive++;
			} else if (field[x][y] == 1 && (neighbors == 2 || neighbors == 3)) { // still alive
				newField[x][y] = 1;
				alive++;
			} else newField[x][y] = 0; // die
		}
	}

	field = newField;

	return alive;
}

void renderScene(void) {
	timer_end = std::chrono::high_resolution_clock::now();
	static int alive;

	if (std::chrono::duration_cast<std::chrono::microseconds>(timer_end - step_time).count() > 1e6f / SPEED) {
		if (!pause) {
			alive = fieldAnalyse();
			step_time = timer_end;
		}
	} else return;
	
	if (alive == 0) {
		finishCode = "Everyone died.";
		std::cout << finishCode << "\n";
		return;
	}

	// очистка буфера цвета и глубины
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1, 0.1, 0.1, 1.0);
	// обнуление трансформации
	glLoadIdentity();
	
	glColor3f(0.15, 0.15, 0.15);
	glLineWidth(1);

	drawField();

	glColor3f(1.0, 1.0, 1.0);

	drawString(WIDTH * gridSize + 20, winHeight - 30, -1, "Elapsed time: " + std::to_string(elapsedTime/1000) + " ms");
	drawString(WIDTH * gridSize + 20, winHeight - 60, -1, "FPS: " + std::to_string(FPS));
	drawString(WIDTH * gridSize + 20, winHeight - 90, -1, "Frame: " + std::to_string(frame));
	drawString(WIDTH * gridSize + 20, winHeight - 120, -1, "Field size: " + std::to_string(WIDTH * HEIGHT));
	drawString(WIDTH * gridSize + 20, winHeight - 150, -1, "Alive: " + std::to_string(alive));
	drawString(WIDTH * gridSize + 20, winHeight - 180, -1, std::string("Pause: ") + (pause ? "ON" : "OFF"));

	glutSwapBuffers();

	frame++;

	elapsedTime = (std::chrono::duration_cast<std::chrono::microseconds>(timer_end - timer_start).count());
	FPS = 1e6 / elapsedTime;
	timer_start = timer_end;
}

int main(int argc, char** argv) {

	// Инициализация GLUT и создание окна
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 80);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("LifeGame");

	// регистрация
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(processNormalKeys);

	if (argc > 1)name = argv[1];
	setup();

	//td::vector<std::vector<bool>> field(WIDTH + 1, std::vector<bool>(HEIGHT + 1));
	//std::vector<std::vector<bool>> newField(WIDTH + 1, std::vector<bool>(HEIGHT + 1));

	// основной цикл
	glutMainLoop();

	return 1;
}