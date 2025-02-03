#include <mujoco/mujoco.h>
#include <iostream>
#include <iomanip>
#include <string>

// MuJoCoのモデルとデータ
static mjData* mujoco_data;
static mjModel* mujoco_model;
static const std::string model_path = "models/tb3.xml";

// 指定したボディ名のインデックスを取得
int get_body_id(const std::string& body_name) {
    int id = mj_name2id(mujoco_model, mjOBJ_BODY, body_name.c_str());
    if (id == -1) {
        std::cerr << "[ERROR] Body name not found: " << body_name << std::endl;
    }
    return id;
}

// シミュレーションの状態を出力
void print_state(int step, double dt, const mjModel* model, const mjData* data) {
    std::cout << "========== Step: " << step << " ==========" << std::endl;
    std::cout << "Time: " << std::fixed << std::setprecision(3) << step * dt << "s" << std::endl;

    // 重心の位置
    std::cout << "Center of Mass (com): ";
    std::cout << data->subtree_com[0] << ", " << data->subtree_com[1] << ", " << data->subtree_com[2] << std::endl;

    // 各部位の状態を表示
    std::string bodies[] = {"tb3_base", "left_wheel", "right_wheel", "back_castor"};
    for (const auto& body : bodies) {
        int id = get_body_id(body);
        if (id == -1) continue;

        // 各部位の位置（xpos）と速度（cvel）
        std::cout << "[" << body << "] Position: (";
        std::cout << data->xpos[id * 3] << ", " << data->xpos[id * 3 + 1] << ", " << data->xpos[id * 3 + 2] << ")";

        std::cout << " | Velocity: (";
        std::cout << data->cvel[id * 6] << ", " << data->cvel[id * 6 + 1] << ", " << data->cvel[id * 6 + 2] << ")" << std::endl;
    }

    // 制御入力（ctrl）
    std::cout << "Control Inputs: ";
    for (int j = 0; j < model->nu; j++) {
        std::cout << data->ctrl[j] << ", ";
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
