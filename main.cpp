//	main.cpp
//	4simb

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <Windows.h>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include "BMP.h"
#include "LifeGame.h"

//#define DEBUG

float distance = -100.f;

float SPEED = 60.f;
int WIDTH = 120;
int HEIGHT = 39;

int winWidth = 1200, winHeight = 900;

int rightOffset = 250, downOffset = 30;

float gridSize;

static int frame;
float FPS = 0.f;
long int elapsedTime = 0;

std::vector<float> FPSs;

bool pause = false, lastPause = true;
bool erase = false, ctrl = false;

std::vector<std::vector<char>> field(WIDTH + 1, std::vector<char>(HEIGHT + 1));
std::vector<std::vector<char>> newField(WIDTH + 1, std::vector<char>(HEIGHT + 1));

std::string name = PATH;
std::string finishCode;

std::chrono::high_resolution_clock::time_point timer_start, timer_end, step_time;

void glPoint(float x, float y) {
	glVertex3f(crdX(x, winWidth), -crdY(y, winHeight), -1);
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
	} else if (key == 'z' || key == 'Z') {
		if (SPEED < 240) SPEED += 1;
	} else if (key == 'x' || key == 'X') {
		if (SPEED > 1) SPEED -= 1;
	} else if (key == 'c' || key == 'C') {
		// clear the whole field
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				field[x][y] = EMPTY;
			}
		}
#ifdef DEBUG
		std::cout << "Field cleared.";
#endif
	}
}

void processMouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			erase = true;
			int fieldX = (float)x / gridSize;
			int fieldY = (float)y / gridSize;
			field[fieldX][fieldY] = EMPTY;
		} else erase = false;
	} else if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) {
			int mod = glutGetModifiers();
			int fieldX = (float)x / gridSize;
			int fieldY = (float)y / gridSize;
			if (mod == GLUT_ACTIVE_CTRL) field[fieldX][fieldY] = WALL;
			else field[fieldX][fieldY] = ALIVE;
		}
	}
#ifdef DEBUG
	std::cout << "Erase: " << erase << "\n";
#endif
}

void processMouseMotion(int x, int y) {
	int fieldX = (float)x / gridSize;
	int fieldY = (float)y / gridSize;

#ifdef DEBUG
	std::cout << fieldX << "  " << fieldY << "\n";
#endif

	if (fieldX > WIDTH || fieldY > HEIGHT) return;
	
	if (erase) { // delete cells
		field[fieldX][fieldY] = EMPTY;
#ifdef DEBUG
		std::cout << "Erase cell " << fieldX << " " << fieldY << "\n";
#endif
	} else { // set cells
		field[fieldX][fieldY] = ALIVE;
#ifdef DEBUG
		std::cout << "Draw cell " << fieldX << " " << fieldY << "\n";
#endif
	}
}

void drawString(float x, float y, float z, const char* string) {
	glRasterPos3f(crdX(x, winWidth), crdY(y, winHeight), z);
	for (const char* c = string; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);  // Updates the position
	}
}

void drawString(float x, float y, float z, char* string) {
	glRasterPos3f(crdX(x, winWidth), crdY(y, winHeight), z);
	for (char* c = string; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);  // Updates the position
	}
}

void drawString(float x, float y, float z, std::string string) {
	glRasterPos3f(crdX(x, winWidth), crdY(y, winHeight), z);
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

		field = std::vector<std::vector<char>>(WIDTH + 1, std::vector<char>(HEIGHT + 1));
		newField = std::vector<std::vector<char>>(WIDTH + 1, std::vector<char>(HEIGHT + 1));

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
	gridSize = min((float)(winWidth - rightOffset) / (float)WIDTH, (float)(winHeight - downOffset) / (float)HEIGHT);

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
				if (field[x][y] == WALL) glColor3f(0.2, 0.2, 0.2);
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
int oldFieldAnalyse() { // TO OPTIMIZE
	int alive = 0;
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int neighbors = 0; // number of neighbors
			if (field[(WIDTH + x - 1) % WIDTH][(HEIGHT + y - 1) % HEIGHT] == ALIVE) neighbors++; //  (0, 0)
			if (field[(WIDTH + x - 1) % WIDTH][y] == ALIVE) neighbors++;						 //  (0, 1)
			if (field[(WIDTH + x - 1) % WIDTH][(y + 1) % HEIGHT] == ALIVE) neighbors++;			 //  (0, 2)
			if (field[x][(HEIGHT + y - 1) % HEIGHT] == ALIVE) neighbors++;						 //  (1, 0)
			if (field[x][(y + 1) % HEIGHT] == ALIVE) neighbors++;								 //  (1, 2)
			if (field[(x + 1) % WIDTH][(HEIGHT + y - 1) % HEIGHT] == ALIVE) neighbors++;		 //  (2, 0)
			if (field[(x + 1) % WIDTH][y] == ALIVE) neighbors++;								 //  (2, 1)
			if (field[(x + 1) % WIDTH][(y + 1) % HEIGHT] == ALIVE) neighbors++;					 //  (2, 2)

			if (field[x][y] == EMPTY && neighbors == 3) { // newborn
				newField[x][y] = ALIVE;
				alive++;
			} else if (field[x][y] == ALIVE && (neighbors == 2 || neighbors == 3)) { // still alive
				newField[x][y] = ALIVE;
				alive++;
			} else newField[x][y] = EMPTY; // die
		}
	}

	field = newField;

	return alive;
}

