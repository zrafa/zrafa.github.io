
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <string>


// Función para dibujar un cubo con rejilla
void dibujar_cubo(float size) {
    size *= 0.5f; // Ajustamos para que el tamaño sea el lado completo

    // Primero dibujamos el cubo sólido rojo
    glColor3f(1.0f, 0.0f, 0.0f); // Color rojo
    glutSolidCube(size * 2); // Dibuja un cubo sólido

    // Luego dibujamos la rejilla blanca
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para la rejilla

    // Cara frontal
    for(float i = -size; i <= size; i += size/5.0f) {
        // Líneas verticales
        glVertex3f(i, -size, size);
        glVertex3f(i, size, size);

        // Líneas horizontales
        glVertex3f(-size, i, size);
        glVertex3f(size, i, size);
    }

    // Cara trasera
    for(float i = -size; i <= size; i += size/5.0f) {
        // Líneas verticales
        glVertex3f(i, -size, -size);
        glVertex3f(i, size, -size);

        // Líneas horizontales
        glVertex3f(-size, i, -size);
        glVertex3f(size, i, -size);
    }

    // Conectar caras frontales y traseras
    for(float i = -size; i <= size; i += size/5.0f) {
        for(float j = -size; j <= size; j += size/5.0f) {
            glVertex3f(i, j, size);
            glVertex3f(i, j, -size);

            glVertex3f(i, size, j);
            glVertex3f(i, -size, j);

            glVertex3f(size, i, j);
            glVertex3f(-size, i, j);
        }
    }
    glEnd();
}

// Función para dibujar una esfera con rejilla
void dibujar_esfera(float radius, int slices, int stacks) {
    // Primero dibujamos la esfera sólida roja
    glColor3f(1.0f, 0.0f, 0.0f); // Color rojo
    glutSolidSphere(radius, slices, stacks);

    // Luego dibujamos la rejilla blanca
    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para la rejilla
    glutWireSphere(radius, slices, stacks);
}

// Función para dibujar un cono con rejilla
void dibujar_cono(float base, float height, int slices, int stacks) {
    // Primero dibujamos el cono sólido rojo
    glColor3f(1.0f, 0.0f, 0.0f); // Color rojo
    glutSolidCone(base, height, slices, stacks);

    // Luego dibujamos la rejilla blanca
    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para la rejilla
    glutWireCone(base, height, slices, stacks);
}

// Función para dibujar un cilindro con rejilla
void dibujar_cilindro(float radius, float height, int slices, int stacks) {
    GLUquadricObj *quadric = gluNewQuadric();

    // Primero dibujamos el cilindro sólido rojo
    glColor3f(1.0f, 0.0f, 0.0f); // Color rojo
    gluQuadricDrawStyle(quadric, GLU_FILL);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Orientación correcta
    gluCylinder(quadric, radius, radius, height, slices, stacks);
    glPopMatrix();

    // Luego dibujamos la rejilla blanca
    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para la rejilla
    gluQuadricDrawStyle(quadric, GLU_LINE);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Orientación correcta
    gluCylinder(quadric, radius, radius, height, slices, stacks);
    glPopMatrix();

    gluDeleteQuadric(quadric);
}

// Función para dibujar un toroide con rejilla
void dibujar_toroide(float innerRadius, float outerRadius, int sides, int rings) {
    // Primero dibujamos el toroide sólido rojo
    glColor3f(1.0f, 0.0f, 0.0f); // Color rojo
    glutSolidTorus(innerRadius, outerRadius, sides, rings);

    // Luego dibujamos la rejilla blanca
    glColor3f(1.0f, 1.0f, 1.0f); // Color blanco para la rejilla
    glutWireTorus(innerRadius, outerRadius, sides, rings);
}

