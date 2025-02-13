#include "mujoco_debug.hpp"
#include "mujoco_viewer.hpp"
#include <mujoco/mujoco.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

// MuJoCoのモデルとデータ
static mjModel* mujoco_model;
static mjData* mujoco_data;
static const std::string model_path = "models/drone.xml";

static std::mutex data_mutex;

// プロペラの制御設定
const char* prop_names[] = {"prop1", "prop2", "prop3", "prop4"};
std::vector<double> prop_thrust(4, 0.0);
std::vector<double> prop_torque(4, 0.0);

// **mjcb_control コールバック関数**
void my_control_callback(const mjModel* model, mjData* data) {
    //std::lock_guard<std::mutex> lock(data_mutex);

    for (int i = 0; i < 4; i++) {
        int body_id = mj_name2id(model, mjOBJ_BODY, prop_names[i]);
        if (body_id == -1) {
            std::cerr << "[ERROR] Body not found: " << prop_names[i] << std::endl;
            continue;
        }
        //std::cout << "[INFO] Applying control to: " << prop_names[i] << std::endl;

        // ボディ座標系での推力とトルク
        double F_body[3] = {0, 0, prop_thrust[i]};  // Z軸方向推力
        double T_body[3] = {0, 0, prop_torque[i]}; // 偶数は時計回り、奇数は反時計回り

        // ワールド座標系の回転行列（3×3）
        double* R = data->xmat + 9 * body_id;

        // ボディ座標系 → ワールド座標系変換
        double F_world[3] = {
            R[0] * F_body[0] + R[1] * F_body[1] + R[2] * F_body[2],
            R[3] * F_body[0] + R[4] * F_body[1] + R[5] * F_body[2],
            R[6] * F_body[0] + R[7] * F_body[1] + R[8] * F_body[2]
        };

        double T_world[3] = {
            R[0] * T_body[0] + R[1] * T_body[1] + R[2] * T_body[2],
            R[3] * T_body[0] + R[4] * T_body[1] + R[5] * T_body[2],
            R[6] * T_body[0] + R[7] * T_body[1] + R[8] * T_body[2]
        };

        // `xfrc_applied` に適用
        for (int j = 0; j < 3; j++) {
            data->xfrc_applied[body_id * 6 + j] = F_world[j];
            data->xfrc_applied[body_id * 6 + 3 + j] = T_world[j];
        }
    }
}

// **シミュレーションスレッド**
void simulation_thread(mjModel* model, mjData* data, bool& running_flag, std::mutex& mutex) {
    double simulation_timestep = model->opt.timestep;
    std::cout << "[INFO] Simulation timestep: " << simulation_timestep << " sec" << std::endl;

    while (running_flag) {
        auto start = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(mutex);
            prop_thrust[0] = 1.2;
            prop_thrust[1] = 1.2;
            prop_thrust[2] = 1.2;
            prop_thrust[3] = 1.2;
            prop_torque[0] = 0.01;
            prop_torque[1] = 0.0;
            prop_torque[2] = 0.0;
            prop_torque[3] = 0.0;
            mj_step(model, data);
        }

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double sleep_time = simulation_timestep - elapsed.count();

        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
        }
    }
}

// **メイン関数**
int main() {
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
    
    // **コールバック登録**
    mjcb_control = my_control_callback;

    // **シミュレーションの実行**
    const double dt = mujoco_model->opt.timestep;
    std::cout << "[INFO] Starting simulation." << std::endl;
    
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
