#pragma once
#include <random>

#if !defined(OUTPUT_DIRECTORY_NO_SLASH)
#define OUTPUT_DIRECTORY_NO_SLASH "Output2"
#endif
#define OUTPUT_DIR OUTPUT_DIRECTORY_NO_SLASH "/"

#if !defined(ACTOR_NUM)
#define ACTOR_NUM 8
#endif

#if !defined(OBJ_NUM)
#define OBJ_NUM 16
#endif

#if !defined(SEED)
#define SEED 777
#endif

#if !defined(TEST_VAR_SEED)
#define TEST_VAR_SEED 777
#endif

constexpr bool console_input =
#if defined(CONSOLE_INPUT)
true;
#else
false;
#endif

//#define FORCE_INLINE
#define FORCE_NOINLINE

constexpr bool print_acts_objs_len =
#if defined(PRINT_ACTS_OBJS_LENS)
true;
#else
false;
#endif

constexpr bool print_function_name =
#if defined(PRINT_FUNCS)
true;
#else
false;
#endif

inline int rand_int(int min, int max);
inline float rand_float(float min, float max);
inline bool rand_bool();
inline int rand_sign();

inline int actor_num = ACTOR_NUM;
inline int obj_num = OBJ_NUM;
inline int seed = SEED;

inline std::mt19937 gen(seed);

constexpr float world_left = 0.0f;
constexpr float world_right = 20.0f;
constexpr float world_up = 20.0f;
constexpr float world_down = 0.0f;

constexpr float world_width = world_right - world_left;
constexpr float world_height = world_up - world_down;

constexpr float min_dist_thresh = std::min(world_width, world_height) / 20.0f;
constexpr float max_dist_thresh = std::min(world_width, world_height) / 5.0f;

constexpr float min_obj_x_adv = min_dist_thresh / 15.0f;
constexpr float max_obj_x_adv = max_dist_thresh / 10.0f;
constexpr float min_obj_y_adv = min_dist_thresh / 15.0f;
constexpr float max_obj_y_adv = max_dist_thresh / 10.0f;

constexpr float min_act_x_adv = world_width / 16.0f;
constexpr float max_act_x_adv = world_width / 2.0f;
constexpr float min_act_y_adv = world_height / 16.0f;
constexpr float max_act_y_adv = world_height / 2.0f;

constexpr int min_paint = 1;
constexpr int max_paint = 20;

inline float gen_dist_thresh() {
	return rand_float(min_dist_thresh, max_dist_thresh);
}

inline float gen_obj_x_adv() {
	return rand_sign() * rand_float(min_obj_x_adv, max_obj_x_adv);
}

inline float gen_obj_y_adv() {
	return rand_sign() * rand_float(min_obj_y_adv, max_obj_y_adv);
}

inline float gen_act_x_adv() {
	return rand_sign() * rand_float(min_act_x_adv, max_act_x_adv);
}

inline float gen_act_y_adv() {
	return rand_sign() * rand_float(min_act_y_adv, max_act_y_adv);
}

inline std::vector<std::vector<int>> obj_act_asoc_mat;

inline void INIT_OBJ_ACT_ASOC_MAT() {
	obj_act_asoc_mat.resize(obj_num);

	for (int k = 0; k < obj_num; ++k) {
		std::vector<int>& obj_k_act_asoc = obj_act_asoc_mat[k];
		obj_k_act_asoc.resize(actor_num);

		std::generate(obj_k_act_asoc.begin(), obj_k_act_asoc.end(), [n = 1]() mutable { return n++; });
		std::shuffle(obj_k_act_asoc.begin(), obj_k_act_asoc.end(), gen);

		int act_sel_len = rand_int(1, actor_num);
		obj_k_act_asoc.resize(act_sel_len);

		std::sort(obj_k_act_asoc.begin(), obj_k_act_asoc.end());
	}
}

enum class Axis {
	X = 1,
	Y = 2,
	XY = X | Y
};

inline bool operator&(Axis a, Axis b)
{
	return static_cast<bool>(static_cast<int>(a) & static_cast<int>(b));
}

inline int gen_paint() {
	return rand_int(min_paint, max_paint);
}

inline Axis gen_axis() {
	return (Axis)rand_int(1, 3);
}

inline std::uniform_int_distribution<int> uid;
inline int rand_int(int min, int max) {
	using param_type = decltype(uid)::param_type;
	uid.param(param_type{ min, max });
	return uid(gen);
}

inline std::uniform_real_distribution<float> urd;
inline float rand_float(float min, float max) {
	using param_type = decltype(urd)::param_type;
	urd.param(param_type{ min, max });
	return urd(gen);
}

inline bool rand_bool() {
	return rand_int(0, 1);
}

inline int rand_sign() {
	return rand_int(0, 1) ? 1 : -1;
}

#define nl "\n"
#define nl2 nl nl
#define tab "\t"
#define tab2 tab tab
#define tab3 tab2 tab
#define tab4 tab2 tab2

std::string OpenNamespace(const char* namesp) {
	return std::format("namespace {} {}", namesp, "{\n\n");
}

std::string CloseNamespace(const char* namesp) {
	return std::format("{} // namespace {}\n\n", "}\n\n", namesp);
}

std::string UsingNamespace(const char* namesp) {
	return std::format("using namespace {};\n\n", namesp);
}

std::string UsingAlias(const char* alias, const char* subject) {
	return std::format("using {} = {};\n\n", alias, subject);
}

std::string UsingAlias(const std::string& alias, const char* subject) {
	return std::format("using {} = {};\n\n", alias, subject);
}