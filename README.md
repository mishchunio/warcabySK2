1. Wprowadzenie

Celem projektu było stworzenie sieciowej gry turowej, opartej na klasycznych warcabach. Aplikacja umożliwia rozgrywki między dwoma graczami, którzy łączą się z centralnym serwerem. Projekt realizuje zarówno logikę gry, jak i obsługę komunikacji sieciowej, zapewniając walidację ruchów, powiadomienie o wyniku rozgrywki oraz obsługę rozłączenia jednego z graczy.

2. Opis funkcjonalności

Projekt umożliwia:

Równoległe rozgrywki – Serwer obsługuje wiele równoległych gier, każda pomiędzy dwoma graczami.
Walidację ruchów – Wszystkie ruchy są sprawdzane na serwerze pod kątem poprawności (np. czy ruch odbywa się w obrębie planszy, czy pole docelowe jest wolne, czy wykonane bicie jest obowiązkowe). Weryfikowane są również specjalne przypadki, takie jak bicie przez damkę oraz promocja pionka do damki.
Powiadomienie o wyniku gry – Po zakończeniu rozgrywki, serwer wysyła komunikat GAME_OVER wraz z informacją o zwycięzcy, co pozwala graczom zobaczyć wynik w interfejsie klienta.
Obsługę rozłączenia gracza – W przypadku utraty połączenia z jednym z graczy, serwer wykrywa to zdarzenie i informuje drugiego gracza komunikatem OPPONENT_DISCONNECTED, co pozwala na odpowiednią reakcję w interfejsie użytkownika.
3. Architektura systemu

Projekt został podzielony na dwie główne części: serwer oraz klient.

Serwer:
Odpowiedzialny za walidację ruchów, zarządzanie stanem gry oraz komunikację między graczami.
Realizuje wielowątkowość – każdy klient jest obsługiwany w osobnym wątku.
Stan gry przechowywany jest w klasie Game, która zawiera m.in. planszę, liczbę pionków, aktualnego gracza oraz logikę wykonywania ruchów (w tym obsługę bicia, promocji i walidacji ruchów).
Komunikaty są wysyłane do klientów przy użyciu prostego protokołu tekstowego, np. "MOVE_UPDATE", "GAME_OVER", "OPPONENT_DISCONNECTED".
Klient:
Implementowany w języku Python z wykorzystaniem biblioteki Tkinter do stworzenia graficznego interfejsu użytkownika.
Klient łączy się z serwerem, wysyła komendy (np. ruchy gracza) oraz odbiera aktualizacje stanu gry, które są następnie wyświetlane na planszy.
Obsługuje interakcję użytkownika – zaznaczanie pionków, wysyłanie ruchów oraz wyświetlanie komunikatów o zakończeniu gry czy rozłączeniu przeciwnika.
4. Implementacja i wyzwania

W trakcie implementacji napotkano kilka wyzwań, m.in.:

Walidacja ruchów:
Konieczność dokładnego sprawdzania poprawności ruchów (dla zwykłych pionków oraz damki) wymagała implementacji wielu warunków. Szczególną trudność stanowiły ruchy damką, które mogą poruszać się o dowolną liczbę pól po przekątnej. Problem rozwiązywano poprzez iterację po polach pomiędzy punktem startowym a docelowym oraz sprawdzanie liczby zbitych pionków.
Promocja pionka do damki:
Implementacja promocji wymagała nie tylko zmiany stanu gry, ale również modyfikacji komunikatów wysyłanych do klientów (dodanie flagi "KING"), aby klient mógł poprawnie wyświetlić zmieniony symbol.
Obsługa rozłączenia:
Wykrycie rozłączenia klienta oraz informowanie przeciwnika stanowiło wyzwanie, ponieważ trzeba było zachować spójność stanu gry i usunąć powiązane wpisy z map, takich jak playerToGameId i activeGames.
5. Wnioski

Projekt spełnia postawione wymagania – serwer poprawnie weryfikuje ruchy, informuje o zakończeniu gry i radzi sobie z rozłączaniem graczy. Realizacja gry turowej wymagała zarówno solidnego zaprojektowania logiki gry, jak i poprawnej obsługi komunikacji sieciowej. Efektem końcowym jest system umożliwiający równoległe rozgrywki między dwoma graczami, gdzie każdy ruch jest weryfikowany na serwerze, co gwarantuje uczciwość rozgrywki.
W trakcie pracy nad projektem zdobyto doświadczenie w implementacji rozwiązań wielowątkowych, zarządzaniu stanem gry oraz tworzeniu protokołu komunikacji między klientem a serwerem. Projekt stanowi solidną bazę do dalszej rozbudowy, np. o nowe tryby gry lub ulepszony interfejs graficzny.