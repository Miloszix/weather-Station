# weather-Station


Stacja pogodowa z podstacją


Miłosz Mynarczuk
2025-01-25

 
1.	TEMAT PROJEKTU
Tematem projektu było stworzenie stacji pogodowej wraz z drugą podstacją, która wysyła dane do stacji głównej.
2.	KOMPONENTY
Stacja główna:
ESP32 – mikrokontrolery sterujące układami
BME280 – czujniki temperatury, wilgotności i ciśnienia
Wyświetlacz TFT ILI9341 240x320px
RTC DS3231 – zegar czasu rzeczywistego
NRF24L01 – antena RF
Podstacja:
ESP32 – mikrokontrolery sterujące układami
BME280 – czujniki temperatury, wilgotności i ciśnienia
NRF24L01 – antena RF
Wyświetlacz oled SSD1306 128x64px
3.	SCHEMATY UKŁADÓW
Stacja główna:
 

Podstacja:
 
4.	OPIS DZIAŁANIA STACJI GŁÓWNEJ

Układ to rozbudowany system monitorujący dane środowiskowe, który składa się z dwóch głównych stacji: stacji głównej i podstacji. Jego zadaniem jest zbieranie, wyświetlanie oraz przesyłanie informacji o temperaturze, wilgotności i ciśnieniu atmosferycznym. Działanie układu można podzielić na następujące etapy:
________________________________________
1. Uruchamianie i konfiguracja
Po włączeniu układu inicjalizowane są kluczowe moduły i komponenty:
•	Wi-Fi – Stacja główna próbuje połączyć się z lokalną siecią Wi-Fi w celu synchronizacji czasu z serwerem NTP.
•	RTC (Real-Time Clock) – Układ czasu rzeczywistego DS3231 jest konfigurowany i synchronizowany z czasem pobranym z serwera NTP.
•	RF24 – Moduł komunikacji bezprzewodowej jest konfigurowany do odbierania danych z podstacji.
•	Wyświetlacz TFT – Inicjalizowany jest ekran TFT, który będzie wyświetlał aktualne dane.
________________________________________
2. Pomiar danych środowiskowych
•	Stacja główna mierzy lokalne parametry środowiskowe (temperaturę, wilgotność i ciśnienie) za pomocą czujnika BME280.
•	Podstacja przesyła dane środowiskowe bezprzewodowo do stacji głównej za pomocą modułu RF24.
________________________________________
3. Przetwarzanie danych
•	Po odebraniu danych z podstacji, układ przetwarza je i przygotowuje do wyświetlenia.
•	W przypadku stacji głównej dane z lokalnego czujnika oraz te odebrane z podstacji są odczytywane i przechowywane w odpowiednich zmiennych.
________________________________________
4. Wyświetlanie informacji
•	Wyświetlacz TFT prezentuje zarówno lokalne dane środowiskowe (stacja główna), jak i dane odebrane z podstacji.
•	Na ekranie wyświetlane są:
o	Bieżący czas i data.
o	Parametry środowiskowe z obu stacji: temperatura, wilgotność i ciśnienie.
o	Ikony symbolizujące poszczególne pomiary.
________________________________________
5. Synchronizacja czasu
Jeśli układ jest podłączony do sieci Wi-Fi, regularnie synchronizuje czas z serwerem NTP. Synchronizacja odbywa się co godzinę, aby zapewnić dokładne odczyty.
________________________________________
6. Obsługa przerwań i timerów
•	Przerwania: Moduł RF24 korzysta z przerwań, aby natychmiast reagować na odebrane dane.
•	Timery: Dwa niezależne timery cyklicznie wyzwalają:
o	Odczyt danych z czujników.
o	Aktualizację wyświetlacza.
________________________________________
7. Działanie w pętli głównej
W pętli głównej układ:
•	Odczytuje dane z czujników.
•	Sprawdza, czy odebrano nowe dane z podstacji.
•	Regularnie aktualizuje wyświetlacz.
•	Monitoruje połączenie Wi-Fi i, w razie potrzeby, synchronizuje czas z serwerem NTP.
________________________________________
Podsumowanie
Układ działa jako system stacji meteorologicznej, zbierając i przetwarzając dane z dwóch lokalizacji (stacji głównej i podstacji). Umożliwia wyświetlanie wyników na ekranie oraz automatyczną synchronizację czasu. Dzięki połączeniu Wi-Fi, komunikacji RF24 i wykorzystaniu różnych czujników, system jest elastyczny, dokładny i wygodny w obsłudze.

