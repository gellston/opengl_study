#include "render.h"
#include "implRenderRuntime.h"


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <thread>
#include <mutex>
#include <math.h>
#include <algorithm>


#pragma region Impl
namespace hv {
	namespace v1 {
		struct impl_render {
		public:

			std::thread renderThread;
			std::mutex mtx;
			std::condition_variable condV;
			bool running = true;
			bool shutdown = false;

			int targetWidth = 800;
			int targetHeight = 600;

			hv::v1::clearColor clear;

			bool sizeDirty = true;
			bool clearDirty = true;
			bool viewDirty = true;
			bool dataDirty = false;

			GLFWwindow* win = nullptr;
			GLuint fbo = 0;
			GLuint color = 0;
			GLuint depthstencil = 0;

			int width = 0;
			int height = 0;
			bool hasAttachments = false;

			std::vector<unsigned char> rgba;


			frameCallback callback;
			frameID nextID = 1;

			float angleDeg = 20.0f;
            float zoom = 1.0f;
            float panX = 0.0f;
            float panY = 0.0f;
            float refDistance = 3.0f;

            float yawDeg = 0.0f;
            float pitchDeg = 0.0f;

            int axesSizePx = 96;
            int axesMarginPx = 12;
            bool showAxes = true;


            static constexpr float kMinZoom = 0.05f;
            static constexpr float kMinDistance = 0.3f;

            static std::mutex g_createMtx;
           



			void threadMain() {
                // [1] GLFW/컨텍스트 생성(Windows/Linux 전제. macOS면 메인스레드 생성 필요)
                {
                    // Thread safe 초기화 직렬화.
                    std::lock_guard<std::mutex> lock(g_createMtx);
                    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return; }
                    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, hv::v1::global_glMajor);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, hv::v1::global_glMinor);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
                    glfwWindowHint(GLFW_SAMPLES, 4);
                    glEnable(GL_MULTISAMPLE);
                    win = glfwCreateWindow(32, 32, "hidden", nullptr, nullptr);
                }

                if (!win) { 
                    std::fprintf(stderr, "create window failed\n"); 
                    glfwTerminate(); 
                    return; 
                }

                glfwMakeContextCurrent(win);
                glewExperimental = GL_TRUE;
                if (glewInit() != GLEW_OK) { 
                    std::fprintf(stderr, "glewInit failed\n"); 
                    return; 
                }



