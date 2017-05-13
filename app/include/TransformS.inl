/**
 * @file TransformS.inl
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_TRANSFORMS_INL
#define ECS_FIT_TRANSFORMS_INL

#include "TransformS.h"

void TransformS::doTransform()
{
    PROF_BLOCK("Matrix loop");
    for (auto &e : foreach())
    {
        PositionC *pos{e.get<PositionC>()};

        e.get<TransformC>()->modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), pos->pos) *
                                           glm::mat4_cast(glm::quat(pos->rot)), pos->scale);
    }
    PROF_BLOCK_END();
    /*
    if (foreach().size() < PARALLEL_THRESHOLD)
    {
    }
    else
    {
        std::vector<std::thread> threads;

        u64 numThreads{std::thread::hardware_concurrency()};
        if (numThreads == 0)
        { // Unable to detect number of threads...
            numThreads = 2;
        }

        u64 perThread{foreach().size() / numThreads};
        auto beginIterator{foreach().begin()};
        auto endIterator{foreach().end()};

        for (u64 iii = 0; iii < numThreads; ++iii)
        {
            threads.emplace_back([iii, perThread, numThreads, &beginIterator, &endIterator] () {
                u64 index{iii * perThread};
                u64 endIndex{(iii + 1) * perThread};
                auto iterator(beginIterator + index);
                auto end{(iii == (numThreads - 1)) ? (beginIterator + endIndex) : endIterator};

                for (; iterator != end; ++iterator)
                {
                    iterator->get<TransformC>()->modelMatrix = glm::translate(glm::mat4(1.0f), iterator->get<PositionC>()->p) *
                                                               glm::mat4_cast(glm::quat(iterator->get<PositionC>()->r));
                    index++;
                }
            });
        }

        for (auto &th : threads)
        {
            th.join();
        }
    }
     */
}

#endif //ECS_FIT_TRANSFORMS_INL