5.	OPIS DZIAŁANIA PODSTACJI

Układ został zaprojektowany na bazie mikroprocesora ESP32, wyświetlacza OLED, czujników środowiskowych oraz modułu RF24, aby umożliwić monitorowanie parametrów środowiskowych, takich jak temperatura, ciśnienie i wilgotność. Odczytane dane są wyświetlane na ekranie OLED oraz przesyłane bezprzewodowo do zdalnego odbiornika.
________________________________________
1. Konfiguracja wyświetlacza OLED (SSD1306)
•	Wyświetlacz OLED o rozdzielczości 128x64 pikseli jest używany do prezentacji danych środowiskowych w formie graficznej.
•	Komunikacja między ESP32 a OLED odbywa się za pomocą interfejsu I2C.
•	Dane są wyświetlane w cyklicznych ekranach, z których każdy prezentuje inny parametr (temperatura, ciśnienie, wilgotność). Ekrany zmieniają się automatycznie co 3 sekundy, co realizowane jest za pomocą timera sprzętowego.
2. Konfiguracja modułu RF24
•	Moduł RF24 umożliwia bezprzewodową komunikację między układami.
•	Dane z czujników środowiskowych są zapisywane w strukturze danych, a następnie przesyłane do odbiornika.
•	W celu zapewnienia niezawodnej transmisji, komunikacja RF24 korzysta z unikalnych adresów, co umożliwia prawidłowe połączenie między nadajnikiem a odbiornikiem.


3. Konfiguracja czujników środowiskowych
•	Czujniki środowiskowe mierzą trzy podstawowe parametry: temperaturę, ciśnienie atmosferyczne oraz wilgotność względną.
•	Dane są odczytywane co 5 sekund za pomocą dedykowanego timera sprzętowego, co zapewnia regularne aktualizacje pomiarów.
4. Obsługa timerów sprzętowych
•	W układzie wykorzystano dwa niezależne timery ESP32:
o	Timer 1: odpowiada za odczyt danych z czujników co 5 sekund.
o	Timer 2: steruje zmianą ekranów wyświetlacza OLED co 3 sekundy, umożliwiając prezentację kolejnych parametrów w sposób cykliczny.
•	Timery ustawiają odpowiednie flagi, które są przetwarzane w pętli głównej.
5. Prezentacja danych na wyświetlaczu
•	Wyniki pomiarów środowiskowych są prezentowane na wyświetlaczu OLED w formie przejrzystych ekranów:
o	Ekran 1: Temperatura wraz z ikoną termometru.
o	Ekran 2: Ciśnienie atmosferyczne z ikoną barometru.
o	Ekran 3: Wilgotność względna z ikoną kropli wody.
•	Każdy ekran zawiera odpowiednio sformatowane dane (np. jednostki: °C, hPa, %).
6. Praca w pętli głównej
•	Pętla główna układu obsługuje dwie kluczowe funkcje:
o	Odczyt danych z czujników po ustawieniu flagi przez timer.
o	Aktualizację wyświetlacza OLED, w zależności od aktywnego ekranu.
•	Po każdorazowym odczycie danych są one również wysyłane do zdalnego odbiornika za pomocą modułu RF24.
Układ zapewnia ciągłą pracę, regularny odczyt i przesył danych, a także przejrzystą prezentację parametrów środowiskowych, co czyni go użytecznym w zastosowaniach takich jak domowa stacja pogodowa lub monitor środowiskowy.
środowiskowego.





6.	OPIS DZIAŁANIA KODÓW

Sensor.h
Ten kod implementuje komunikację z czujnikiem środowiskowym BME280 przy użyciu platformy Arduino. Czujnik BME280 umożliwia pomiar temperatury, ciśnienia oraz wilgotności względnej. Kod został napisany w języku C++ z użyciem biblioteki Wire dla protokołu I2C.
Kod obejmuje:
1.	Inicjalizację czujnika i jego konfigurację.
2.	Odczyt danych kalibracyjnych.
3.	Wykonanie pomiarów w trybie forced.
4.	Przeliczenie wartości surowych na rzeczywiste dane z uwzględnieniem kalibracji.
________________________________________