                // [2] FBO 1회 생성 (첨부물은 리사이즈 시 재할당)
                glGenFramebuffers(1, &fbo);
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);

                // Render Loop 진입
                while (this->running)
                {
                    // [A] 깨울 조건: 더티 발생 또는 종료 요청
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        condV.wait(lock, [&] { return shutdown || sizeDirty || clearDirty || viewDirty || dataDirty; });
                        if (shutdown) break;
                    }

                    // [B] 리사이즈 반영
                    if (sizeDirty)
                    {
                        // 0 또는 음수 크기면 렌더 스킵(첨부물 해제 옵션)
                        if (targetWidth <= 0 || targetHeight <= 0)
                        {
                            if (hasAttachments) {
                                glDeleteRenderbuffers(1, &depthstencil); 
                                depthstencil = 0;
                                glDeleteTextures(1, &color);             
                                color = 0;
                                hasAttachments = false;
                            }
                            rgba.clear();
                            sizeDirty = false;
                            continue; // 이번 프레임은 건너뜀
                        }

                        // 첨부물이 없다면 새로 생성(있다면 재할당만)
                        if (!hasAttachments)
                        {
                            glGenTextures(1, &color);
                            glBindTexture(GL_TEXTURE_2D, color);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

                            glGenRenderbuffers(1, &depthstencil);
                            glBindRenderbuffer(GL_RENDERBUFFER, depthstencil);
                            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthstencil);
                            hasAttachments = true;
                        }

                        // 스토리지 재할당
                        width = targetWidth; 
                        height = targetHeight;

                        glBindTexture(GL_TEXTURE_2D, color);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                        glBindRenderbuffer(GL_RENDERBUFFER, depthstencil);
                        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

                        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                            std::fprintf(stderr, "FBO incomplete after resize\n");
                            // 치명 오류. 다음 루프에서 다시 시도할 수도 있지만 여기선 스킵.
                            sizeDirty = false;
                            continue;
                        }

                        glViewport(0, 0, width, height);
                        rgba.assign(static_cast<size_t>(width) * height * 4, 0);

                        sizeDirty = false;
                        viewDirty = true; // 해상도 의존 투영이면 뷰 갱신 유도
                    }

                    // [C] 공통 준비(클리어)
                    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                    glViewport(0, 0, width, height);
                    if (clearDirty) {
                        glClearColor(clear.r, clear.g, clear.b, clear.a);
                        clearDirty = false;
                    }
                    glEnable(GL_DEPTH_TEST);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // [D] 뷰/투영 설정(FFP, 간단한 퍼스펙티브)
                    if (viewDirty) {
                        const float aspect = (height > 0) ? float(width) / float(height) : 1.0f;

                        // FOV 고정 범위
                        /*if (fovY_deg < 30.0f) fovY_deg = 30.0f;
                        if (fovY_deg > 70.0f) fovY_deg = 70.0f;*/
                        auto fovY_deg = 70.0f;

                        // zoom → distance
                        const float z = (zoom < kMinZoom) ? kMinZoom : zoom; // zoom>1 == 확대
                        float distance = refDistance / z;
                        if (distance < kMinDistance) distance = kMinDistance;

                        // near/far는 distance 비례
                        const float n = std::max(0.01f, distance * 0.05f);
                        const float f = std::max(n + 1.0f, distance * 20.0f);

                        // 투영
                        const float fovY = fovY_deg * (3.1415926535f / 180.0f);
                        const float top = n * std::tan(fovY * 0.5f);
                        const float right = top * aspect;

                        glMatrixMode(GL_PROJECTION);
                        glLoadIdentity();
                        glFrustum(-right, right, -top, top, n, f);

                        glMatrixMode(GL_MODELVIEW);
                        glLoadIdentity();
                        glTranslatef(-panX, -panY, -distance); // 팬 + dolly(distance)
                        glRotatef(-pitchDeg, 1, 0, 0);
                        glRotatef(-yawDeg, 0, 1, 0);

                        viewDirty = false;
                    }


                    // [E] 데이터 갱신(스켈레톤: 고정 지오메트리 → 아무것도 안 함)
                    if (dataDirty) {
                        // TODO: 여기에서 포인트클라우드/라인 등의 VBO 업로드/갱신을 수행
                        dataDirty = false;
                    }

                    // [F] *** 실제 렌더링: 고정 "삼각뿔" (Immediate Mode) ***
                    //  - 스켈레톤이므로 셰이더/VBO 없이 즉시 모드로 간단히
                    //glPushMatrix();
                    //glRotatef(angleDeg, 0.577f, 0.577f, 0.577f); // 살짝 비틀어진 축
                    // 정점(삼각뿔 4개 면)
                    const GLfloat v0[3] = { 0.f,  1.f,  0.f };  // 꼭짓점
                    const GLfloat v1[3] = { -1.f, -1.f,  1.f };
                    const GLfloat v2[3] = { 1.f, -1.f,  1.f };
                    const GLfloat v3[3] = { 0.f, -1.f, -1.f };

                    glBegin(GL_TRIANGLES);
                    // 옆면1: v0-v1-v2
                    glColor3f(1, 0, 0); 
                    glVertex3fv(v0);
                    glColor3f(0, 1, 0); 
                    glVertex3fv(v1);
                    glColor3f(0, 0, 1); 
                    glVertex3fv(v2);

                    // 옆면2: v0-v2-v3
                    glColor3f(1, 0, 0); 
                    glVertex3fv(v0);
                    glColor3f(0, 0, 1); 
                    glVertex3fv(v2);
                    glColor3f(1, 1, 0); 
                    glVertex3fv(v3);

                    // 옆면3: v0-v3-v1
                    glColor3f(1, 0, 0); 
                    glVertex3fv(v0);
                    glColor3f(1, 1, 0); 
                    glVertex3fv(v3);
                    glColor3f(0, 1, 0); 
                    glVertex3fv(v1);

                    // 바닥: v1-v3-v2
                    glColor3f(0.3f, 0.3f, 0.3f); 
                    glVertex3fv(v1);
                    glColor3f(0.3f, 0.3f, 0.3f); 
                    glVertex3fv(v3);
                    glColor3f(0.3f, 0.3f, 0.3f); 
                    glVertex3fv(v2);
                    glEnd();
                    //glPopMatrix();


                    // Show Axies
                    if (showAxes) {
                        // 기존 뷰포트 백업
                        GLint prevViewport[4];
                        glGetIntegerv(GL_VIEWPORT, prevViewport);

                        // 상태 백업 (SCISSOR까지 포함)
                        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_SCISSOR_BIT);

                        // 작은 뷰포트(좌하단). 우상단이면 vy/vx만 바꾸세요.
                        const GLint   vx = axesMarginPx;
                        const GLint   vy = axesMarginPx; // 우상단: height - axesMarginPx - axesSizePx
                        const GLsizei vs = axesSizePx;
                        glViewport(vx, vy, vs, vs);

                        // 이 뷰포트 영역의 depth만 클리어(메인 depth 보존)
                        glEnable(GL_SCISSOR_TEST);
                        glScissor(vx, vy, vs, vs);
                        glClearDepth(1.0);
                        glClear(GL_DEPTH_BUFFER_BIT);
                        glDisable(GL_SCISSOR_TEST);

                        // 축끼리 자연스러운 가림을 위해 depth ON
                        glEnable(GL_DEPTH_TEST);
                        glDepthFunc(GL_LESS);
                        glDepthMask(GL_TRUE);

                        // 라인 품질(선택)
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        glEnable(GL_LINE_SMOOTH);
                        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                        glLineWidth(2.0f);

                        // 별도 투영행렬(작은 FOV 권장: 35도 전후)
                        glMatrixMode(GL_PROJECTION);
                        glPushMatrix();
                        glLoadIdentity();
                        {
                            const float fov_deg = 35.0f;
                            const float n = 0.01f, f = 10.0f;
                            const float fov = fov_deg * 3.1415926535f / 180.0f;
                            const float top = n * std::tan(fov * 0.5f);
                            const float right = top; // 정사각 뷰포트 → aspect=1
                            glFrustum(-right, right, -top, top, n, f);
                        }

                        // 별도 모델뷰행렬(카메라 회전만 반영: 팬/줌 무시)
                        glMatrixMode(GL_MODELVIEW);
                        glPushMatrix();
                        glLoadIdentity();
                        glTranslatef(0.0f, 0.0f, -2.0f);
                        glRotatef(-pitchDeg, 1, 0, 0);  // 카메라 역회전과 동일 부호
                        glRotatef(-yawDeg, 0, 1, 0);

                        // 3축 그리기 (X=Red, Y=Green, Z=Blue)
                        const float L = 0.8f;  // 축 길이
                        glBegin(GL_LINES);
                        // X (→)
                        glColor3f(1, 0, 0); 
                        glVertex3f(0, 0, 0); 
                        glVertex3f(L, 0, 0);
                        // Y (↑)  ※ OpenGL 기준 +Y가 위입니다.
                        glColor3f(0, 1, 0); 
                        glVertex3f(0, 0, 0); 
                        glVertex3f(0, L, 0);
                        // Z (앞)
                        glColor3f(0, 0, 1); 
                        glVertex3f(0, 0, 0); 
                        glVertex3f(0, 0, L);
                        glEnd();

                        // 복원
                        glPopMatrix();                 // MODELVIEW
                        glMatrixMode(GL_PROJECTION);
                        glPopMatrix();                 // PROJECTION
                        glMatrixMode(GL_MODELVIEW);
                        glPopAttrib();                 // 상태 복원
                        glViewport(prevViewport[0], prevViewport[1],
                                   prevViewport[2], prevViewport[3]);
                    }


                    // [G] Readback (동기, 상단-원점 보정은 소비자에서 처리 권장)
                    if (hasAttachments && !rgba.empty())
                    {
                        glReadBuffer(GL_COLOR_ATTACHMENT0);
                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

                        // TODO: FrameReady 콜백 호출 지점
                         if (this->callback) {
                             frameView view{};
                             view.id     = nextID++;
                             view.width  = width;
                             view.height = height;
                             view.stride = width*4;
                             view.format = hv::v1::pixelFormat::RGBA8;
                             view.data = this->rgba.data();
                             this->callback(view);
                         }
                    }

                    // 온디맨드 스켈레톤: 추가 더티가 없으면 잠깐 쉼
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        if (!sizeDirty && !clearDirty && !viewDirty && !dataDirty)
                            condV.wait_for(lock, std::chrono::milliseconds(1));
                    }
                }


                // Render Loop 탈출
                // [종료] GL 자원 정리(컨텍스트 current인 같은 스레드에서)
                if (hasAttachments) {
                    glDeleteRenderbuffers(1, &depthstencil);
                    depthstencil = 0;
                    glDeleteTextures(1, &color);             
                    color = 0;
                    hasAttachments = false;
                }

                if (fbo) { 
                    glDeleteFramebuffers(1, &fbo); 
                    fbo = 0; 
                }

                {
                    // Thread safe 해제 직렬화.
                    std::lock_guard<std::mutex> lk(g_createMtx);
                    if (win) {
                        glfwMakeContextCurrent(nullptr);
                        glfwDestroyWindow(win);
                        win = nullptr;
                    }
                    glfwTerminate();
                }

			}

		};
	}
}

