#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <stdbool.h>

#define MAX_VEHICLES 512

typedef struct { // the highest value is vehicles->autonomy[0]
    int *autonomy;
    int num_vehicles;
} Vehicles;

typedef struct {
    int distance;
    Vehicles *cars;
} Station; // Node

typedef struct {
    int distance;
    int distancePlusMax;
} StationForPath;

typedef struct {
    Station *stations;
    int num_stations;
} Highway; //Graph


// region initialization and deletion of graph
void initializeHighway(Highway *highway) {
    highway->num_stations = 0;
    highway->stations = NULL;
}

void freeHighway(Highway *highway) {
    for (int i = 0; i < highway->num_stations; i++) {
        free(highway->stations[i].cars->autonomy);
        free(highway->stations[i].cars);
    }
    free(highway->stations);
}
// endregion initialization and deletion of graph

// region max heap methods for vehicles in each station
void maxHeapify(Vehicles *v, int index) {
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    int max = index;

    if (left >= v->num_vehicles || left < 0) {
        left = -1;
    }
    if (right >= v->num_vehicles || right < 0) {
        right = -1;
    }

    if (left != -1 && v->autonomy[left] > v->autonomy[max]) {
        max = left;
    }
    if (right != -1 && v->autonomy[right] > v->autonomy[max]) {
        max = right;
    }

    if (max != index) {
        int temp = v->autonomy[max];
        v->autonomy[max] = v->autonomy[index];
        v->autonomy[index] = temp;

        maxHeapify(v, max);
    }
}

void insertionHelper(Vehicles *v, int index) {
    int parent = (index - 1) / 2;

    if (v->autonomy[parent] < v->autonomy[index]) {
        int temp = v->autonomy[parent];

        v->autonomy[parent] = v->autonomy[index];
        v->autonomy[index] = temp;

        insertionHelper(v, parent);
    }
}

Vehicles *createHeap(int num_vehicles, int *cars) {
    // allocating memory to v
    Vehicles *v = (Vehicles *) malloc(sizeof(Vehicles));

    // checking if memory is allocated
    if (v == NULL) {
        return NULL;
    }

    v->num_vehicles = num_vehicles;
    // allocating memory to array of cars
    v->autonomy = (int *) malloc(MAX_VEHICLES * sizeof(int));

    // checking if memory is allocated
    if (v->autonomy == NULL) {
        return NULL;
    }

    int i;
    for (i = 0; i < num_vehicles; i++) {
        v->autonomy[i] = cars[i];
    }

    i = (v->num_vehicles - 2) / 2;
    while (i >= 0) {
        maxHeapify(v, i);
        i--;
    }

    return v;
}

void addVehicle(Highway *highway, int distance, int vehicle) {
    for (int i = 0; i < highway->num_stations; i++) {
        if (highway->stations[i].distance == distance) {
            int num_vehicles = highway->stations[i].cars->num_vehicles;

            if (num_vehicles < MAX_VEHICLES) {
                highway->stations[i].cars->autonomy[num_vehicles] = vehicle;
                insertionHelper(highway->stations[i].cars, highway->stations[i].cars->num_vehicles);
                highway->stations[i].cars->num_vehicles++;

                printf("aggiunta\n");
                return;
            }
            printf("non aggiunta\n"); // max vehicles reached
            return;
        }
    }

    printf("non aggiunta\n"); // station not found
}

void removeVehicle(Highway *highway, int distance, int vehicle) {
    for (int i = 0; i < highway->num_stations; i++) {
        if (highway->stations[i].distance == distance) {
            int num_vehicles = highway->stations[i].cars->num_vehicles;

            if (num_vehicles == 0) {
                printf("non rottamata\n");
                return;
            }

            for (int j = 0; j < num_vehicles; j++) {
                if (highway->stations[i].cars->autonomy[j] == vehicle) {
                    highway->stations[i].cars->autonomy[j] = highway->stations[i].cars->autonomy[num_vehicles - 1];
                    highway->stations[i].cars->num_vehicles--;
                    maxHeapify(highway->stations[i].cars, j);
                    printf("rottamata\n");
                    return;
                }
            }
            printf("non rottamata\n"); // vehicle not found
            return;
        }
    }

    printf("non rottamata\n"); // station not found
}
// endregion max heap methods for vehicles in each station

// region graph methods
void addStation(Highway *highway, int distance, int num_vehicles, int *cars) {
    // check if a station already exists at the given distance
    for (int i = 0; i < highway->num_stations; i++) {
        if (highway->stations[i].distance == distance) {
            printf("non aggiunta\n");
            return;
        }
    }

    // add the new station
    highway->num_stations++;
    highway->stations = (Station *) realloc(highway->stations, highway->num_stations * sizeof(Station));
    highway->stations[highway->num_stations - 1].distance = distance;
    highway->stations[highway->num_stations - 1].cars = createHeap(num_vehicles, cars);

    printf("aggiunta\n");
}

void removeStation(Highway *highway, int distance) {
    for (int i = 0; i < highway->num_stations; i++) {
        if (highway->stations[i].distance == distance) {
            free(highway->stations[i].cars->autonomy);

            for (int j = i; j < highway->num_stations - 1; j++) {
                highway->stations[j] = highway->stations[j + 1];
            }

            highway->num_stations--;
            highway->stations = (Station *) realloc(highway->stations, highway->num_stations * sizeof(Station));

            printf("demolita\n");
            return;
        }
    }

    printf("non demolita\n");
}
// endregion graph methods

// region find path methods
void swap(Station *s1, Station *s2) {
    Station temp = *s1;
    *s1 = *s2;
    *s2 = temp;
}