Funkcjonalności
1.	Inicjalizacja czujnika:
•	Sprawdzenie, czy BME280 jest poprawnie podłączony.
•	Ustawienie trybu pracy, oversamplingu i filtracji.
2.	Odczyt kalibracji:
•	Pobranie współczynników kalibracyjnych niezbędnych do przeliczeń temperatury, ciśnienia i wilgotności.
3.	Wykonywanie pomiarów:
•	Odczyt surowych wartości pomiarowych z czujnika.
4.	Przetwarzanie danych:
•	Przeliczenie surowych danych na rzeczywiste wartości w postaci temperatury (°C), ciśnienia (hPa) i wilgotności (%RH).
3. Główne funkcje programu
1.	Inicjalizacja czujnika
void initBME280();
•	Sprawdza poprawność połączenia z czujnikiem, odczytując rejestr ID (adres 0xD0).
•	Konfiguruje oversampling i tryb pracy.
•	Wywołuje funkcję odczytu kalibracji.
2.	Odczyt danych kalibracyjnych
void readCalibrationData();
•	Pobiera współczynniki kalibracyjne z czujnika i zapisuje je do zmiennych globalnych.
•	Używane do obliczeń temperatury, ciśnienia i wilgotności.
3.	Kompensacja surowych danych
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T);
Przelicza surową wartość temperatury (adc_T) na rzeczywistą temperaturę w °C. Podobnie wyglądają funkcje dla ciśnienia i wilgotności

4.	Odczyt surowych danych
void readRawData();
•	Włącza tryb forced, co inicjuje jednorazowy pomiar.
•	Pobiera surowe dane temperatury, ciśnienia i wilgotności z rejestrów czujnika (0xF7 - 0xFE).
•	Przelicza je na rzeczywiste wartości za pomocą funkcji kompensacyjnych.
5.	Pobieranie wyników
•	Temperatury:
float getTemp();
dzięki temu można pobrać dane w głównym kodzie, tak samo wygląda pobór wilgotności i ciśnienia

displayBMP.h
Ten kod odpowiada za odczyt pliku zapisanego w pamięci mikrokontrolera i zinterpretowanie go na formę możliwą do wyświetlania. Kod jest lekko zmodyfikowaną wersją kodu z przykładów w bibliotece TFTeSPI.h.

Bitmaps.h
Plik zawiera bitmapy z pomniejszymi ikonkami wyświetlanymi na ekranie.






Main.c głównej stacji:

1. setup()
•	Funkcja inicjalizuje wszystkie komponenty i ustawia środowisko działania programu.
•	Inicjalizacja komunikacji SPI i I2C dla czujników, wyświetlacza i modułu RF24.
•	Inicjalizacja modułu RF24 (ustawienie adresu, mocy nadawania, szybkości transmisji).
•	Konfiguracja i połączenie z siecią Wi-Fi.
•	Inicjalizacja modułu RTC DS3231 (jeśli utracono zasilanie RTC, synchronizuje go z czasem NTP).
•	Ustawienie dwóch timerów:
•	timerReadSensor: Odczyt danych z czujników co 5 sekund.
•	timerDispaly: Wyświetlanie czasu i danych na ekranie co 1 sekundę.
•	Wstępne rysowanie elementów na wyświetlaczu.
________________________________________
2. syncRTCWithNTP()
•	Synchronizuje moduł RTC DS3231 z serwerem czasu NTP.
•	Pobiera aktualny czas z serwera NTP i ustawia go w module RTC.
•	Wyświetla na serial monitorze bieżącą datę i czas.
________________________________________
3. loop()
•	Główna pętla programu wykonująca następujące operacje:
1.	Obsługa modułu RF24:
	Odczyt danych z innego urządzenia (temperatura, wilgotność, ciśnienie).
2.	Odczyt danych z czujników:
	Odczytuje dane z czujnika środowiskowego (temperatura, wilgotność, ciśnienie).
	Wyświetla odczytane wartości oraz dane z modułu RF24 w serial monitorze.
3.	Wyświetlanie czasu i danych:
	Aktualizuje czas, datę oraz wartości na wyświetlaczu OLED.
4.	Synchronizacja czasu RTC:
	Co godzinę synchronizuje RTC z serwerem NTP, jeśli Wi-Fi jest aktywne.
________________________________________

