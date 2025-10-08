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
			bool dataDirty = true;

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
                        auto fovY_deg = 55.0f;

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
                    glPushMatrix();
                    //glRotatef(angleDeg, 0.577f, 0.577f, 0.577f); // 살짝 비틀어진 축
                    // 정점(삼각뿔 4개 면)
                    const GLfloat v0[3] = { 0.f,  1.f,  0.f };  // 꼭짓점
                    const GLfloat v1[3] = { -1.f, -1.f,  1.f };
                    const GLfloat v2[3] = { 1.f, -1.f,  1.f };
                    const GLfloat v3[3] = { 0.f, -1.f, -1.f };

                    glBegin(GL_TRIANGLES);
                    // 옆면1: v0-v1-v2
                    glColor3f(1, 0, 0); glVertex3fv(v0);
                    glColor3f(0, 1, 0); glVertex3fv(v1);
                    glColor3f(0, 0, 1); glVertex3fv(v2);

                    // 옆면2: v0-v2-v3
                    glColor3f(1, 0, 0); glVertex3fv(v0);
                    glColor3f(0, 0, 1); glVertex3fv(v2);
                    glColor3f(1, 1, 0); glVertex3fv(v3);

                    // 옆면3: v0-v3-v1
                    glColor3f(1, 0, 0); glVertex3fv(v0);
                    glColor3f(1, 1, 0); glVertex3fv(v3);
                    glColor3f(0, 1, 0); glVertex3fv(v1);

                    // 바닥: v1-v3-v2
                    glColor3f(0.3f, 0.3f, 0.3f); glVertex3fv(v1);
                    glColor3f(0.3f, 0.3f, 0.3f); glVertex3fv(v3);
                    glColor3f(0.3f, 0.3f, 0.3f); glVertex3fv(v2);
                    glEnd();
                    glPopMatrix();


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


void hv::v1::render::setOrbitAngle(float yawDeg, float pitchDeg) {
    if (!impl) return;
    {
        std::scoped_lock lock(this->impl->mtx);
        impl->yawDeg = yawDeg;
        impl->pitchDeg = pitchDeg;
        impl->pitchDeg = std::clamp(impl->pitchDeg, -89.0f, 89.0f);
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

        impl->pitchDeg = std::clamp(impl->pitchDeg, -89.0f, 89.0f);

        impl->viewDirty = true;
    }
    impl->condV.notify_all();
}

#pragma endregion


