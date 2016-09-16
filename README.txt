Implementacja typu versioned_graph rozszerzajacego typy adjacency_list oraz adjacency_matrix zdefiniowane w
Boost Graph Library w wersji 1.58.0
o możliowść zapisywania aktualnego stanu grafu oraz jego póżniejszego przywracania.

Program został stworzony w ramach pracy magisterskiej
"Struktury danych dla algorytmów z nawrotami"
autor: Damian Lipka


versioned_graph rozszerza interfejs grafu o cztery dodatkowe funkcje:

commit(versioned_graph& g)
Zapisuje aktualny stan atrybutów grafu.

revert_changes(versioned_graph& g)
Przywraca ostatnio zapisany stan grafu.

undo_commit(versioned_graph& g)
Przywraca przedostatni stan grafu oraz usuwa ostatni punkt zapisu.


erase_history(versioned_graph& g)
Usuwa całkowicie historię zapisanych stanów.


Kod programu:

versioned_graph.h
versioned_graph_impl.h
versioned_graph_non_members.h

testy używające biblioteki Google Test:

versioned_graph_test.h
basic_tests.cpp
versioned_adjacency_list_test.cpp
versioned_adjacency_matrix_test.cpp

Przykład użycia:
example.cpp


Kompilacja:

plik example.cpp
można skompilować za pomocą:
g++ -std=c++11 example.cpp

Testy można skompilować za pomocą
cmake <ścieżka do tego katalogu>