int fieldAnalyse() { // TO OPTIMIZE
	int alive = 0;
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int neighbors = field[(WIDTH + x - 1) % WIDTH][(HEIGHT + y - 1) % HEIGHT]; // number of neighbors
			neighbors += field[(WIDTH + x - 1) % WIDTH][y] + field[(WIDTH + x - 1) % WIDTH][(y + 1) % HEIGHT];
			neighbors += field[x][(HEIGHT + y - 1) % HEIGHT] + field[x][(y + 1) % HEIGHT];
			neighbors += field[(x + 1) % WIDTH][(HEIGHT + y - 1) % HEIGHT] + field[(x + 1) % WIDTH][y] + field[(x + 1) % WIDTH][(y + 1) % HEIGHT];

			if (field[x][y] == EMPTY && neighbors == 3) { // newborn
				newField[x][y] = ALIVE;
				alive++;
			} else if (field[x][y] == ALIVE && (neighbors == 2 || neighbors == 3)) { // still alive
				newField[x][y] = ALIVE;
				alive++;
			} else newField[x][y] = EMPTY; // die
		}
	}

	field = newField;

	return alive;
}

void renderScene(void) {
	timer_end = std::chrono::high_resolution_clock::now();
	static int alive = 1;

	/*
	if (alive == 0) {
		finishCode = "Everyone died.";
		std::cout << finishCode << "\n";
		return;
	}
	//*/

	// очистка буфера цвета и глубины
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1, 0.1, 0.1, 1.0);
	// обнуление трансформации
	glLoadIdentity();
	
	glColor3f(0.15, 0.15, 0.15);
	glLineWidth(1);

	drawField(); // 7ms

	glColor3f(1.0, 1.0, 1.0);

	drawString(WIDTH * gridSize + 20, winHeight - 30, -1, "Elapsed time: " + std::to_string(elapsedTime/1000) + " ms");
	drawString(WIDTH * gridSize + 20, winHeight - 60, -1, "FPS: " + std::to_string(FPS));
	drawString(WIDTH * gridSize + 20, winHeight - 90, -1, "Max FPS: " + std::to_string((int)SPEED));
	drawString(WIDTH * gridSize + 20, winHeight - 120, -1, "Frame: " + std::to_string(frame));
	drawString(WIDTH * gridSize + 20, winHeight - 150, -1, "Field size: " + std::to_string(WIDTH * HEIGHT));
	drawString(WIDTH * gridSize + 20, winHeight - 180, -1, "Alive: " + std::to_string(alive));
	drawString(WIDTH * gridSize + 20, winHeight - 210, -1, std::string("Pause: ") + (pause ? "ON" : "OFF"));

	drawString(10, winHeight - gridSize * HEIGHT - 25, -1, "Z: +FPS  X: -FPS  C: Clear  Space: Pause");
	// +7ms
	glutSwapBuffers();

	if (std::chrono::duration_cast<std::chrono::microseconds>(timer_end - step_time).count() > 1e6f / SPEED) {
		if (!pause) {
			alive = fieldAnalyse(); // 33ms
			frame++;
		}
		step_time = timer_end;
	} else return;

	elapsedTime = (std::chrono::duration_cast<std::chrono::microseconds>(timer_end - timer_start).count());
	FPS = 1e6f / elapsedTime;

	timer_start = timer_end;
}

int main(int argc, char** argv) {
	// Инициализация GLUT и создание окна
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(10, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("LifeGame");

	// регистрация
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(processNormalKeys);
	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseMotion);
	//glutKeyboardUpFunc(processNormalKeys);

	if (argc > 1)name = argv[1];
	setup();

	FPSs.resize(3);
	FPSs = { 0.f, 0.f, 0.f };

	// основной цикл
	glutMainLoop();

	return 1;
}