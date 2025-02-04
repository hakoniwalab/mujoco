#include <mujoco/mujoco.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include "mujoco_debug.hpp"
#include "mujoco_viewer.hpp"

// MuJoCoのモデルとデータ
static mjData* mujoco_data;
static mjModel* mujoco_model;
static const std::string model_path = "models/tb3.xml";

// シミュレーションスレッド
void simulation_thread(mjModel* model, mjData* data, bool& running_flag, std::mutex& mutex) {
    double simulation_timestep = model->opt.timestep;  // **XMLから `timestep` を取得**
    std::cout << "[INFO] Simulation timestep: " << simulation_timestep << " sec" << std::endl;

    while (running_flag) {
        auto start = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(mutex);
            data->ctrl[0] = 0.2;  // 左モーター
            data->ctrl[1] = 0.5;  // 右モーター
            mj_step(model, data);
            //print_body_state_by_name(model, data, "tb3_base");
        }

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double sleep_time = simulation_timestep - elapsed.count();

        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
        }
    }
}


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
    const double dt = mujoco_model->opt.timestep;
    std::cout << "[INFO] Starting simulation." << std::endl;
    
    std::mutex data_mutex;
    bool running_flag = true;
    std::thread sim_thread(simulation_thread, mujoco_model, mujoco_data, std::ref(running_flag), std::ref(data_mutex));
    viewer_thread(mujoco_model, mujoco_data, std::ref(running_flag), std::ref(data_mutex));
    running_flag = false;
    sim_thread.join();
    // **リソース解放**
    std::cout << "[INFO] Cleaning up resources." << std::endl;
    mj_deleteData(mujoco_data);
    mj_deleteModel(mujoco_model);

    std::cout << "[INFO] Simulation completed successfully." << std::endl;
    return 0;
}
