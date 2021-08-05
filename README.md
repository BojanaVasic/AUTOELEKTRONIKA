# AUTOELEKTRONIKA

## KONTROLA SIGURNOSNIH POJASEVA

Postoji dva nacina za pokretanje sistema:
1. slanjem poruke start+ na kanalu 1 serijske komunikacije 
2. klikom na donju levu diodu

Pri pokretanju sistema pali se dioda u donjem desnom uglu i po default-u vrednost praga je 400 i oba putnika su odvezana.
Vrednost praga mozemo menjati slanjem stringa u formi prag vrednost+ na kanalu 1 (npr prag 600+).

Na kanalu 0 serijske komunikacije, koriscenjem opcije auto i slanjem trigger signala 'T', na svakih 100ms primamo informaciju o vrednosti senzora koje dobijamo iz
stringa u obliku npr 1 1 700+

Prva cifra moze da ima dve vrednosti: 1 - vezan vozac, 0 - odvezan vozac
Druga cifra moze da ima dve vrednosti: 1 - vezan suvozac, 0 - odvezan
Treci broj predstavlja vrednost analognog senzora. Ukoliko je ta vrednost veca od vrednosti praga, suvozac je prisutan, a ukoliko je manja, suvozac uopste nije u kolima,
pa njegov pojas ne uzimamo u razmatranje.

Ukoliko je bar jedan od putnika odvezan, izlazni LED stubac treba da treperi na svake dve sekunde u trajanju od 20s, ukoliko se nakon 20 sekundi ne vezu oba putnika(ili samo vozac, 
ukoliko je samo on prisutan), frekvencija ce se povecati, tj treperenje ce biti na 200ms.

Sve vreme na display-u i kanalu 1 serijske komunikacije dobijamo poruke:
-both - oba putnika odvezana
-left - odvezan vozac
-right - odvezan suvozac
-ili je prazan display, i bez slanja na serijsku ukoliko su oba zavezana, ili samo vozac(ukoliko nema suvozaca u kolima)

Sistem zaustavljamo slanjem poruke stop+ ili gasenjem diode u donjem levom uglu.
