
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <tuple>
#include <sstream>
#include <regex>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>       // close()
#include <arpa/inet.h>    // sockaddr_in, inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

#include "figuras.h"

using namespace cv;
using namespace std;

void detectarRostros(Mat &frame);

// Ruta al clasificador Haar Cascade pre-entrenado
String rostros_cascade = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml";
CascadeClassifier face_cascade;

CascadeClassifier eye_cascade;


float total_rostros_width[10];
float total_x[3];
float total_y[3];
int total_rostros_n = 0;
int n_eje = 0;
float tamanio_rostro = 160;
int x_rostro = 160;
int y_rostro = 160;
int figura = 0; // 0 = ninguna, 1 = cubo, 2 = esfera, etc.


const int WIDTH = 800, HEIGHT = 600;
GLuint cameraTex;
VideoCapture cap;
Mat frame, flippedFrame;
float angX = 0, angY = 0;

// socket
constexpr int BUFFER_SIZE = 1024;
int sockfd;
struct sockaddr_in servaddr{}, cliaddr{};
socklen_t len;
    double t = 0;
    char buffer[BUFFER_SIZE];

    double roll, pitch, yaw;
    double ax, ay, az;


int camara = 0;


bool initCamera() {
    cap.open(camara, CAP_V4L2);
    if(!cap.isOpened()) cap.open(0, CAP_ANY);

    if(!cap.isOpened()) {
        std::cerr << "ERROR: No se pudo abrir la cámara" << std::endl;
        return false;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, WIDTH);
    cap.set(CAP_PROP_FRAME_HEIGHT, HEIGHT);

    glGenTextures(1, &cameraTex);
    glBindTexture(GL_TEXTURE_2D, cameraTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

int cada_cuanto = 0;
void updateCamera() {
    if(!cap.read(frame)) return;
        // Detectar rostros en el frame actual
    cada_cuanto++;
    if (cada_cuanto ==3) {
         detectarRostros(frame);
	 cada_cuanto = 0;
    }

    flip(frame, flippedFrame, 0);
    glBindTexture(GL_TEXTURE_2D, cameraTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, flippedFrame.cols, flippedFrame.rows,
                 0, GL_BGR, GL_UNSIGNED_BYTE, flippedFrame.data);
}

void drawBackground() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, cameraTex);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(0,0);
    glTexCoord2f(1,0); glVertex2f(WIDTH,0);
    glTexCoord2f(1,1); glVertex2f(WIDTH,HEIGHT);
    glTexCoord2f(0,1); glVertex2f(0,HEIGHT);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void drawCube(float size) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    // Material rojo brillante
    GLfloat mat_ambient[] = {0.7f, 0.0f, 0.0f, 1.0f};
    GLfloat mat_diffuse[] = {1.0f, 0.0f, 0.0f, 1.0f};
    GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat mat_shininess[] = {50.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glutSolidCube(size);

    // Rejilla blanca
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutWireCube(size * 1.01f);
}






// --- PARTE 1 --- Angulos desde acelerómetro
std::tuple<double, double, double> get_angles_from_accel(double gx, double gy, double gz) {
    double roll  = atan2(gy, gz);
    double pitch = atan2(-gx, sqrt(gy * gy + gz * gz));
    double yaw = 0.0;  // No se puede calcular con acelerómetro solo
    return {roll, pitch, yaw};
}

// --- PARTE 2 --- Extraer valores del string y calcular ángulos
std::tuple<double, double, double> parsear_y_calcular_angles(const std::string& linea) {
    std::regex valores_re(R"(\[([^\]]+)\])");
    std::smatch match;

    if (std::regex_search(linea, match, valores_re)) {
        std::string valores_str = match[1];  // "0.031050002,0.066,9.933001"
        std::stringstream ss(valores_str);
        std::string item;
        double v[3];
        int i = 0;

        while (std::getline(ss, item, ',') && i < 3) {
            v[i++] = std::stod(item);
        }

        //return get_angles_from_accel(v[0], v[1], v[2]);
        return {v[0], v[1], v[2]};
    } else {
        throw std::runtime_error("No se encontraron valores en el string.");
    }
}





// Convertir radianes a grados
float radianesAGrados(float radianes) {
    return radianes * (180.0 / M_PI);
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // DESDE EL SOCKET
        len = sizeof(cliaddr);
	for (int i=0; i<10; i++) {
	        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr*)&cliaddr, &len);
       		 if (n < 0) {
       		     perror("recvfrom");
       		     break;
       		 }

       		 buffer[n] = '\0'; // Terminar string
	}
	    try {
        std::tie(ax, ay, az) = parsear_y_calcular_angles(buffer);
        std::tie(roll, pitch, yaw) = get_angles_from_accel(ax, ay, az);

        // std::cout << buffer << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
     // FIN DESDE EL SOCKET





    updateCamera();
    drawBackground();

    // Configurar vista 3D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)WIDTH/HEIGHT, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0,0,5, 0,0,0, 0,1,0);

    // Dentro de display(), antes de drawCube()
glPushMatrix();


float posX = (x_rostro * 4.0 / 800.0)-2;
//float posX = 1.0f;  // Cambia estos valores
float posY = (y_rostro + tamanio_rostro*4)*(-3.0)/600.0 + 1.5;
std::cout << " y = " << y_rostro << " tam " << tamanio_rostro << " calc " << (y_rostro + tamanio_rostro*2.0) << "  en f " <<   ((y_rostro + tamanio_rostro)*3.0/600.0 - 1.5)  <<  "  " <<  (y_rostro + tamanio_rostro*4)*(-3.0)/600.0 - 1.5 << endl;
//float posY = -1.5f;
float posZ = 1.0f;
glTranslatef(posX, posY, posZ);  // <-- Esta es la línea clave

angX = radianesAGrados(roll);
angY = radianesAGrados(pitch);
glRotatef((-1)*angX,1,0,0);
glRotatef((-1)*angY,0,1,0);

float tamanio = tamanio_rostro / 300.0;
std::cout << " TAMANIO ROSTRO DIVIDIO " << tamanio << endl;

//drawCube(tamanio);
 // Dibujamos la figura según el nombre
    if (figura == 0) 
        dibujar_cubo(tamanio);
    else if (figura == 1) 
        dibujar_esfera(tamanio, 20, 20);
    else if (figura == 2) 
        dibujar_cono(tamanio, tamanio*2, 20, 10);
    else if (figura == 3) 
        dibujar_cilindro(tamanio, tamanio*2, 20, 10);
    else if (figura == 4) 
        dibujar_toroide(tamanio*0.3, tamanio, 20, 20);

glPopMatrix();

    glutSwapBuffers();
}


