#ifndef GRAVITY_BH_GRID_HPP
#define GRAVITY_BH_GRID_HPP

#include "AABB.hpp"
#include "BH_QuadTree.hpp"

namespace gravity {
    /* Grid index goes from left to right, bottom to top */
    template <int WIDTH, int HEIGHT>
    struct BH_Grid {
        static constexpr int N = WIDTH * HEIGHT;
    private:
        std::vector<BH_QuadTree> trees_;
    public:
        AABB const WORLD;
        BH_Grid(AABB bounds, unsigned int entities, float theta_sqr) : WORLD(std::move(bounds)) {
            using namespace mathsimd;
            float2 const step = (WORLD.max - WORLD.min) / float2{static_cast<float>(WIDTH),static_cast<float>(HEIGHT)};
            trees_.reserve(N);
            for (int i = 0; i < N; ++i) {
                float2 min{static_cast<float>(i % WIDTH),static_cast<float>(i / WIDTH)};
                min = WORLD.min + min * step;
                trees_.emplace_back(theta_sqr, {min, min + step}, (entities / N) << 2u);
            }
        }
        [[nodiscard]] int getIndex(mathsimd::float2 p) const  {
            using namespace mathsimd;
            p = p - WORLD.min;
            p = p / (WORLD.max - WORLD.min);
            p = p * float2{static_cast<float>(WIDTH),static_cast<float>(HEIGHT)};
            p = float2(std::floor(p.x()),std::floor(p.y()));
            return static_cast<int>(dot(p, {1, static_cast<float>(WIDTH)}));
        }
        [[nodiscard]] BH_QuadTree const& tree(int idx) const { return trees_[idx]; }
        BH_QuadTree& tree(int idx) { return trees_[idx]; }

    };
}



#endif //GRAVITY_BH_GRID_HPP
