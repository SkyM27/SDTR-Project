# SDTR-Project-Sistem-de-Alarmă-Laser-cu-LDR-și-FreeRTOS

Specificațiile Produsului:

1. Introducere
Acest proiect prezintă un sistem de alarmă pe bază de LDR, realizat pe platforma NUCLEO STM32-F446RE, ce utilizează un sistem de operare în timp real (FreeRTOS) pentru a crea un sistem eficient de monitorizare a securității. Sistemul are o aplicație dedicată pentru armarea și dezarmarea alarmei, acesta detectând orice întrerupere a fasciculului laser și declanșează o alarmă acustică.

2. Obiective
•	Crearea a 4 task-uri cu diferite priorități, unul fiind de tipul real-time
•	Armarea și dezarmarea sistemului printr-o conexiune Bluetooth.
•	Implementarea unui semafor binar pe task-uri.
•	Detectarea precisă a întreruperilor fasciculului laser.
•	Respectarea cerințelor de timp real și sincronizarea corectă între modulele software și hardware.
•	Măsurarea timpului de răspuns în cicluri CPU cu DWT-CYCCNT (raportat in microsecunde).
•	Generarea unei alarme acustice (buzzer) imediat de la detectarea unei întreruperi.

3. Descriere generală
•	Armarea sistemului: Sistemul este activat printr-un mesaj Bluetooth (“A” -> ARMED; “D” -> DISARMED).
•	Monitorizarea fasciculului laser: Laserul este menținut permanent ON (PB5). Starea fascicului este urmărită continuu prin fotorezistor (ADC1).
•	Declanșarea alarmei:
-	Detecție: când ADC > threshold_hi, task-ul LDRread marchează întreruperea fasciculului .
-	Task-ul LDRread dă osSemaphoreRelease(ldrSem).
-	Buzzer-ul iese din osSemaphoreWait, pune PB3 = HIGH (buzzer ON).
-	Timp de răspuns: se salvează t0 = DWT->CYCCNTc chiar înainte de release și t1 imediat după wait; se afișează t1-t0 pe USART2 (cicluri CPU și convertirea in microsecunde). Timpul de răspuns din proiect este de 20 microsecunde. 
-	Buzzer-ul se oprește odată cu mesajul din aplicație de DISARM.

4. Descrierea Hardware și Software
Hardware:
•	Placa de dezvoltare: NUCLEO STM32F446RE - ARM Cortex-M4.
•	Componente adiționale:
o	Fotorezistor (LDR) – ADC1
o	Modul laser – PB5
o	Buzzer active – PB3
o	Modul Bluetooth HC-05 – USART1, BAUDRATE 9600
•	Alte accesorii: cabluri de conexiune, cablu de alimentare miniUSB.
Software:
•	IDE: STM32CubeIDE
•	Sistem de operare: FreeRTOS
•	Librării STM32 HAL: ADC, GPIO, UART + CMSIS core_cm4 pentru DWT/CYCCNT
•	Consolă serială: Tera TERM pe COM-ul ST-Link (USART2) pentru afișarea timpului de răspuns și mesaje pentru debugging (afișare valoare LDR, starea semaforului binar).

5. Task-uri și priorități (FreeRTOS)
•	LDRreadTask – osPriorityRealtime
•	BTreadTask – osPriorityAboveNormal
•	BuzzerTask – osPriorityAboveNormal
•	LaserWriteTask – osPriorityLow
