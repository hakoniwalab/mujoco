#include <mujoco/mujoco.h>
#include <iostream>
#include <cmath>

static mjData* mujoco_data;
static mjModel* mujoco_model;
static const std::string model_path = "models/tb3.xml";

int main(int argc, const char* argv[])
{
    // モデルの読み込み
    char error[1000];
    std::cout << "Loading model...: " << model_path << std::endl;
    mujoco_model = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
    if (!mujoco_model) {
        std::cerr << "Failed to load model: " << model_path << error << std::endl;
        return 1;
    }
    std::cout << "Model loaded successfully" << std::endl;

    // データの作成
    mujoco_data = mj_makeData(mujoco_model);

    // シミュレーションの実行
    const int n_steps = 1000;
    const double dt = 0.01;
    for (int i = 0; i < n_steps; i++) {
        // モデルの状態更新
        mj_step(mujoco_model, mujoco_data);

        // ロボットの状態を表示
        std::cout << "Time: " << i * dt << std::endl;
        std::cout << "qpos: ";
        for (int j = 0; j < mujoco_model->nq; j++) {
            std::cout << mujoco_data->qpos[j] << ", ";
        }
        std::cout << std::endl;
        std::cout << "qvel: ";
        for (int j = 0; j < mujoco_model->nv; j++) {
            std::cout << mujoco_data->qvel[j] << ", ";
        }
        std::cout << std::endl;
        std::cout << "qacc: ";
        for (int j = 0; j < mujoco_model->nv; j++) {
            std::cout << mujoco_data->qacc[j] << ", ";
        }
        std::cout << std::endl;
        std::cout << "ctrl: ";
        for (int j = 0; j < mujoco_model->nu; j++) {
            std::cout << mujoco_data->ctrl[j] << ", ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }


    // リソース解放
    mj_deleteData(mujoco_data);
    mj_deleteModel(mujoco_model);

    return 0;
}