std::mutex  hv::v1::impl_render::g_createMtx;
#pragma endregion




#pragma region Constructor
hv::v1::render::render() : impl(std::make_unique<hv::v1::impl_render>()){

    impl->renderThread = std::thread([this] {
        impl->threadMain();
    });
    
}
#pragma endregion

#pragma region Destructor
hv::v1::render::~render() {
    this->cleanUp();
}
#pragma endregion

#pragma region Public Functions
void hv::v1::render::cleanUp() {
    if (!impl) return;

    //lock
    {
        std::scoped_lock lock(this->impl->mtx);
        if (impl->shutdown) return;
        impl->shutdown = true;
        impl->condV.notify_all();
    }

    if (impl->renderThread.joinable())
        impl->renderThread.join();

    impl.reset();
}

void hv::v1::render::resizeOutput(const hv::v1::outputDesc& out) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->targetWidth = out.width;
        impl->targetHeight = out.height;
        impl->sizeDirty = true;
    }
    impl->condV.notify_all();
}

void hv::v1::render::setClearColor(const hv::v1::clearColor& clear) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->clear.r = clear.r;
        impl->clear.g = clear.g;
        impl->clear.b = clear.b;
        impl->clear.a = clear.a;
        impl->clearDirty = true;
    }
    impl->condV.notify_all();
}

