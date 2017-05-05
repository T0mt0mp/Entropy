# Entropy - Entity-Component-System library.

# What is Entropy

Entropy is an ECS library written in C++ programming language.

# Library usage

## Requirements

Entropy has following usage requirements:
 - C++ compiler, with C++14 standard support - GCC 6.3.1, Clang 3.9.1 and MSVC 19.10 were tested.
 - (optional) CMake - of version 2.8 or higher.

## Downloading

Easiest way to get the library is to use git:

```
mkdir Entropy
git clone https://github.com/T0mt0mp/Entropy Entropy
cd Entropy
git submodule update --init --recursive
```

## Test compilation

Entropy contains tests, which can be compiled using CMake:

```
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
```
or for Windows operating system:
```
cmake ../ -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017 Win64"
```
after which the actual build can be ran:
```
cmake --build . --config Release
```

These actions result in a number of executable files, in the build (or Release in case of MSVC) directories:
 - **Comparison** - Comparison of Entropy against other ECS libraries.
 - **Test** - Testing features of Entropy.
 - **gameTest** - Test implementation of a game, using Entropy.

## Installation

Library can be installed in the /usr/local/include (or Windows alternative) directory using the following sequence of commands:

```
mkdir build && cd build
cmake ../Entropy/ && make install
```

The alternative is to manually copy the content of the Entropy/include/ directory (which contains "Entropy" folder) into a folder, where the compiler can find it.

## Basics

Before using the library, it needs to be included in the C++ source:

```C++
#include <Entropy/Entropy.h>
```

### Universe

`Universe` class is the interface, which allows communication with underlying systems. The base class needs to be inherited first, using the CRTP:

```C++
class MyUniverse : public ent::Universe<MyUniverse>
{ };
```

After which it can be instantiated:

```C++
MyUniverse u;
```

### Entity

Entities are represented with numerical identifiers, which can be composed of 0-1 components of each type. Entities can be either active, inactive or invalid. The creation of new entities is possible, through the `Universe` class:

```C++
using Entity = MyUniverse::EntityT;
Entity e = u.createEntity();
```

Entities can be destroyed, using the `destroy` method:
```C++
e.destroy();
```

`Entity` becomes invalid at the moment of destruction. Amongst other methods, available through 'Entity' class, are `activate`, `deactivate`, `active` and a plethora of other methods, which allow manipulation of entities components.

### Components

Component are basic data holders in the ECS design paradigm. New components can be defined in a following way:

```C++
struct PositionC
{
    float x, y;
};
```

In order for a class, or structure, to become a component, it has to fulfill following requirements. It has to allow for default construction. Also the components should be a POD type, to allow for simple memory copy. Components can also contain other constructors, which could be used for passing data to newly added components.

Each type of component has a specific component holder. New types of holder can be created, through inheritance of `BaseComponentHolder` class and implementation of all required methods. Entropy already comes with some predefined holders, which are the following:

 - `ComponentHolder` - Used as default, if components doesn't specify its holder. It is implemented using `std::map`. This holder is useful in cases, when not every entity contains given component type.
 - `ComponentHolderMapList` - Also uses `std::map`, but the component are held in a contiguous array.
 - `ComponentHolderList` - Používá pole, do kterého jsou přímo namapovány identifikátory entit. Tento nosič je výhodný pro případy, kdy každá entita obsahuje daný typ komponent.
 - `ComponentHolderList` - Uses a simple array, where the mapping function is simple indexation, using the entity identifier. This holder is advantageous in cases, when almost every entity should contain this type of component.

Component holder can be specified in a following way:

```C++
struct PositionC
{
    using HolderT = ent::ComponentHolderList<PositionC>;

    PositionC(float px, float py) :
        x{px}, y{py} { }
    PositionC() : 
        x{0.0f}, y{0.0f}
    float x, y;
};
```

Entities contains operations, which allow the manipulation of present component. Amongst these operations are - `add`, `remove`, `get` and `has`:

```C++
Entity e = u.createEntity();
e.add<PositionC>();
e.remove<PositionC>();
e.has<PositionC>();             // -> false
e.add<PositionC>(1.0f, 2.0f);
e.has<PositionC>();             // -> true
e.get<PositionC>->y;            // -> 2.0f
```

### System

Systems allow iteration over entities and their components. Each system defined the type of entities, which it will iterate over, using a filter. System, which is going to iterate over entities with components `PositionC` and `MovementC`, could be declared in a following way:

```C++
class MovementS : public MyUniverse::SystemT
{
    using Require = ent::Require<PositionC, MovementC>;
    using Reject = ent::Reject<>;
public:
    void doMove();
};
```

`Reject` is unnecessary in this case, since the default value is an empty type list. Implementation of the `doMove` action could look like this:

```C++
void MovementS::doMove()
{
    for (auto &e : foreach())
    {
        PositionC *p{e.get<PositionC>()};
        MovementC *m{e.get<MovementC}()};
        p->x += m->dX;
        p->y += m->dY;
    }
}
```

When inherited, `System` class allows acces to 3 special methods - `foreach`, `foreachAdded` and `foreachRemoved`. These methods can be used as iterators, to iterate over entities.

### Control flow

Important part of Entropy library is its flow of control:
1. Construction of `Universe` class.
2. Registration of components, using the `registerComponent<C>` method. After this phase, registration of new components is not allowed.
3. Initialization of the universe, using `init` method.
4. Normal work with entity system can now commence, including the adding of new systems.
5. When no more systems are running, the entity system can switch to refresh phase, using the `refresh` method. After refresh finished, the system returns to the previous step. Important part of this phase is refresh of entity lists, which are being iterated by the systems.

```C++
MyUniverse u;
u.registerComponent<PositionC>();
u.registerComponent<MovementC>();
u.init();
MovementS *ms{u.addSystem<MovementS>()};
u.refresh();
/* create entities, add components etc. */
while (running)
{
    ms->doMove();
    u.refresh();
}
```