void heapify(Highway *highway, int dim, int index) {
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < dim && highway->stations[left].distance > highway->stations[largest].distance) {
        largest = left;
    }

    if (right < dim && highway->stations[right].distance > highway->stations[largest].distance) {
        largest = right;
    }

    if (largest != index) {
        swap(&highway->stations[index], &highway->stations[largest]);
        heapify(highway, dim, largest);
    }

}

void orderStationsHeapSort(Highway *highway) {
    int dim = highway->num_stations;

    for (int i = dim / 2 - 1; i >= 0; i--) {
        heapify(highway, dim, i);
    }

    for (int i = dim - 1; i > 0; i--) {
        swap(&highway->stations[0], &highway->stations[i]);
        heapify(highway, i, 0);
    }

}

bool isPresent(StationForPath *path, int dim, int distance) {
    for (int i = 0; i < dim; i++) {
        if (path[i].distance == distance)
            return true;
    }
    return false;
}

void findShortestPath(Highway *highway, int start, int target) {
    Station starting_station;
    starting_station.distance = 0;
    starting_station.cars = NULL;

    int start_index = -1, target_index = -1;

    // Search for start and target stations
    for (int i = 0; i < highway->num_stations && (start_index == -1 || target_index == -1); i++) {
        if (highway->stations[i].distance == start) {
            starting_station = highway->stations[i];
            start_index = i;
        } else if (highway->stations[i].distance == target) {
            target_index = i;
        }
    }

    // Check if stations where found
    if (start_index == -1 || target_index == -1) {
        printf("nessun percorso\n"); // start and target stations not found
        return;
    }

    // Initialize path
    StationForPath *path = (StationForPath *) malloc(sizeof(StationForPath) * (target_index - start_index + 1));
    int path_dim = 1;

    path[0].distance = start;
    path[0].distancePlusMax = start + starting_station.cars->autonomy[0];


    int last_no = start_index + 1;
    int old_last = last_no;
    int found_target = 0;
    for (int i = start_index; i < target_index + 1 && !found_target; i++) {
        int reach = highway->stations[i].distance + highway->stations[i].cars->autonomy[0];

        for (int j = last_no; j < target_index + 1 && !found_target; j++) {
            int current_distance = highway->stations[j].distance;

            if (reach >= current_distance) {
                if (!isPresent(path, path_dim, current_distance)) {
                    int max = current_distance + highway->stations[j].cars->autonomy[0];

                    path[path_dim].distance = current_distance;
                    path[path_dim].distancePlusMax = max;
                    path_dim++;
                }

                if (highway->stations[j].distance == target) {
                    found_target = 1;
                }
            } else {
                last_no = j;
                break;
            }
        }

        if (old_last == last_no) {
            break;
        }
    }

    if (found_target == 0) {
        printf("nessun percorso\n"); // couldn't reach the target station
        return;
    }

    int last_distance = path[path_dim - 1].distance;
    for (int i = path_dim - 2; i > 0; i--) {
        if (path[i - 1].distancePlusMax >= last_distance) {
            path[i].distance = -1;
        } else {
            last_distance = path[i].distance;
        }
    }

    for (int i = 0; i < path_dim - 1; i++) {
        if (path[i].distance != -1) {
            printf("%d ", path[i].distance);
        }
    }
    printf("%d\n", path[path_dim - 1].distance);

    free(path);
}

void findShortestPathReverse(Highway *highway, int start, int target) {
    printf("nessun percorso\n");
}

// endregion find paths methods

// region print
void printHeap(Vehicles *v) {
    for (int i = 0; i < v->num_vehicles; i++) {
        printf("%d ", v->autonomy[i]);
    }
    printf("\n");
}

void printGraph(Highway *highway) {
    for (int i = 0; i < highway->num_stations; i++) {
        printf("Station: %d km, %d veicoli, veicoli: ", highway->stations[i].distance,
               highway->stations[i].cars->num_vehicles);
        printHeap(highway->stations[i].cars);
    }
}
// endregion print


int main() {
    Highway highway;
    initializeHighway(&highway);

    char command[20];
    int distance, num_vehicles, autonomy, start_station, target_station;
    int vehicles[MAX_VEHICLES];

    while (scanf("%s", command) != EOF) {
        if (strcmp(command, "aggiungi-stazione") == 0) {
            if (scanf("%d %d", &distance, &num_vehicles) != 2) {
                return 1;
            }
            for (int i = 0; i < num_vehicles; i++) {
                if (scanf("%d", &vehicles[i]) != 1) {
                    return 1;
                }
            }
            addStation(&highway, distance, num_vehicles, vehicles);
        } else if (strcmp(command, "demolisci-stazione") == 0) {
            if (scanf("%d", &distance) != 1) {
                return 1;
            }
            removeStation(&highway, distance);
        } else if (strcmp(command, "aggiungi-auto") == 0) {
            if (scanf("%d %d", &distance, &autonomy) != 2) {
                return 1;
            }
            addVehicle(&highway, distance, autonomy);
        } else if (strcmp(command, "rottama-auto") == 0) {
            if (scanf("%d %d", &distance, &autonomy) != 2) {
                return 1;
            }
            removeVehicle(&highway, distance, autonomy);
        } else if (strcmp(command, "pianifica-percorso") == 0) {
            if (scanf("%d %d", &start_station, &target_station) != 2) {
                return 1;
            }

            orderStationsHeapSort(&highway);

            if (start_station < target_station) {
                findShortestPath(&highway, start_station, target_station);
            } else if (start_station > target_station) {
                findShortestPathReverse(&highway, start_station, target_station);
            } else
                printf("%d\n", start_station);
        }
    }

    freeHighway(&highway);

    return 0;
}