void animate(int value) {
    angX += 0.5f;
    angY += 0.3f;
    glutPostRedisplay();
    glutTimerFunc(16, animate, 0);
}

int main(int argc, char** argv) {

    // Valores por defecto
    // int camara = 0;

    // Verificar número de argumentos
    if (argc < 3) {
        std::cout << "Uso: " << argv[0] << " <numero_camara> <figura>" << std::endl;
        std::cout << "Ejemplo: " << argv[0] << " 1 cilindro" << std::endl;
        std::cout << "FIGURAS: cubo, esfera, cono, cilindro, toroide" << std::endl;
        return 1;
    }

    // Procesar argumento de cámara
    try {
        camara = std::stoi(argv[1]);
        if (camara < 0 || camara > 2) {
            std::cerr << "Error: La cámara debe ser 0, 1 o 2" << std::endl;
            return 1;
        }
    } catch (...) {
        std::cerr << "Error: El primer argumento debe ser un número (0, 1 o 2)" << std::endl;
        return 1;
    }

    // Procesar argumento de figura
    if (strcmp(argv[2], "cubo") == 0) {
        figura = 0;
    } else if (strcmp(argv[2], "esfera") == 0) {
        figura = 1;
    } else if (strcmp(argv[2], "cono") == 0) {
        figura = 2;
    } else if (strcmp(argv[2], "cilindro") == 0) {
        figura = 3;
    } else if (strcmp(argv[2], "toroide") == 0) {
        figura = 4;
    } else {
        std::cerr << "Error: Figura no reconocida. Opciones válidas: cubo, esfera, cono, cilindro, toroide" << std::endl;
        return 1;
    }

    // Mostrar los valores seleccionados
    std::cout << "Configuración seleccionada:" << std::endl;
    std::cout << "Cámara: " << camara << std::endl;
    std::cout << "Figura: " << argv[2] << " (código: " << figura << ")" << std::endl;





	eye_cascade.load("haarcascade_eye.xml");


  // Verificar si se puede cargar el clasificador
    if (!face_cascade.load(rostros_cascade)) {
        cerr << "Error al cargar el clasificador de rostros" << endl;
        return -1;
    }	
	// SOCKET
    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error al crear socket");
        return EXIT_FAILURE;
    }

    // Configurar dirección del servidor
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;  // Escuchar en todas las interfaces
    servaddr.sin_port = htons(5005);        // Puerto 5005




    // Bind
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error en bind");
        close(sockfd);
        return EXIT_FAILURE;
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Realidad Aumentada");

    if(!initCamera()) {
        std::cerr << "Usando fondo estático (falló la cámara)" << std::endl;
    }

    // Configuración de luz
    GLfloat light_pos[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,0,1);

    glutDisplayFunc(display);
    glutTimerFunc(0, animate, 0);

    glutMainLoop();

    if(cap.isOpened()) cap.release();

    // CERRAR SOCKET
    close(sockfd);


    return 0;
}





