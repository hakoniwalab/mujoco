#include <mujoco/mujoco.h>
#include <iostream>
#include <iomanip>
#include <string>

static mjData* mujoco_data;
static mjModel* mujoco_model;
static const std::string model_path = "models/tb3.xml";

void print_state(int step, double dt, const mjModel* model, const mjData* data) {
    std::cout << "========== Step: " << step << " ==========" << std::endl;
    std::cout << "Time: " << std::fixed << std::setprecision(3) << step * dt << "s" << std::endl;

    // qpos
    std::cout << "qpos: ";
    for (int j = 0; j < model->nq; j++) {
        std::cout << std::fixed << std::setprecision(5) << data->qpos[j] << ", ";
    }
    std::cout << std::endl;

    // qvel
    std::cout << "qvel: ";
    for (int j = 0; j < model->nv; j++) {
        std::cout << std::fixed << std::setprecision(5) << data->qvel[j] << ", ";
    }
    std::cout << std::endl;

    // qacc
    std::cout << "qacc: ";
    for (int j = 0; j < model->nv; j++) {
        std::cout << std::fixed << std::setprecision(5) << data->qacc[j] << ", ";
    }
    std::cout << std::endl;

    // ctrl
    std::cout << "ctrl: ";
    for (int j = 0; j < model->nu; j++) {
        std::cout << std::fixed << std::setprecision(5) << data->ctrl[j] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, const char* argv[])
{
    // モデルの読み込み
    char error[1000];
    std::cout << "[INFO] Loading model: " << model_path << std::endl;
    mujoco_model = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
    if (!mujoco_model) {
        std::cerr << "[ERROR] Failed to load model: " << model_path << "\n" << error << std::endl;
        return 1;
    }
    std::cout << "[INFO] Model loaded successfully." << std::endl;

    // データの作成
    std::cout << "[INFO] Creating simulation data." << std::endl;
    mujoco_data = mj_makeData(mujoco_model);
    
    // 初期状態の確認
    print_state(0, 0.0, mujoco_model, mujoco_data);

    // シミュレーションの実行
    const int n_steps = 1000;
    const double dt = 0.01;
    std::cout << "[INFO] Starting simulation for " << n_steps << " steps." << std::endl;
    
    for (int i = 1; i <= n_steps; i++) {
        std::cout << "[DEBUG] Step: " << i << std::endl;
        
        // モデルの状態更新
        mj_step(mujoco_model, mujoco_data);

        // ロボットの状態を表示
        print_state(i, dt, mujoco_model, mujoco_data);
    }

    // リソース解放
    std::cout << "[INFO] Cleaning up resources." << std::endl;
    mj_deleteData(mujoco_data);
    mj_deleteModel(mujoco_model);

    std::cout << "[INFO] Simulation completed successfully." << std::endl;
    return 0;
}
