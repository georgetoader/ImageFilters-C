# image-processing 
Pentru a citi fiecare test am folosit "sprintf" pentru a modifica path-ul catre
fisiere in fuctie de numarul testului si dupa deschiderea fisierelor de input/output
am citit matricea de pixeli si am scris in fiecare fisier de output fileheader-ul.
### TASK 1
Am parcurs matricea de pixeli si am folosit relatia specificata in
enuntul cerintei pentru a calcula noua valoare a pixelilor. Dupa atribuirea acestei 
valori am scris in fisierul de output pixelii de pe acea linie si apoi padding-ul
calculat.
### TASK 2
am calculat noile dimensiuni ale imaginii si am completat noua
imagine in cele 2 cazuri, daca inaltimea este mai mare sau invers. 
### TASK 3
am citit matricea filtrului si am construit pentru fiecare pixel
care apartinea imaginii o matrice mai mica de dimensiune "filterSize" careia i-am
inmultit elementele cu matricea de filtru.
### TASK 4
am citit datele din fisierul de pooling si am construit ca la TASK3
o matrice mai mica pentru fiecare pixel in care am calculat valoarea min/max cu
ajutorul unor functii. Daca pixelul calculat se afla in afar imaginii ii atribuiam
valoarea 0.
### TASK 5
am construit o structura Queue si o matrice "matrix" ce retinea 
pixelii ce au fost integrati intr-un grup. La fiecare pixel neintegrat aplicam 
o versiune modificata a algoritmului lui Lee pentru a gasi toti pixelii din threshold.
