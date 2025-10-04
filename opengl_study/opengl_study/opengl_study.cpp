#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cstdio>
#include <cmath>

#include <opencv2/opencv.hpp>



int main() {
    const int W = 800, H = 600;

    // 1) GLFW: 보이지 않는 창(컨텍스트만 필요)
    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // FFP가 편한 OpenGL 2.1 호환 컨텍스트
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* win = glfwCreateWindow(32, 32, "hidden", nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "create window failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::fprintf(stderr, "glewInit failed\n"); return 1; }

    // 2) 오프스크린 FBO (컬러 텍스처 + 깊이/스텐실)
    GLuint fbo = 0, color = 0, rbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, W, H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "FBO incomplete\n");
        return 1;
    }

    // 3) 렌더 타깃: FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, W, H);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.13f, 0.20f, 0.33f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---- 기본 좌표계(좌하단 원점) 그대로: 2D 직교 투영 ----
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // left=0, right=W, bottom=0, top=H  → y가 위로 증가 (OpenGL 기본)
    glOrtho(0, W, 0, H, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 4) 삼각형 (좌하단 원점 기준)
    //    위 꼭짓점은 화면 "위쪽"에 있으려면 y가 크게(H에 가까움)
    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0); glVertex2f(W * 0.5f, H - 80.0f);  // 위
    glColor3f(0, 1, 0); glVertex2f(160.0f, 80.0f);    // 좌
    glColor3f(0, 0, 1); glVertex2f(W - 160.0f, 80.0f);  // 우
    glEnd();

    // 5) 픽셀 읽기 → OpenCV로 저장
    // OpenGL은 (0,0)이 좌하단 → OpenCV(좌상단)로 저장할 때는 수직 플립 필요
    std::vector<unsigned char> rgba(W * H * 4);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

    cv::Mat imgRGBA(H, W, CV_8UC4, rgba.data());
    cv::Mat flipped, bgr;
    cv::flip(imgRGBA, flipped, 0);               // 수직 뒤집기(행 순서 보정)
    cv::cvtColor(flipped, bgr, cv::COLOR_RGBA2BGR);
    if (!cv::imwrite("triangle_offscreen.png", bgr))
        std::fprintf(stderr, "imwrite failed\n");
    else
        std::printf("Saved: triangle_offscreen.png\n");

    // 6) 정리
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &color);
    glDeleteFramebuffers(1, &fbo);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}