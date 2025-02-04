#include <mujoco/mujoco.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "mujoco_debug.hpp"

// MuJoCoのモデルとデータ
static mjData* mujoco_data;
static mjModel* mujoco_model;
static const std::string model_path = "models/tb3.xml";

int main(int argc, const char* argv[])
{
    // **MuJoCoモデルの読み込み**
    char error[1000];
    std::cout << "[INFO] Loading model: " << model_path << std::endl;
    mujoco_model = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
    if (!mujoco_model) {
        std::cerr << "[ERROR] Failed to load model: " << model_path << "\n" << error << std::endl;
        return 1;
    }
    std::cout << "[INFO] Model loaded successfully." << std::endl;

    // **データの作成**
    std::cout << "[INFO] Creating simulation data." << std::endl;
    mujoco_data = mj_makeData(mujoco_model);

    // **初期状態を正しく計算する**
    mj_forward(mujoco_model, mujoco_data);
    
    // **シミュレーションの実行**
    const int n_steps = 10;
    const double dt = mujoco_model->opt.timestep;
    std::cout << "[INFO] Starting simulation for " << n_steps << " steps." << std::endl;
    
    for (int i = 1; i <= n_steps; i++) {
        std::cout << "[DEBUG] Step: " << i << std::endl;
        
        // **モデルの状態更新**
        mj_step(mujoco_model, mujoco_data);

        // **関節の状態を表示**
        print_body_state_by_name(mujoco_model, mujoco_data, "tb3_base");
    }

    // **リソース解放**
    std::cout << "[INFO] Cleaning up resources." << std::endl;
    mj_deleteData(mujoco_data);
    mj_deleteModel(mujoco_model);

    std::cout << "[INFO] Simulation completed successfully." << std::endl;
    return 0;
}