4. onTimerRead()
•	Funkcja ISR wywoływana przez timerReadSensor co 5 sekund.
•	Ustawia flagę readSensors na true, informując główną pętlę programu o konieczności odczytu danych z czujników.
________________________________________
5. onTimerDisplay()
•	Funkcja ISR wywoływana przez timerDispaly co 1 sekundę.
•	Ustawia flagę displayTime na true, co powoduje aktualizację wyświetlacza w głównej pętli programu.
________________________________________
6. DataReceived()
•	Funkcja ISR wywoływana po odebraniu danych przez moduł RF24.
•	Ustawia flagę dataReceived na true, co umożliwia przetworzenie odebranych danych w głównej pętli programu.
________________________________________
7. readRawData(), getHum(), getTemp(), getPress()
•	Funkcje odpowiedzialne za odczyt surowych danych z czujnika środowiskowego.
•	readRawData(): Inicjuje odczyt wszystkich parametrów.
•	getHum(): Zwraca wilgotność.
•	getTemp(): Zwraca temperaturę.
•	getPress(): Zwraca ciśnienie atmosferyczne.
________________________________________
8. Wyświetlanie danych na wyświetlaczu TFT
•	Aktualizacja czasu i daty:
•	Wyświetla godzinę i datę na środku górnej części ekranu.
•	Wyświetlanie danych środowiskowych:
•	Wykorzystuje bitmapy (np. ikonę temperatury, wilgotności) i wyświetla dane w dwóch kolumnach:
	Lewa kolumna: Dane lokalne.
	Prawa kolumna: Dane z odległej stacji (przesłane przez RF24).
________________________________________


9. Inicjalizacja RF24
•	Ustawienia modułu RF24:
•	Ustawienie adresu komunikacji.
•	Ustawienie poziomu mocy nadawania.
•	Włączenie trybu odbiornika.
•	Obsługa przerwania (IRQ) dla sygnalizowania odebrania danych.
________________________________________
Dzięki powyższym funkcjom program zapewnia płynną współpracę między wszystkimi modułami: RF24, RTC, wyświetlaczem i czujnikami środowiskowymi.

Main.c Podstacji:

1. setup()
Funkcja inicjalizująca system:
•	Inicjalizacja SPI i I2C: Ustawia połączenie SPI i I2C, używane przez nRF24L01 i czujniki BME280.
•	Inicjalizacja wyświetlacza OLED: Sprawdza, czy wyświetlacz OLED (SSD1306) działa poprawnie i przygotowuje go do wyświetlania danych.
•	Inicjalizacja modułu RF24: Ustawia moduł RF24 do przesyłania danych do innego urządzenia. Otwiera kanał komunikacyjny, ustawia moc i prędkość transmisji.
•	Inicjalizacja timerów:
o	Timer do odczytu danych (timerReadSensor): Uruchamia odczyt danych z czujników co 5 sekund.
o	Timer do zmiany wyświetlacza (timerDisplay): Ustawia cykliczną zmianę wyświetlanych danych na OLED co 3 sekundy.
________________________________________
2. onTimerRead()
ISR (Interrupt Service Routine), która ustawia flagę readSensors na true, aby wskazać, że czas odczytać dane z czujników.
________________________________________
3. onTimerDisplay()
ISR zmieniająca indeks aktualnie wyświetlanego ekranu (currentDisplay) i ustawia flagę updateDisplay na true, aby zaktualizować wyświetlacz OLED.

4. showTemp(float temperature)
Wyświetla aktualną temperaturę na ekranie OLED:
•	Rysuje bitmapę termometru.
•	Wyświetla wartość temperatury z jednostką °C.
________________________________________
5. showHum(float humidity)
Wyświetla aktualną wilgotność na ekranie OLED:
•	Rysuje bitmapę przedstawiającą wilgotność.
•	Wyświetla wartość wilgotności z jednostką %.
________________________________________
6. showPress(float pressure)
Wyświetla aktualne ciśnienie atmosferyczne na ekranie OLED:
•	Rysuje bitmapę przedstawiającą ciśnienie.
•	Wyświetla wartość ciśnienia z jednostką hPa.
________________________________________
7. loop()
Główna pętla programu, odpowiedzialna za obsługę systemu:
•	Odczyt danych z czujników: Jeśli flaga readSensors jest ustawiona, wykonuje:
•	Odczyt danych surowych z czujnika BME280.
•	Aktualizację zmiennych temperatury, ciśnienia i wilgotności.
•	Wysyłanie danych przez moduł RF24.
•	Aktualizacja wyświetlacza OLED: Jeśli flaga updateDisplay jest ustawiona, wykonuje:
•	Wyświetlenie odpowiednich danych (temperatura, ciśnienie lub wilgotność) w zależności od wartości currentDisplay.
________________________________________
8. Struktura SensorData
Struktura do przechowywania danych z czujników:
•	temperature: Temperatura odczytana z czujnika.
•	pressure: Ciśnienie atmosferyczne.
•	humidity: Wilgotność względna.


