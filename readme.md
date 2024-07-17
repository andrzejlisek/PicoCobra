# Opis ogólny

PicoCobra jest to projekt roboczy emulatora polskiego komputera **Cobra1** przeznaczony na płytkę Picomputer, który jest oparty na Raspberry Pi Zero\.

Projekt bazuje na [MCUME](https://github.com/Jean-MarcHarvengt/MCUME/tree/master "https://github.com/Jean-MarcHarvengt/MCUME/tree/master"), który jest emulatorem innych komputerów na to samo urzadzenie\.

W tym projekcie jest zawarty pierwotny projekt, ale jest skonfigurowany i dostosowany do kompilacji emulatora **Cobra1**\. Kod źródłowy **PicoCobra** jest w podkatalogu **MCUME\_pico/picocobra**\.

# Kompilowanie i uruchomienie

Ponizszy opis dotyczy systemu Linux, w innych systemach operacyjnych przebieg czynności może się trochę różnić\.

Aby móc skompilować emulator potrzebna jest istalacja [Rapberry Pi SDK](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html "https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html")\., w repozytorium tego projektu jest już SDK potrzebny do PICO\.

Do systemu Linux jest już gotowy skrypt **Compile\.sh**, który należy otworzyc w dowolnym edytorze tekstu, jak **GEdit**, **Kate**, **VIM**, zmienić przypisanie do zmiennej **PicoCobraDir**, do której zamiast przykładowej ścieżki, nalezy podać pełną ścieżkę, w której jest niniejszy projekt, czyli katalogi **MCUME\_pico** i **pico\-sdk**\. Jest to czynność jednorazowa\.

Następnie, każdorazowo w celu skompilowania programu, należy uruchomić skrypt Compile\.sh i w przypadku poprawnego kodu źródłowego, pojawi się plik **picocobra\.uf2**\.

Aby wprowadzić plik do Picomputer, nalezy podłączyć urzadzenie do USB trzymając przy tym przycisk **BOOTSEL** na przytwierdzonym do niej modułu Raspberry Pi Zero\. W komputerze zgłosi się jako dodatkowy dysk\. na który nalezy skopiowac plik **picocobra\.uf2**, po czym nastąpi samoczynne odłączenie dysku\.

# Konfiguracja emulatora

Emulator wykorzystuje pliki z karty SD włożonej do czytnika znajdującym się w Picomputer\. Na karcie SD, w katalogu głównym powinien być podkatalog **cobra**, a w nim plik **cobra1\.cfg**, który jest taki sam, jak w emulatorze [Cobra1](https://github.com/andrzejlisek/Cobra1 "https://github.com/andrzejlisek/Cobra1"), jednak oczytywane są tylko niektóre parametry:


* **CpuMem\_ScreenMemCommon** \- Określa, czy pamięć w zakresie **F800\-FBFF** i **FC00\-FFFF** jest tą samą pamięcią \(jak w oryginalnym komputerze Cobra1\), czy dwoma osobnymi obszarami pamięci \(wymagane do wyświetlania kolorów\)\.
* **Screen\_ScreenColor** \- Określa, czy używać kolorów \(kolor jeszcze nie zaimplementowany\)\.
* **FileChr** \- Nazwa pliku ze wzorami znaków na ekranie\.
* **FileLst** \- Nazwa pliku listy banków pamięci ROM\. Budowa pliku identyczna, jak do emulatora [Cobra1](https://github.com/andrzejlisek/Cobra1 "https://github.com/andrzejlisek/Cobra1")\.
* **FileRom** \- Nazwa pliku pamięci ROM\.
* **FileRam** \- Nazwa pliku początkowej zawartości pamięci RAM\. W przypadku braku początkowej zawartości dla danej konfiguracji, wartośc parametru powinna byc pusta lub parametr może zostać pominięty\.

Obecnie, w projekcie znajduje się folder **cobra** z przykładową zawartością podkatalogu karty SD:


* **cobra1\.cfg** \- Przykładowy plik konfiguracyjny\.
* **s\_chr\.rom** \- Oryginalny zestaw znaków\.
* **s\_cobra1\.lst** \- Lista banków ROM oryginalnego komputera \(w rzeczywistości jest to jeden i ten sam bank przypisany dla każdej pozycji\)\.
* **s\_cobra1\.rom** \- Plik ROM oryginalny z wprowadzoną obsługą kombinacji SH\+T według oryginalnej dokumentacji\.
* **s\_cobra1\.ram** \- Plik zawartości RAM zawierajacy interpreter BASIC, który oryginalnie nalezało wgrać z kasety magnetofonowej\.
* **z\_chr\.rom** \- Zestawy znaków do współpracy z plikiem **z\_cobra1\.rom**\.
* **z\_cobra1\.lst** \- Lista banków pakietu ROM opracowanego przez **Zdzis\_ek**\., współpracuje z plikiem **z\_cobra1\.com** i **x\_cobra1\.rom**\.
* **z\_cobra1\.rom** \- Pakiet ROM opracowanego przez **Zdzis\_ek**\.
* **x\_chr\.rom** \- Zestawy znaków do współpracy z plikiem **x\_cobra1\.rom**\.
* **x\_cobra1\.rom** \- Pakiet ROM opracowanego przez **Zdzis\_ek**\. Wykorzystuje liste banków **z\_cobra1\.lst**\.

# Obsługa i działanie

Zaraz po uchuchomieniu **Picomputer**, uruchamia się emulator **Cobra1** według bieżącej konfiguracji\. Ze względu na zasadę działania komputera **Cobra1** i oznaczenia klawiszy, klawiatura nie jest bezpośrednim odzwierciedleniem klawiatury oryginalnego komputera\.

W oryginalnym komputerze znaki były wprowadzane w następujący sposób:


* Bezpośrednio wprowadzane były tylko litery i cyfry\.
* Wielkość liter była przełączana poprzez kombinache **SH\+T**\. Domyślnie wprowadzane sa wielkie litery\.
* Znaki specjalne wprowadza sie poprzez naciśnięcie klawisza SH i litery, do której jest przypisany dany znak specjalny\.

W **Picocobra** klawisze działają w nastepujący sposób:


* Bezpośrednie wciskanie klawiszy wprowadza wielkie litery\.
* Aby wprowadzić małą literę, należy przytrzymać klawisz **ESC/TAB** i nacisnąc daną literę\. Emulator automatycznie zasymuluje naciśnięcie **SH\+T** w razie potrzeby\.
* W przypadku potrzeby wprowadzenia znaku specjalnego lub cyfry, należy nacisnąć i przytrzymać klawisz **FN** i nacisnąć odpowiedni klawisz\. Emulator zasymuluje naciśnięcie odpowiedniej kombinacji klawiszy\.
* Klawisz **DEL** symuluje klawisz **SH** w przypadku naciśnięcia litery\. Np\. naciśnięcie **DEL\+T** spowoduje taki sam skutek, jak naciśnięcie klawisza **SH\+T** na oryginalnym komputerze, czyli odwrócenie działania wielkości liter\.
* Przytrzymanie klawisza **FN** i naciśnięcie **DEL** przełącza miedzy widokiem ekranu emulatora, a ekranu informacyjnego\.

# Ekran informacyjny

Ekran informacyjny zawiera informacje, jakie parametry i wartości zostały wczytane z pliku **cobra1\.cfg**\. Na górze są parametry dotyczące pamięci i wyświetlania i możliwe są następujaće napisy:


* **Separa** \- Odczytano **CpuMem\_ScreenMemCommon=0**\.
* **Common** \- Odczytano **CpuMem\_ScreenMemCommon=1**\.
* **Mono** \- Odczytano **Screen\_ScreenColor=0**\.
* **Color** \- Odczytano **Screen\_ScreenColor=1**\.

Poniżej są wielkości i nazwy plików odczytane z parametrów **FileChr**, **FileLst**, **FileRom** i **FileRam**\. jeżeli dany parametr istnieje, ale nie istnieje wskazany plik, będzie podany rozmiar pliku **0**\.

Na dole ekranu są podane surowe wartości stanu linii klawiatury, przy naciskaniu klawiszy pojawiają się zmiany stanu tych linii\. jest to stan bezpośrednio odczytany z **Picomputer**, przed przetworzeniem na potrzeby emulatora\.




