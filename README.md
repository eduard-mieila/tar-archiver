# TAR Archiver

## Descriere Implementare
Aplicația are ca scop principal arhivarea de fișiere dintr-un folder, dezarhivarea și afișarea elementelor unei arhive. Arhivele cu care aplicația rulează sunt doar cele de tip .tar(informații cu privire la acest tip de arhive se pot găsi [aici](https://www.fileformat.info/format/tar/corion.htm)). Pentru acest lucru am folosit biblioteca standard C pentru crearea de structuri ce conțin datele despre fișiere, completarea acestor câmpuri și formarea arhivei. 

## How to use
### Build
	make build

### Run
	./archiver

### Commands
Pentru creare de arhive:
	
	create nume_arhiva nume_director/

Pentru afișarea fișierelor din cadrul unei arhive:

	list nume_arhiva

Pentru extragerea unui singur fișier din cadrul unei arhive:

	extract nume_fisier nume_arhiva


### Exemplu Utilizare
	user@user-pc:~\$ ./archiver
	create arhiva_mea.tar
	> Wrong command!
	create arhiva_mea.tar imagini/
	> Done!
	list arhiva_mea.tar
	> imagine1.png
	> imagine2.png
	> imagine3.png
	create arhiva1.tar fisiere_text/
	> Done!
	extract f0.txt arhiva1.tar
	> File not found!
	extract f1.txt arhiva1.tar
	> File extracted!
	exit

## Detalii suplimentare despre implementare
Pe parcurs, pentru transformarile valorilor numerice din baza 10 in baza 8 si invers vom folosi functiile DtoO, UItoO, OtoUI, LUtoO.

---

### Interacțiunea cu utilizatorul
Vom citi linia de instructiuni folosind comanda readCommand. 
Aceasta va completa variabilele command si param1 respectiv param2, daca va fi cazul.
Comenzile pe care programul le poate primi sunt "codificate" de functia commandChoice astfel:
- 0 - exit
- 1 - create
- 2 - list
- 3 - extract
- -1 - comanda necunoscuta

Se vor face apoi verificarile necesare fiecarei ramuri in parte.
La final comanda si param1 respectiv param2 vor fi stersi pentru citirea liniei urmatoare.

---

### Arhivare(comanda *archive*):
Se va folosi functia createArchive care primeste ca parametri numele arhivei ce urmeaza sa fie creata si folderul din care vor fi extrase fisierele.
Vom dechide fisierul files.txt si ii vom determina lungimea.
Apoi, vom extrage datele necesare din acest fisier si le vom scrie in header, folosind functia readFileData.
Pentru fiecare fisier identificat in files.txt vom deschide si fisierul usermap.txt.
Datele extrase sunt urmatoarele:
- permisiunile asupra fisierului, necesare pentru completarea campului MODE din header.
	> Se vor calcula ultimele 3 cifre din acest camp folosind functia getPerm, ele reprezentand  permisiunile.
	> Ceilalti bytes vor fi completati cu 0.
- se va citi apoi o valoare care nu ne este necesara
- Uname
-  Gname
- pe baza Uname si Gname tocmai citite, vom folosi functia getUIDGID care vor cauta UID si GID in usermap.txt.
	> Se va gasi utilizatorul caruia ii apartine fisierul, i se vor identifica UID-ul si GID-ul, vor fi transformati in baza 8 si apoi vor fi scrisi in header.
-dimensiunea fisierului; ea va fi transformata in baza 8 si apoi va fi scrisa in header in dreptul campului SIZE.
- data si ora modificarii fisierului.
	- Vom folosi functia transformDate care va extrage anul, luna si ziua ultimei modificari.
		> Datele extrase vor fi scrise intr-o structura de tipul tm din time.h.
		(Este important de mentionat faptul ca anul de referinta in aceasta structura este 1900, iar luna Ianuarie este considerata luna 0.)
	- De asemenea, vom folosi functia transformTime pentru a extrage ora, minutul si secunda ultimei modificari.
		> Datele extrase vor fi scrie in aceeasi structura in care am scris si data.
	- Dupa ce aceste date necesare au fost extrase, vom transforma structura intr-un format valabil de timp.
	- Vom calcula secundele trecute de la Epoch pana la modificarea fisierului folosind functia mktime. Aceasta valoare va fi apoi transformata in octal.
- se va citi apoi o valoare care nu ne este necesara
- numele care va fi completat in spatiile NAME SI LINKNAME
- se va scrie "GNUtar " in campul MAGIC
- in final, se va calcula CHKSUM, considerand campul CHKSUM fiind completat cu spatii(codul 30 in ASCII). Suma va fi scrisa in octal.

Headerul este alocat dinamic cu calloc, prin urmare, toate campurile vor fi completate din start cu valoarea 0.

Vom scrie headerul in arhiva. Vom retine numele fisierului. Vom elibera blocul de date pentru header.
Vom determina calea relativa a fisierului ce urmeaza sa fie scris in arhiva, adaugand folderul inaintea numelui fisierului.
Vom deschide fisierul pentru citire si il vom scrie in blocuri de cate 512 bytes. Ne vom folosi de charptr aflat in uniunea dataBlock. Aceasta va fi alocata dinamic.
> Pentru ca prin alocarea cu calloc, spatiile din dataBlock vor fi umplute cu 0, in momentul in care vom ajunge la finalul fisierului vom scrie blocul de date in arhiva, avand garantia ca restul spatiilor pana la 512 vor exista in arhiva finala.

Procedura va continua pana cand se va ajunge la finalul fisierului files.txt.
La final, vom scrie un bloc de date de 512 bytes gol la sfarsitul arhivei, reprezentand finalul acesteia.

---

### Afișarea fișierelor dintr-o arhivă(comanda *list*):
Pentru a afisa toate fisierele dintr-o arhiva vom folosi functia listArchive.
Functia va citi intr-un block de date un header. Pentru a ne asigura ca datele citite reprezinta un header vom verifica integritatea campului MODE, el fiind, in cazul nostru, mereu "GNUtar ".
De fiecare data cand in campul header, campul MAGIC va contine "GNUtar " vom afisa numele fisierului.


Extragerea unui fișier dintr-o arhivă(comanda *extract*):
Pentru a extrage un fisier dintr-o arhiva vom folosi functia extractFile.
Se va deschide arhiva pentru citire.
Vom cauta headerul fisierului cautat.
- In cazul in care acesta nu va fi gasit, vom afisa mesajul corespunzator.
- Cand vom gasi headerul, vom transforma valoarea din campul SIZE in baza 10.
	>Avand in vedere ca am citit un intreg block de 512 bytes, ne vom afla acum la inceputul fisierului corespunzator headerului, prin urmare vom citi byte cu byte din arhiva si vom scrie intr-un nou fisier cu numele corespunzator, pana cand vom atinge dimensiunea fisierului.