9. Ogólne działanie
•	System cyklicznie odczytuje dane z czujnika BME280 (co 5 sekund) i wysyła je za pomocą modułu RF24.
•	Wyświetlacz OLED zmienia ekran co 3 sekundy, prezentując temperaturę, ciśnienie lub wilgotność w zależności od aktualnego stanu currentDisplay.
7.	Biblioteki użyte w projekcie

1.	<Arduino.h>
•	Biblioteka bazowa platformy Arduino, zawierająca podstawowe definicje i funkcje, takie jak setup(), loop(), obsługę pinów oraz przerwań.
2.	<Wire.h>
•	Biblioteka umożliwiająca komunikację poprzez protokół I2C.
•	W projekcie służy do komunikacji z wyświetlaczem OLED (SSD1306) i czujnikiem BME280.
3.	<SPI.h>
•	Biblioteka obsługująca protokół komunikacji SPI.
•	W projekcie używana do komunikacji z modułem radiowym nRF24L01.
4.	<Adafruit_SSD1306.h>
•	Biblioteka Adafruit do obsługi wyświetlacza OLED SSD1306.
•	Umożliwia łatwe rysowanie bitmap, tekstów oraz innych elementów graficznych na ekranie.
•	W projekcie obsługuje wyświetlanie danych na ekranie OLED.
5.	<Adafruit_GFX.h>
•	Biblioteka graficzna wspomagająca bibliotekę SSD1306.
•	Zapewnia funkcje do rysowania kształtów (np. prostokąty, linie, bitmapy) i tekstów.
•	W projekcie wykorzystywana przy rysowaniu bitmap (ikon temperatury, wilgotności, ciśnienia) na OLED.
6.	<nRF24L01.h>
•	Specjalistyczna biblioteka zawierająca definicje dla modułu nRF24L01, odpowiedzialnego za komunikację radiową.
•	Używana jako podstawa dla funkcji RF24.
7.	<RF24.h>
•	Biblioteka obsługująca moduł radiowy nRF24L01.
•	Zapewnia funkcje takie jak otwieranie kanału komunikacyjnego, wysyłanie i odbieranie danych.
•	W projekcie służy do przesyłania danych z czujników do innego urządzenia.
8.	<FS.h>
•	Biblioteka systemu plików dla mikrokontrolerów, zapewniająca abstrakcję dla operacji na plikach, takich jak otwieranie, zapisywanie, odczyt czy usuwanie plików.
•	Wykorzystywana w projektach korzystających z wewnętrznej pamięci mikrokontrolera do przechowywania danych.
9.	 <SPIFFS.h>
•	Biblioteka implementująca system plików SPIFFS (SPI Flash File System).
•	Umożliwia zapis, odczyt i organizację plików w pamięci flash mikrokontrolera.
•	W projekcie używana do przechowywania np. konfiguracji, obrazów lub innych danych potrzebnych podczas działania aplikacji.
10.	  <TFT_eSPI.h>
•	Biblioteka obsługująca wyświetlacze TFT, oparta na sterownikach SPI (np. ILI9341, ST7789, ST7735).
•	Zapewnia funkcje do rysowania tekstów, kształtów, obrazów bitmapowych oraz innych elementów graficznych.
•	W projekcie używana do sterowania wyświetlaczem TFT, np. do wizualizacji danych lub interfejsu użytkownika.
11.	 <WiFi.h>
•	Biblioteka do obsługi Wi-Fi na mikrokontrolerach ESP32.
•	Pozwala na podłączenie do sieci Wi-Fi, tworzenie punktów dostępowych (AP), obsługę klienta HTTP, TCP, UDP i innych protokołów sieciowych.
•	W projekcie umożliwia np. wysyłanie danych do serwera, odbieranie zdalnych poleceń, aktualizację OTA lub komunikację w sieci lokalnej.



