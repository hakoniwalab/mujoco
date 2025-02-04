#include "mujoco_viewer.hpp"

#define GL_SILENCE_DEPRECATION  // OpenGL の非推奨警告を無効化

#include <GLFW/glfw3.h>
#include <sstream> // 文字列ストリームで時間表示
#include <iomanip>
#include <iostream>
#include "mujoco_viewer.hpp"
#include <sstream>
#include <iomanip>
#include <thread>

// OpenGL & MuJoCo 変数
mjvCamera cam;
mjvOption opt;
mjvScene scn;
mjrContext con;
GLFWwindow* window;

// キーボードコールバック
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);  // ESCでウィンドウを閉じる
        }
    }
}

// 3Dビューアとシミュレーションを統合
void viewer_thread(mjModel* model, mjData* data, bool& running_flag, std::mutex& mutex) {
    // GLFW の初期化
    if (!glfwInit()) {
        std::cerr << "[ERROR] GLFW Initialization failed!" << std::endl;
        return;
    }

    // MuJoCo のビューア用ウィンドウを作成
    window = glfwCreateWindow(800, 600, "MuJoCo Simulation Viewer", NULL, NULL);
    if (!window) {
        std::cerr << "[ERROR] GLFW Window creation failed!" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync有効化
    glfwSetKeyCallback(window, keyboard);  // キーボード操作

    // MuJoCo ビューアの初期化
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_makeScene(model, &scn, 2000);
    mjr_makeContext(model, &con, mjFONTSCALE_150);

    // **ビューポート設定**
    mjrRect viewport = {0, 0, 800, 600};  // ウィンドウサイズと一致させる

    // シミュレーションループ
    while (!glfwWindowShouldClose(window)) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            mjv_updateScene(model, data, &opt, NULL, &cam, mjCAT_ALL, &scn);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mjr_render(viewport, &scn, &con);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // **リソース解放**
    mjr_freeContext(&con);
    mjv_freeScene(&scn);
    glfwDestroyWindow(window);
    glfwTerminate();
}