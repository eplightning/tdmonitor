# Token-based Distributed Monitor

Rozproszony monitor oparty na algorytmie Suzukiego-Kasami

## Kompilacja
Wykorzystywany jest system budowania Qt czyli qmake

`
qmake tdmonitor.pro
`

`
make qmake_all
`

`
make
`

## Uruchamianie przykładu
Dostarczony jest przykład użycia biblioteki w postaci programu konsumenta i producenta.


`
prodconsumer [adres_nasłuchiwania] [indeks_węzła] [adres_węzła1] [adres_węzła2] ...
`

`
prodconsumer 0.0.0.0:31410 0 127.0.0.1:31410 127.0.0.1:31411 127.0.0.1:31412
`

## Opis implementacji
### Algorytm i ogólne działanie
W celu osiągnięcia rozproszonego wzajmnego wykluczania został użyty algorytm Suzuki-Kasami, dodatkowo w tokenie przechowywane są dane monitora. Węzły czekają z rozpoczęciem pracy na nawiązanie połączenia z wszystkimi pozostałymi węzłami, węzły z wyższymi indeksami łączą się do węzłów z niższymi (które nasłuchują i przyjmują połączenia). 

Po tym następuje utworzenie pierwszego tokenu (początkowo posiadanego przez węzeł o indeksie 0) - systemowego w którym są przechowywane identyfikatory istniejących monitorów. Utworzyć monitor można jedynie posiadając token systemowy - rozwiązany jest tym samym problem tego kto ma być pierwszym posiadaczem tokenów (każdy monitor ma osobny). Następnie jest wysyłany ostatni handshake do wszystkich węzłów, po otrzymaniu wiadomości od wszystkich węzłow rozpoczyna się praca klastra monitorów i wszystkie oczekujące żądania (żądanie tokenu systemowego i ewentualnie wpuszczenie do sekcji krytycznej w przypadku węzła zerowego)  zostają wykonane.

Wszystkie operacje klienta z wyjątkiem lock i wait są nieblokujące, cały powyższy proces przechodzi transparentnie dla klienta. Klastr węzłów może posiadać wiele monitorów każdy identyfikowany unikalną nazwą i posiadający osobny token.

Każdy monitor może posiadać własne pola (oprócz tych wymaganych przez algorytm Suzuki-Kasami), które będą przechowywane i przekazywane w tokenie. Przy tworzeniu monitora tworzymy wiązanie zwyczajnego pola z w właściwościami przechowywanymi w tokenie, które są transparentnie synchronizowane. Obsługiwane typy to wszystkie typy liczbowe (8,16,32,64 unsigned i signed, float oraz double), tablica typów liczbowych (std::vector) oraz std::vector<std::string>. Klient może rejestrować własne typy podając funkcje obsługującą dekodowania z tablicy bajtów i kodowania na tablice bajtów, w ten sposób jest obsługiwany typ std::vector<std::string> wykorzysytywany przez token systemowy do trzymania listy identyfikatorów istniejących monitorów.

Jeśli chodzi o mechanizmy wzajemnego wykluczania jest zaimplementowany mutex oraz zmienne warunkowe (signalAll oraz wait), może być stworzonych wiele zmiennych warunkowych.

W celu ukrycia działania główna pętla klastra działa we własnym wątku, komunikacja ze strony klienta do pętli polega na przesyłaniu wiadomości (umieszczaniu ich w kolejce wydarzeń). Dodatkowo tworzony jest wątek, w którym przebiega równolegle działająca pętla zajmująca się komunikacją sieciową.

Komunikacja sieciowa opiera się na gniazdach TCP gdzie wykorzystywana jest własna biblioteka do tego celu napisana na potrzeby projektu z przedmiotu Sieci Komputerowe.

## Opis plików
* *prodconsumer* - Przykładowa aplikacja wykorzystująca bibliotekę
* *lib* - Biblioteka
  * *cluster.cpp* - Klasa reprezentująca klastr, uruchamia wątki pętli wydarzeń klastra i managera TCP i dostarcza interfejs do komunikacji z klastrem
  * *cluster_loop.cpp* - Pętla wydarzeń klastra gdzie zachodzi cała logika działania rozproszonego monitora
  * *event.cpp* - Implementacja pętli wydarzeń wykorzystywanych przez *cluster_loop.cpp*
  * *marshaller.cpp* - Klasa odpowiedzialna za serializację danych a dokładniej właściwości tokenów
  * *monitor.cpp* - Reprezentacja monitora rozproszonego, służy do deklaracji monitora (jego zmiennych warunkowych i właściwości), dostarcza interfejs monitora: lock, unlock, signal, wait
  * *token.cpp* - Reprezentacja tokena, częśc publiczna (tworzona przez użytkownika, służy do koumunikacji pomiędzy pętlą a klientem) i część prywatna obsługiwana wewnętrznie przez pętle wydarzeń
  * *packets/core.php* - Pakiety wymieniane pomiędzy węzłami
  * *tcp_manager.cpp* - Główna częśc biblioteki sieciowej
  * *selector/selector_epoll.cpp*, *selector.cpp* - Abstrakcja do mechanizmów oczekiwania na deskryptory, implementacja dla epolla, na potrzeby projektu na SK była implementacja również dla kQueue (pod OSX i *BSD) jednak nie była aktualizowana
