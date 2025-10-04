#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

int main() {
    if (!glfwInit()) { std::cerr << "glfw init failed\n"; return 1; }

    // FFP가 편한 OpenGL 2.1 호환 컨텍스트
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* win = glfwCreateWindow(800, 600, "FFP Pyramid", nullptr, nullptr);
    if (!win) { std::cerr << "create window failed\n"; glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "glew init failed\n"; return 1; }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.13f, 0.20f, 0.33f, 1.0f);

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int w, h; glfwGetFramebufferSize(win, &w, &h);
        if (h == 0) h = 1;
        glViewport(0, 0, w, h);

        // ---- 투영행렬(원근) : glFrustum 사용 (fovy=60deg)
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float fovy = 60.0f;
        float znear = 0.1f, zfar = 10.0f;
        float aspect = (float)w / (float)h;
        float top = znear * std::tan(fovy * 0.5f * 3.1415926535f / 180.0f);
        float right = top * aspect;
        glFrustum(-right, right, -top, top, znear, zfar);

        // ---- 모델뷰행렬
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -2.5f);             // 카메라 뒤로
        double t = glfwGetTime();
        glRotatef((float)(t * 45.0), 0, 1, 0);       // Y축 회전
        glRotatef((float)(t * 25.0), 1, 0, 0);       // X축 회전

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- 삼각뿔 정점 (아펙스 + 바닥 3점)
        // 아펙스(위쪽)
        float ax = 0.0f, ay = 0.7f, az = 0.0f;
        // 바닥(삼각형, y=-0.5 평면)
        float b1x = 0.0f, b1y = -0.5f, b1z = 0.7f;
        float b2x = -0.6f, b2y = -0.5f, b2z = -0.4f;
        float b3x = 0.6f, b3y = -0.5f, b3z = -0.4f;

        // ---- 옆면 3개 (아펙스 + 바닥의 변 두 점) : 각 면마다 색 다르게
        glBegin(GL_TRIANGLES);
        // 옆면 1: apex-b1-b2
        glColor3f(1, 0, 0); glVertex3f(ax, ay, az);
        glColor3f(1, 0, 0); glVertex3f(b1x, b1y, b1z);
        glColor3f(1, 0, 0); glVertex3f(b2x, b2y, b2z);

        // 옆면 2: apex-b2-b3
        glColor3f(0, 1, 0); glVertex3f(ax, ay, az);
        glColor3f(0, 1, 0); glVertex3f(b2x, b2y, b2z);
        glColor3f(0, 1, 0); glVertex3f(b3x, b3y, b3z);

        // 옆면 3: apex-b3-b1
        glColor3f(0, 0, 1); glVertex3f(ax, ay, az);
        glColor3f(0, 0, 1); glVertex3f(b3x, b3y, b3z);
        glColor3f(0, 0, 1); glVertex3f(b1x, b1y, b1z);
        glEnd();

        // ---- 바닥면(삼각형) : 원하면 색 다르게
        glBegin(GL_TRIANGLES);
        glColor3f(1, 1, 0);
        glVertex3f(b1x, b1y, b1z);
        glVertex3f(b2x, b2y, b2z);
        glVertex3f(b3x, b3y, b3z);
        glEnd();

        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}