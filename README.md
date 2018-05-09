# MPI_PSRS

#### Kroki do skompilowania i uruchomienia:

1. Przejdź do katalogu z projektem i plikiem makefile MPI_PSRS: ```cd MPI_PSRS``` <br/>

##### Jeden węzeł:
2. Wykonaj ```make run N=<liczba procesów> FILE=sample/input_50.data```

##### Kilka węzłów:
2. Wykonaj ```make run N=<liczba procesów> FILE=sample/input_50.data NODES=nodes```,
gdzie plik nodes zawiera informację o adresach węzłów
