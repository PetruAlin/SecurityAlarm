Tomescu Petru - Alin

## Continut:
- AlarmSystem.sch, System_Alarm_Schema_Electrica.png sunt schema electrica a proiectului;
- AlarmNotification.aia reprezinta codul aplicatiei de android ce poate fi editat cu MIT app inventor;
Link: https://appinventor.mit.edu/ ;
- SecurityAlarm.ino reprezinta codul pentru arduino;

## Moduri functionare alarma:
# Initializare:
- Setarea pinilor pentru PIR (INPUT), Buzzer(OUTPUT), precum si a celui pentru Buton (INPUT PULLUP);
- Activarea intreruperilor externe pentru buton;
- Initializarea LCD-ului;
- Pornirea modului bluetooth, moment din care aplicatia poate fi conectata la acesta;
- Citirea parolei, setata in cadrul functionarii anterioare, de pe cardul micro-sd, precum si salvarea ei in memorie (prin functia getPassword());
- Initializarea keypad-ului;
- Apelarea functiei de reset, pentru a permite utilizatorului sa reseteze parola, inainte de inceperea sistemului;
- Calibrarea modului PIR, prin asteptarea unui minut pentru a se obsinui cu temperatura camerei;

# Pe durata functionarii sistemului de alarma, exista 4 stari(moduri) in care acesta se afla:
- Modul 0 (Activ): Aici are loc citirea senzorului PIR. In cazul in care acesta a detectat miscare, se afiseaza pe LCD mesajul corespunzator, se trimite o alerta pe aplicatie. Apoi se face trecerea sistemului in modul 1. Tot aici, in cazul in care butonul este apasat, se trece in modul 3 unde se realizeaza resetarea parolei sistemului si salvarea ei atat in memoria programului, cat si pe cardul micro-sd.
- Modul 1 (Miscare detecata): In acest mod, miscare a fost detectata, deci este pornit si sunetul buzzerului. In cazul in care se primeste un mesaj de oprire de la aplicatie, alarma se opreste si are loc recalibrarea senzorului PIR (asteptarea 1 minut), precum si trecerea in modul 0. In cazul in care se apasa be buton, se realizeza trecerea in modul 2, pentru introducerea parolei.
- Modul 2 (Introducere parola): In acest mod, poate fi introdusa parola. In cazul in care aceasta este incorecta, se trece inapoi in modul 1. Daca parola e corecta, se realizeaza recalibrarea senzorului PIR, precum si trecerea inapoi in modul 0.
- Modul 3 (Resetare parola): In acest mod se poate ajunge doar prin apasare butonului, din modul 0. Are loc resetarea parolei, precum si intoarcerea in modul 0.

## Descriere  functii implementate:
# calibratePir():
Functia realizeaza asteptarea unui minut, ceea ce permite resetarea senzorului PIR pentru a putea detecta din nou.
# setPassword():
Functia asteapta introducerea unei parole de 4 digits de la keypad, si o salveaza in memoria programului.
# getPassword():
Functia citeste din memoria cardului micro sd parola, si o incarca in memoria programului.
# setButton():
Functia realizeaza setarea pinului butonului, in mod INPUT PULLUP.
# setPins():
Functia realizeaza setarea pinilor pentru PIR si Buzzer, precum si pentru buton prin apelarea lui setButton().
# setInterupts():
Functia seteaza intreruperile externe, declansate la apasarea butonului.
# initLCD():
Functia initializeaza LCD-ul.
# writeToCard():
Functia scrie pe cardul micro-sd parola curenta salvata in memorie.
# reset():
Functia permite resetarea parolei curente, utilizatorul putand accepta (apasa tasta A) sau nu (apasa tasta D). Daca decide resetarea, parola este setata prin functia setPassword() si apoi scrisa pe cardul micro-sd cu un apel al functiei writeToCard().
introducePassword():
Functia asteapta introducerea unei parole de 4 cifre de la tastatura. Daca aceasta este corecta, are loc recalibrarea senzorului PIR (un apel calibratePir()), precum si trecerea in modul 0 de functionare. Daca parola este gresita, sistemul va ramane in modul 1, iar alarma va ramane activa.