void hv::v1::render::registerFrameCallback(hv::v1::frameCallback callback) {
    if (!impl) return;
    std::scoped_lock lock(this->impl->mtx);
    this->impl->callback = std::move(callback);
}

void hv::v1::render::releaseFrame(hv::v1::frameID id) {
    //링버퍼가 나온다면  추가할거임
}


void hv::v1::render::invalidateView() {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        this->impl->viewDirty = true;
    }
    this->impl->condV.notify_all();
}

void hv::v1::render::invalidateData() {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        this->impl->dataDirty = true;
    }
    this->impl->condV.notify_all();
}

void hv::v1::render::invalidateClear() {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        this->impl->clearDirty = true;
    }
    this->impl->condV.notify_all();
}




void hv::v1::render::zoomBy(float zoom) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->zoom *= zoom;
        impl->zoom = (impl->zoom < impl->kMinZoom) ? impl->kMinZoom : impl->zoom;
        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}
void hv::v1::render::setZoom(float zoom) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->zoom = zoom;
        impl->zoom = (impl->zoom < impl->kMinZoom) ? impl->kMinZoom : impl->zoom;
        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}


void hv::v1::render::setPan(float x, float y) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->panX = x;
        impl->panY = y;
        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}
void hv::v1::render::translateBy(float dx, float dy) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->panX += dx;
        impl->panY += dy;
        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}


