\section*{Co je Entropy}

\textbf{Entropy} je knihovna napsaná v programovacím jazyce \emph{C++}, která umožňuje návrh aplikací za použití \emph{Entity-Component-System} paradigmatu. Knihovna je volně k dispozici, pod \emph{MIT} licencí, v repositáři \href{https://github.com/T0mt0mp/Entropy}{GIT}. 

\section*{Použití knihovny}

\subsection*{Požadavky}

Pro správnou funkci vyžaduje knihovna následující nástroje: 
\begin{itemize}
	\item Překladač jazyka \emph{C++}, který podporuje standard jazyka \emph{C++14} -- mezi testované překladače patří GCC 6.3.1, Clang 3.9.1 a MSVC 19.10
	\item (volitelné) CMake -- alespoň verze 2.8
\end{itemize}

\subsection*{Stažení}

Nejjednodušším způsobem, jak získat knihovnu \textbf{Entropy}, je klonováním jejího repositáře:

\begin{lstlisting}[basicstyle=\ttfamily]
  mkdir Entropy
  git clone https://github.com/T0mt0mp/Entropy Entropy
  cd Entropy
  git submodule update --init --recursive
\end{lstlisting}

\subsection*{Překlad testů}

Součástí knihovny \textbf{Entropy} jsou testy, které lze přeložit pomocí systému \emph{CMake}. 

\begin{lstlisting}[basicstyle=\ttfamily]
  mkdir build && cd build
  cmake ../ -DCMAKE_BUILD_TYPE=Release
\end{lstlisting}
nebo pro operační systém Windows:
\begin{lstlisting}[basicstyle=\ttfamily]
  cmake ../ -DCMAKE_BUILD_TYPE=Release \
    -G "Visual Studio 15 2017 Win64"
\end{lstlisting}
\pagebreak
potom lze pro překlad použít:
\begin{lstlisting}[basicstyle=\ttfamily]
  cmake --build . --config Release
\end{lstlisting}

\noindent Výsledkem jsou spustitelné soubory v adresáři \texttt{build}.
\begin{itemize}
	\item \texttt{Comparison*} -- Porovnání různých volně dostupných knihoven.
	\item \texttt{Test*} -- Testy knihovny \textbf{Entropy}
	\item \texttt{gameTest} -- Testovací implementaci hry za použití knihovny \textbf{Entropy}.
\end{itemize}

\subsection*{Instalace}

Po stažení knihovny ji lze nainstalovat do adresáře \texttt{/usr/local/include} následující sekvencí příkazů: 

%\begin{lstlisting}[language=C++]
\begin{lstlisting}[basicstyle=\ttfamily]
  mkdir build && cd build
  cmake ../Entropy/ && make install
\end{lstlisting}

\noindent Alternativně je možné manuálně nakopírovat obsah adresáře \texttt{Entropy/include/} (obsahuje složku \texttt{Entropy}) do systémového \texttt{include}, nebo jiného adresáře, kde ji překladač najde.

\subsection*{Základy}

Před jejím použitím je nutno knihovnu přidat do zdrojového kódu, pomocí direktivy \texttt{include}:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
#include <Entropy/Entropy.h>
\end{lstlisting}

\subsubsection*{Universe}

Středem knihovny \textbf{Entropy} je třída \textbf{Universe}, která umožňuje komunikaci s entitním systémem. Z této třídy je třeba nejdříve podědit což umožňuje existenci několika oddělených světů:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
class MyUniverse : public ent::Universe<MyUniverse>
{ };
\end{lstlisting}

\noindent Potom již lze vytvořit instance daného vesmíru:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
MyUniverse u;
\end{lstlisting}

\subsubsection*{Entity}

\emph{Entity} jsou v knihovně \textbf{Universe} jednoduché číselné identifikátory, kterým je možné přiřadit 0-1 \emph{komponent}. \emph{Entity} se mohou nacházet ve dvou stavech -- aktivní a neaktivní. Tvorba nových entit je možná skrz třídu \textbf{Universe}:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
using Entity = MyUniverse::EntityT;
Entity e = u.createEntity();
\end{lstlisting}

\noindent \emph{Entity} je dále možné zničit, použitím metody \texttt{destroy}:
\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
e.destroy();
\end{lstlisting}

\noindent Informaci, zda je \emph{entita} platná lze získat metodou \texttt{valid}. \emph{Entity} se stává nevalidní ve chvíli, kdy je zničena.

\subsubsection*{Komponenty}

\emph{Komponenty} jsou základními nosiči dat v \emph{ECS}. Nové typy \emph{komponent} lze definovat následovně:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
struct PositionC
{
	float x, y;
};
\end{lstlisting}

\noindent Aby třída, nebo struktura, mohla být považována za \emph{komponentu}, musí splňovat následující požadavky. Musí obsahovat výchozí konstruktor -- konstruktor bez parametrů. Kromě toho může obsahovat neomezený počet dalších konstruktorů, kterým lze předávat parametry při přidání \emph{komponenty}. 

Každý typ \emph{komponent} v knihovně \textbf{Entropy} má vlastní tzv. \emph{nosič komponent}. Nové typy \emph{nosičů} lze definovat skrz dědění třídy \textbf{BaseComponentHolder} a implementaci všech virtuálních metod. Knihovna \textbf{Entropy} obsahuje tři typy předdefinovaných nosičů:

\begin{itemize}
	\item \textbf{ComponentHolder} -- výchozí nosič pro komponenty, které nemají specifikovaný jiný. Používá \texttt{std::map}. Tento nosič je výhodný v případech, kdy nejsou entity, které obsahují daný typ komponenty nijak seřazené.
	\item \textbf{ComponentHolderMapList} -- Používá \texttt{std::map}, který mapuje entity do souvislého pole komponent.
	\item \textbf{ComponentHolderList} -- Používá pole, do kterého jsou přímo namapovány identifikátory entit. Tento nosič je výhodný pro případy, kdy každá entita obsahuje daný typ komponent.
\end{itemize}

\noindent Specifikaci nosiče komponent lze provést následujícím způsobem:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
struct PositionC
{
	PositionC(float px, float py) :
		x{px}, y{py} { }
	PositionC() : 
		x{0.0f}, y{0.0f}
	using HolderT = ent::ComponentHolderList<PositionC>;
	float x, y;
};
\end{lstlisting}

Nad entitami lze provádět operace, které ovlivňují jaké komponenty daná entita obsahuje. Mezi tyto operace patří -- přidání (\texttt{add}), odebrání (\texttt{remove}), získání komponenty (\texttt{get}) a získání přítomnosti (\texttt{has}). Příklad použití těchto metod je následující:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
Entity e = u.createEntity();
e.add<PositionC>();
e.remove<PositionC>();
e.has<PositionC>();		// -> false
e.add<PositionC>(1.0f, 2.0f);
e.has<PositionC>();		// -> true
e.get<PositionC>->y;	// -> 2.0f
\end{lstlisting}

\subsubsection*{Systémy}

\emph{Systémy} umožňují iteraci nad entitami, tedy i nad komponentami, které vyhovují danému filtru. Jelikož komponenty neobsahují žádnou logiku, \emph{systémy} plní funkci tvorby akcí. Pokud bychom měli dva typy komponent -- \textbf{PositionC}, \textbf{MovementC} -- tak by deklarace \emph{systému}, který by iteroval nad entitami, které obsahují oba dva typy komponent, vypadala následovně:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
class MovementS : public MyUniverse::SystemT
{
	using Require = ent::Require<PositionC, MovementC>;
	using Reject = ent::Reject<>;
public:
	void doMove();
};
\end{lstlisting}

\noindent V tomto případě není \texttt{Reject} nutné, jelikož výchozí hodnota obou typů je prázdný typový seznam. Implementace samotné akce by mohla vypadat následovně:

\begin{lstlisting}[backgroundcolor = \color{lightgray}, language = C++, xleftmargin = 2cm, framexleftmargin = 1em, tabsize=4]
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
\end{lstlisting}

\noindent Každá třída, která dědí z třídy \emph{System}, obsahuje 3 metody -- \texttt{foreach}, \texttt{foreachAdded} a \texttt{foreachRemoved} -- díky kterým lze iterovat před entity, které vyhovují specifikovanému filtru, které byly přidána a které byly odebrány. \emph{Systémy} lze přidat danému vesmíru voláním metody \texttt{addSystem<S>}.

\subsubsection*{Kontrolní tok}

Důležitou součástí knihovny \textbf{Entropy} je tok kontroly, který postupuje následujícím způsobem: 
\begin{enumerate}
	\item Vytvoření instance třídy \textbf{Universe}.
	\item Registrace komponent, metodou \texttt{registerComponent<C>}. Tento krok je možný pouze před inicializací!
	\item Inicializace vesmíru zavoláním metody \texttt{init} na daná instanci třídy \textbf{Universe}.
	\item Následně je již možná práce s entitním systémem -- přidávání \emph{systémů}, tvorba entit apod. \label{Enum:Work}
	\item Kdykoliv lze z předchozí fáze přejít do fáze \emph{obnovy}, kdy je obnovena konzistence celého entitního systému. Po dokončení se systém vrací do kroku \ref{Enum:Work}.
\end{enumerate}

\noindent Součástí \emph{obnovovací} fáze je také aktualizace seznamů, nad kterými iterují \emph{systémy}. 