int umbral_rostros = 0;


void detectarRostros(Mat &frame) {

    Mat frame_gris;
    vector<Rect> rostros;

    // Convertir a escala de grises (la detección funciona mejor en gris)
    cvtColor(frame, frame_gris, COLOR_BGR2GRAY);
    // Ecualizar el histograma para mejorar el contraste
    equalizeHist(frame_gris, frame_gris);

    if (face_cascade.empty()) {
    cerr << "ERROR: ¡El clasificador no se cargó!" << endl;
    return;
}

    // Detectar rostros
    face_cascade.detectMultiScale(
        frame_gris, rostros,
        1.3, 3, 0|CASCADE_SCALE_IMAGE,
        Size(50, 50)
    );
	/*
	 1.1 - Factor de escala (valores típicos: 1.01 a 1.3). Valores más altos = menos detecciones pero más rápido.

3 - minNeighbors (valores típicos: 3 a 6). Filtra falsos positivos. Mayor valor = más estricto.

0|CASCADE_SCALE_IMAGE - Flags (combinables con OR):

CASCADE_SCALE_IMAGE (recomendado)

CASCADE_FIND_BIGGEST_OBJECT (solo el más grande)

Size(30, 30) - Tamaño mínimo del objeto a detectar (ajustar según distancia a la cámar
*/
 
	for (size_t i = 0; i < rostros.size(); i++) {
    float aspect_ratio = (float)rostros[i].width / rostros[i].height;
    if (aspect_ratio < 0.7 || aspect_ratio > 1.5) {
        rostros.erase(rostros.begin() + i);
        i--;  // Ajustar índice después de borrar
    }
}
	int n = 0; // para buscar el mayor
	float n_width = 0;
    // Dibujar elipses alrededor de los rostros
    for (size_t i = 0; i < rostros.size(); i++) {
	    if (rostros[i].width > n_width)
		    n = i;


//	    std::cout << " TAMAÑO " << rostros[i].width << endl;
 //       Point centro(rostros[i].x + rostros[i].width/2, rostros[i].y + rostros[i].height/2);
  //      ellipse(
   //         frame, centro,
    //        Size(rostros[i].width/2, rostros[i].height/2),
     //       0, 0, 360,
      //      Scalar(255, 0, 255), 4
       // );
    }
	if (rostros.size() == 0)
		return;
	total_rostros_width[total_rostros_n] = rostros[n].width;
	x_rostro = rostros[n].x + rostros[n].width/2;
	y_rostro = rostros[n].y + rostros[n].height/2;
	total_x[n_eje] = rostros[n].x + rostros[n].width/2;
	total_y[n_eje] = rostros[n].y + rostros[n].height/2;
	total_rostros_n++;
	if (total_rostros_n == 10)
		total_rostros_n = 0;
	n_eje++;
	if (n_eje == 3)
		n_eje = 0;

	float total = 0;
	float sum_x = 0;
	float sum_y = 0;
	for (int i=0; i<10; i++) {
		total += total_rostros_width[i];
	}
	for (int i=0; i<3; i++) {
		sum_x += total_x[i];
		sum_y += total_y[i];
	}
	tamanio_rostro = total / 10;
	//x_rostro = sum_x / 3;
	//y_rostro = sum_y / 3;
}
