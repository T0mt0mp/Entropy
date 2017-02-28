/**
 * @file app/ECS.h
 * @author Tomas Polasek
 * @brief Configuration of the Entropy ECS.
 */

#ifndef SIMPLE_GAME_ECS_H
#define SIMPLE_GAME_ECS_H

#define NO_ENT_WARNING
#include <Entropy/Entropy.h>

class Universe : public ent::Universe<Universe>
{
public:
private:
protected:
};
extern template class ent::Universe<Universe>;

#endif //SIMPLE_GAME_ECS_H
