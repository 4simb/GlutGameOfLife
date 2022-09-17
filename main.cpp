#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <chrono>
#include <vector>
#include <Windows.h>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include "BMP.h"

//#define DELAY 1
#define SPEED 60.f
#define PATH "Debug/img2.bmp"

float distance = -100.f;

int WIDTH = 120;
int HEIGHT = 39;

int winWidth = 1900, winHeight = 980;

float grid_size;

static int frame;
int FPS = 0;
long int elapsed_time = 0;

bool pause = false;

std::vector<std::vector<bool>> field(WIDTH + 1, std::vector<bool>(HEIGHT + 1));
std::vector<std::vector<bool>> new_field(WIDTH + 1, std::vector<bool>(HEIGHT + 1));

std::string name = PATH;
std::string finish_code;

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

void setup() {
	BMP BMPfield(name);
	//if (BMPfield.get_heigt() != HEIGHT || BMPfield.get_width() != WIDTH) throw std::runtime_error("Error! Invalid size of BMP image.");
	WIDTH = BMPfield.get_width();
	HEIGHT = BMPfield.get_heigt();
	int line_size = BMPfield.get_bytes_per_raw();
	int file_size = line_size * HEIGHT;

	std::ifstream img{ name, std::ios_base::binary };

	if (!img.good()) throw std::runtime_error("Error! Couldn't open the file.");

	img.seekg(0x3e); // 62 bytes, start of pixels info

	std::vector<unsigned char> img_data(file_size);

	img.read((char*)&img_data[0], file_size);
	img.close();

	//field.resize(WIDTH);
	//new_field.resize(WIDTH);

	field = std::vector<std::vector<bool>>(WIDTH + 1, std::vector<bool>(HEIGHT + 1));
	new_field = std::vector<std::vector<bool>>(WIDTH + 1, std::vector<bool>(HEIGHT + 1));

	//*
	for (int y = HEIGHT - 1; y >= 0; y--) {
		for (int x = 0; x < WIDTH; x++) {
			int pos = y * line_size + x / 8; // currently readable byte
			int bit = 1 << (7 - x % 8);
			//std::cout << "Import " << name << "  size: " << img_data.size() << '\t' << "  byte n:  " << pos << '\n';
			//std::cout << img_data.size() << '\t' << pos << '\t' << (int)img_data[pos] << '\n'; //1695, 3rd iter - fail
			//
			int pixel = !((img_data[pos] & bit) > 0); // 1 - black, 0 - white
			field[x][HEIGHT - y - 1] = pixel;
		}
	}
	//*/

	timer_start = timer_end = step_time = std::chrono::high_resolution_clock::now();
}

void draw_field() {
	grid_size = min((float)(winWidth - 200) / (float)WIDTH, (float)winHeight / (float)HEIGHT);

	for (int y = 0; y < HEIGHT; y++) {
		glBegin(GL_LINE_STRIP);
		glPoint(0, y * grid_size);
		glPoint(WIDTH * grid_size, y * grid_size);
		glEnd();

		for (int x = 0; x < WIDTH; x++) {
			glBegin(GL_LINE_STRIP);
			glPoint(x * grid_size, 0);
			glPoint(x * grid_size, HEIGHT * grid_size);
			glEnd();

			if (field[x][y]) {
				glColor3f(0.5, 0.5, 0.5);
				glBegin(GL_QUADS);
				glPoint(((float)x + 0.1) * grid_size, ((float)y + 0.1) * grid_size);
				glPoint(((float)x + 0.9) * grid_size, ((float)y + 0.1) * grid_size);
				glPoint(((float)x + 0.9) * grid_size, ((float)y + 0.9) * grid_size);
				glPoint(((float)x + 0.1) * grid_size, ((float)y + 0.9) * grid_size);
				glEnd();	
			}
			glColor3f(0.15, 0.15, 0.15);
		}
	}
	glBegin(GL_LINE_STRIP);
	glPoint(WIDTH * grid_size, 0);
	glPoint(WIDTH * grid_size, HEIGHT * grid_size);
	glPoint(0, HEIGHT * grid_size);
	glPoint(WIDTH * grid_size, HEIGHT * grid_size);
	glEnd();

}
// game step
int field_analyse() {
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
				new_field[x][y] = 1;
				alive++;
			} else if (field[x][y] == 1 && (neighbors == 2 || neighbors == 3)) { // still alive
				new_field[x][y] = 1;
				alive++;
			} else new_field[x][y] = 0; // die
		}
	}

	field = new_field;

	return alive;
}

void renderScene(void) {
	timer_end = std::chrono::high_resolution_clock::now();
	static int alive;

	if (std::chrono::duration_cast<std::chrono::microseconds>(timer_end - step_time).count() > 1e6f / SPEED) {
		alive = field_analyse();
		step_time = timer_end;
	} else return;
	
	if (alive == 0) {
		finish_code = "Everyone died.";
		return;
	}

	// очистка буфера цвета и глубины
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1, 0.1, 0.1, 1.0);
	// обнуление трансформации
	glLoadIdentity();
	
	glColor3f(0.15, 0.15, 0.15);
	glLineWidth(1);

	draw_field();

	glColor3f(1.0, 1.0, 1.0);
	char chFPS[6];
	char chFrame[10];
	_itoa(FPS, chFPS, 10);
	_itoa(frame, chFrame, 10);
	drawString(WIDTH * grid_size + 20, winHeight - 30, -1, "FPS: ");
	drawString(WIDTH * grid_size + 75, winHeight - 30, -1, chFPS);

	drawString(WIDTH * grid_size + 20, winHeight - 60, -1, "Frame:");
	drawString(WIDTH * grid_size + 100, winHeight - 60, -1, chFrame);

	glutSwapBuffers();

	frame++;

	elapsed_time = (std::chrono::duration_cast<std::chrono::microseconds>(timer_end - timer_start).count());
	FPS = 1e6 / elapsed_time;
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

	if (argc > 1)name = argv[1];
	setup();

	//td::vector<std::vector<bool>> field(WIDTH + 1, std::vector<bool>(HEIGHT + 1));
	//std::vector<std::vector<bool>> new_field(WIDTH + 1, std::vector<bool>(HEIGHT + 1));

	// основной цикл
	glutMainLoop();

	return 1;
}