void hv::v1::render::setPanPixel(float x_pixels, float y_pixels) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        if (impl->width <= 0 || impl->height <= 0) return;

        // 현재 distance(줌 반영)
        float z = std::max(impl->zoom, impl->kMinZoom);
        float distance = impl->refDistance / z;
        if (distance < impl->kMinDistance) distance = impl->kMinDistance;

        // 투영 파라미터 (viewDirty 분기 밖에서도 동일 값 사용)
        const float aspect = (impl->height > 0) ? float(impl->width) / float(impl->height) : 1.0f;
        const float fovY = 70.0f * 3.1415926535f / 180.0f; // 현재 고정 FOV

        // 픽셀 → 월드 스케일
        const float sx_per_px = 2.0f * distance * std::tan(fovY * 0.5f) * aspect / float(impl->width);
        const float sy_per_px = 2.0f * distance * std::tan(fovY * 0.5f) / float(impl->height);

        // 화면 좌표계(y 아래가 +)를 월드(y 위가 +)로 맞추고 싶다면 dy는 부호 반전
        impl->panX = (x_pixels * sx_per_px);
        impl->panY = (y_pixels * sy_per_px);

        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}

void hv::v1::render::translatePixelBy(float dx_pixels, float dy_pixels) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        if (impl->width <= 0 || impl->height <= 0) return;

        // 현재 distance(줌 반영)
        float z = std::max(impl->zoom, impl->kMinZoom);
        float distance = impl->refDistance / z;
        if (distance < impl->kMinDistance) distance = impl->kMinDistance;

        // 투영 파라미터 (viewDirty 분기 밖에서도 동일 값 사용)
        const float aspect = (impl->height > 0) ? float(impl->width) / float(impl->height) : 1.0f;
        const float fovY = 70.0 * 3.1415926535f / 180.0f; // 현재 고정 FOV

        // 픽셀 → 월드 스케일
        const float sx_per_px = 2.0f * distance * std::tan(fovY * 0.5f) * aspect / float(impl->width);
        const float sy_per_px = 2.0f * distance * std::tan(fovY * 0.5f) / float(impl->height);

        // 화면 좌표계(y 아래가 +)를 월드(y 위가 +)로 맞추고 싶다면 dy는 부호 반전
        impl->panX += (dx_pixels * sx_per_px);
        impl->panY -= (dy_pixels * sy_per_px);

        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}


void hv::v1::render::setOrbitAngle(float yawDeg, float pitchDeg) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->yawDeg = yawDeg;
        impl->pitchDeg = pitchDeg;
        //impl->pitchDeg = std::clamp(impl->pitchDeg, -89.0f, 89.0f);
        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}

void hv::v1::render::orbitBy(float dyawDeg, float dpitchDeg) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->yawDeg += dyawDeg;
        impl->pitchDeg += dpitchDeg;

        //impl->pitchDeg = std::clamp(impl->pitchDeg, -89.0f, 89.0f);

        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}

#pragma endregion


