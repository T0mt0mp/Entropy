# Entropy - Entity-Component-System knihovna.

# Co je Entropy

Entropy je knihovna napsaná v programovacím jazyce C++, která umožňuje návrh aplikací za použití Entity-Component-System paradigmatu. Knihovna je volně k dispozici, pod MIT licencí. 

# Použití knihovny

## Požadavky

Pro správnou funkci vyžaduje knihovna následující nástroje: 
 - Překladač jazyka C++, který podporuje standard jazyka C++14 - mezi testované překladače patří GCC 6.3.1, Clang 3.9.1 a MSVC 19.10
 - (volitelné) CMake - alespoň verze 2.8

## Stažení

Nejjednodušším způsobem, jak získat knihovnu Entropy, je klonováním jejího repositáře:

```
mkdir Entropy
git clone https://github.com/T0mt0mp/Entropy Entropy
cd Entropy
git submodule update --init --recursive
```

## Překlad testů

Součástí knihovny Entropy jsou testy, které lze přeložit pomocí systému CMake. 

```
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
```
nebo pro operační systém Windows:
```
cmake ../ -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017 Win64"
```
potom lze pro překlad použít:
```
cmake --build . --config Release
```

Výsledkem jsou spustitelné soubory v adresáři build nebo Release.
 - **Comparison** -- Porovnání různých volně dostupných knihoven.
 - **Test** -- Testy knihovny Entropy.
 - **gameTest** -- Testovací implementaci hry za použití knihovny Entropy.

## Instalace

Po stažení knihovny ji lze nainstalovat do adresáře /usr/local/include (nebo alternativy OS Windows) následující sekvencí příkazů: 

```
mkdir build && cd build
cmake ../Entropy/ && make install
```

Alternativně je možné manuálně nakopírovat obsah adresáře Entropy/include/ (obsahuje složku Entropy) do systémového include, nebo jiného adresáře, kde ji překladač najde.

## Základy

Před jejím použitím je nutno knihovnu přidat do zdrojového kódu, pomocí direktivy include:

```C++
#include <Entropy/Entropy.h>
```

### Universe

Středem knihovny Entropy je třída `Universe`, která umožňuje komunikaci s entitním systémem. Z této třídy je třeba nejdříve podědit což umožňuje existenci několika oddělených světů:

```C++
class MyUniverse : public ent::Universe<MyUniverse>
{ };
```

Potom již lze vytvořit instance daného vesmíru:

```C++
MyUniverse u;
```

### Entity

`Entity` jsou v knihovně Entropy jednoduché číselné identifikátory, kterým je možné přiřadit 0-1 komponent. Entity se mohou nacházet ve dvou stavech - aktivní a neaktivní. Tvorba nových entit je možná skrz třídu Universe:

```C++
using Entity = MyUniverse::EntityT;
Entity e = u.createEntity();
```

`Entity` je dále možné zničit, použitím metody destroy:
```C++
e.destroy();
```

Informaci, zda je entita platná lze získat metodou `valid`. `Entity` se stává nevalidní ve chvíli, kdy je zničena.

### Komponenty

Komponenty jsou základními nosiči dat v ECS. Nové typy komponent lze definovat následovně:

```C++
struct PositionC
{
    float x, y;
};
```

Aby třída, nebo struktura, mohla být považována za komponentu, musí splňovat následující požadavky. Musí obsahovat výchozí konstruktor. Kromě toho může obsahovat neomezený počet dalších konstruktorů, kterým lze předávat parametry při přidání komponenty. 

Každý typ komponent v knihovně Entropy má vlastní tzv. nosič komponent. Nové typy nosičů lze definovat skrz dědění třídy `BaseComponentHolder` a implementaci všech virtuálních metod. Knihovna Entropy obsahuje tři typy předdefinovaných nosičů:

 - `ComponentHolder` -- výchozí nosič pro komponenty, které nemají specifikovaný jiný. Používá `std::map`. Tento nosič je výhodný v případech, kdy nejsou entity, které obsahují daný typ komponenty nijak seřazené.
 - `ComponentHolderMapList` -- Používá `std::map`, který mapuje entity do souvislého pole komponent.
 - `ComponentHolderList` -- Používá pole, do kterého jsou přímo namapovány identifikátory entit. Tento nosič je výhodný pro případy, kdy každá entita obsahuje daný typ komponent.

Specifikaci nosiče komponent lze provést následujícím způsobem:

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

Nad entitami lze provádět operace, které ovlivňují jaké komponenty daná entita obsahuje. Mezi tyto operace patří - přidání (`add`), odebrání (`remove`), získání komponenty (`get`) a získání přítomnosti (`has`). Příklad použití těchto metod je následující:

```C++
Entity e = u.createEntity();
e.add<PositionC>();
e.remove<PositionC>();
e.has<PositionC>();             // -> false
e.add<PositionC>(1.0f, 2.0f);
e.has<PositionC>();             // -> true
e.get<PositionC>->y;            // -> 2.0f
```

### Systémy

Systémy umožňují iteraci nad entitami, tedy i nad komponentami, které vyhovují danému filtru. Jelikož komponenty neobsahují žádnou logiku, systémy plní funkci tvorby akcí. Pokud bychom chtěli vytvořit systém, který bude iterovat nad entitamy s komponenty typu `PositionC` a `MovementC`, vypadala by jeho deklarace následovně:

```C++
class MovementS : public MyUniverse::SystemT
{
    using Require = ent::Require<PositionC, MovementC>;
    using Reject = ent::Reject<>;
public:
    void doMove();
};
```

V tomto případě není `Reject` nutné, jelikož výchozí hodnota obou typů je prázdný typový seznam. Implementace samotné akce by mohla vypadat následovně:

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

Děděná třída `System`, obsahuje 3 metody -- `foreach`, `foreachAdded` a `foreachRemoved` - díky kterým lze iterovat před entity vyhovující specifikovanému filtru. Systémy lze přidat danému vesmíru voláním metody `addSystem<S>`.

### Kontrolní tok

Důležitou součástí knihovny Entropy je tok kontroly, který postupuje následujícím způsobem: 
1. Vytvoření instance třídy `Universe`.
2. Registrace komponent, metodou `registerComponent<C>`. Tento krok je možný pouze před inicializací!
3. Inicializace vesmíru zavoláním metody `init` na dané instanci třídy `Universe`.
4. Následně je již možná práce s entitním systémem - přidávání systémů, tvorba entit apod. 
5. Kdykoliv lze z předchozí fáze přejít do fáze obnovy, kdy je obnovena konzistence celého entitního systému. Po dokončení se systém vrací do čtvrtého kroku .

Součástí obnovovací fáze je také aktualizace seznamů, nad kterými systémy iterují. 
