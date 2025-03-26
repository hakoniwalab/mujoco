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
    mju_zero(data->qfrc_applied, model->nv);

    for (int i = 0; i < 4; i++) {
        int prop_id = mj_name2id(model, mjOBJ_BODY, prop_names[i]);
        if (prop_id == -1) {
            std::cerr << "[ERROR] Body not found: " << prop_names[i] << std::endl;
            continue;
        }
            // ワールド座標でのプロペラ位置
            double prop_pos[3] = {
                data->xpos[prop_id * 3 + 0],  
                data->xpos[prop_id * 3 + 1],  
                data->xpos[prop_id * 3 + 2]
            };

        //std::cout << "[INFO] Applying control to: " << prop_names[i] << std::endl;

        // ボディ座標系での推力とトルク
        double F_body[3] = {0, 0, prop_thrust[i]};  // Z軸方向推力
        double T_body[3] = {0, 0, prop_torque[i]}; // 偶数は時計回り、奇数は反時計回り

        // ワールド座標系の回転行列（3×3）
        double* R = data->xmat + 9 * prop_id;

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
        //std::cout << "body_id: " << body_id << std::endl;
        // `mj_applyFT()` を使用して、プロペラの位置に推力を適用
        mj_applyFT(model, data, F_world, T_world, prop_pos, prop_id, data->qfrc_applied);
    }
}
#include <iostream>
#include <cmath>

// クォータニオンをオイラー角 (roll, pitch, yaw) に変換する関数
void quatToEuler(const double quat[4], double euler[3]) {
    // クォータニオンの成分
    double w = quat[0];
    double x = quat[1];
    double y = quat[2];
    double z = quat[3];

    // Roll (X軸回転)
    double sinr_cosp = 2.0 * (w * x + y * z);
    double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
    euler[0] = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (Y軸回転)
    double sinp = 2.0 * (w * y - z * x);
    if (std::abs(sinp) >= 1)
        euler[1] = std::copysign(M_PI / 2, sinp); // 角度範囲を [-90, 90] に制限
    else
        euler[1] = std::asin(sinp);

    // Yaw (Z軸回転)
    double siny_cosp = 2.0 * (w * z + x * y);
    double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
    euler[2] = std::atan2(siny_cosp, cosy_cosp);
}



// **シミュレーションスレッド**
void simulation_thread(mjModel* model, mjData* data, bool& running_flag, std::mutex& mutex) {
    double simulation_timestep = model->opt.timestep;
    std::cout << "[INFO] Simulation timestep: " << simulation_timestep << " sec" << std::endl;

    auto body_id = mj_name2id(model, mjOBJ_BODY, "drone_base");
    auto jnt_id = model->body_jntadr[body_id];
    auto qpos_index = mujoco_model->jnt_qposadr[jnt_id];
    auto dof_index = mujoco_model->jnt_dofadr[jnt_id];
    while (running_flag) {
        auto start = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(mutex);
            prop_thrust[0] = 1.5;
            prop_thrust[1] = 1.51;
            prop_thrust[2] = 1.5;
            prop_thrust[3] = 1.51;
            prop_torque[0] = -0.00;
            prop_torque[1] = -0.00;
            prop_torque[2] = -0.0;
            prop_torque[3] = -0.0;
            mj_step(model, data);

            // データの表示
            // 位置
            std::cout << "Position: ";
            for (int i = 0; i < 3; i++) {
                std::cout << std::fixed << std::setprecision(3) << data->qpos[i] << " ";
            }
            std::cout << std::endl;
            // 速度
            std::cout << std::endl;
            std::cout << "Velocity2: ";
            for (int i = 0; i < 3; i++) {
                std::cout << std::fixed << std::setprecision(3) << data->qvel[dof_index + i] << " ";
            }
            std::cout << std::endl;
            //角度
            // 1. クォータニオンの取得
            // d->qposからボディの回転（クォータニオン）を取得
            // 通常、qposの最初の3要素は位置(x,y,z)で、次の4要素が回転のクォータニオン(w,x,y,z)
            mjtNum* quat = data->qpos + qpos_index + 3;  // body_idは対象のボディのID
            double euler[3];
            quatToEuler(quat, euler);

            std::cout << "Orientation: ";
            for (int i = 0; i < 3; i++) {
                std::cout << std::fixed << std::setprecision(3) << euler[i] << " ";
            }
            std::cout << std::endl;
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
