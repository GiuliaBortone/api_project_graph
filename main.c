#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VEHICLES 512

typedef struct {
    int start;
    int end;
    int weight;
} Connection; // Edge

typedef struct { // the highest value is vehicles->autonomy[0]
    int *autonomy;
    int num_vehicles;
    // int capacity = MAX_VEHICLES; quindi non lo scrivo
} Vehicles;

typedef struct {
    int distance;
    Vehicles *cars;
    Connection *connections;
    int num_connections;
} Station; // Node

typedef struct {
    Station *stations;
    int num_stations;
} Highway; //Graph

void addConnection(Station *station, int start, int end) {
    if (station->num_connections == 0) {
        station->connections = (Connection *) malloc(sizeof(Connection));
    } else {
        station->connections = (Connection *) realloc(station->connections,
                                                      (station->num_connections + 1) * sizeof(Connection));
    }

    station->connections[station->num_connections].start = start;
    station->connections[station->num_connections].end = end;
    station->connections[station->num_connections].weight = 1;

    station->num_connections++;
}

bool containsConnection(Station *station, int start, int end) {
    for (int i = 0; i < station->num_connections; i++) {
        if (station->connections[i].start == start && station->connections[i].end == end) {
            return true;
        }
    }
    return false;
}

// region initialization and deletion of graph
void initializeHighway(Highway *highway) {
    highway->num_stations = 0;
    highway->stations = NULL;
}

void freeHighway(Highway *highway) {
    for (int i = 0; i < highway->num_stations; i++) {
        free(highway->stations[i].cars->autonomy);
        free(highway->stations[i].cars);
        free(highway->stations[i].connections);
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
                int old_max = highway->stations[i].cars->autonomy[0];

                highway->stations[i].cars->autonomy[num_vehicles] = vehicle;
                insertionHelper(highway->stations[i].cars, highway->stations[i].cars->num_vehicles);
                highway->stations[i].cars->num_vehicles++;

                // add connections if needed
                if (vehicle > old_max) {
                    for (int j = 0; j < highway->num_stations; j++) {
                        if (j != i) {
                            int current_distance = highway->stations[j].distance;

                            if (((distance < current_distance && distance + vehicle >= current_distance)
                                 || (distance > current_distance && distance - vehicle <= current_distance)) &&
                                !containsConnection(&highway->stations[i], distance, current_distance)) {
                                addConnection(&highway->stations[i], distance,
                                              current_distance);
                            }
                        }
                    }
                }

                printf("aggiunta\n");
                return;
            }
            printf("non aggiunta\n"); // max vehicles reached
            return;
        }
    }

    printf("non aggiunta\n"); // station non found
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
                    int old_max = highway->stations[i].cars->autonomy[0];

                    highway->stations[i].cars->autonomy[j] = highway->stations[i].cars->autonomy[num_vehicles - 1];
                    highway->stations[i].cars->num_vehicles--;
                    maxHeapify(highway->stations[i].cars, j);
                    printf("rottamata\n");

                    // fix connections
                    if (old_max == vehicle) {
                        int new_max = highway->stations[i].cars->autonomy[0];

                        for (int k = 0; k < highway->num_stations; k++) {
                            if (k != i) {
                                int current_distance = highway->stations[k].distance;

                                if (!(distance < current_distance && distance + new_max >= current_distance)
                                    && !(distance > current_distance && distance - new_max <= current_distance) &&
                                    containsConnection(&highway->stations[i], distance, current_distance)) {
                                    // remove connection
                                    for (int t = j; t < highway->stations[i].num_connections - 1; t++) {
                                        highway->stations[i].connections[t] = highway->stations[i].connections[t + 1];
                                    }

                                    highway->stations[i].num_connections--;
                                    highway->stations[i].connections = (Connection *) realloc(
                                            highway->stations[i].connections,
                                            (highway->stations[i].num_connections) * sizeof(Connection));
                                }
                            }
                        }
                    }
                    return;
                }
            }
            printf("non rottamata\n"); // vehicle not found
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
    highway->stations[highway->num_stations - 1].num_connections = 0;
    highway->stations[highway->num_stations - 1].connections = NULL;

    int max = highway->stations[highway->num_stations - 1].cars->autonomy[0];

    // adding edges related to new station
    for (int i = 0; i < highway->num_stations - 1; i++) {// - 1 because I don't want to check the station I just added
        int current_distance = highway->stations[i].distance;
        int current_max = highway->stations[i].cars->autonomy[0];

        // adding edges starting from the new station
        if ((distance < current_distance && distance + max >= current_distance)
            || (distance > current_distance && distance - max <= current_distance)) {
            addConnection(&highway->stations[highway->num_stations - 1], distance,
                          current_distance);
        }

        // adding edges from old stations to the new station
        if ((distance > current_distance && current_distance + current_max >= distance)
            || (distance < current_distance && current_distance - current_max <= distance)) {
            addConnection(&highway->stations[i], current_distance, distance);
        }
    }

    printf("aggiunta\n");
}

void removeStation(Highway *highway, int distance) {
    for (int i = 0; i < highway->num_stations; i++) {
        if (highway->stations[i].distance == distance) {

            free(highway->stations[i].connections);

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

// region print
void printHeap(Vehicles *v) {
    for (int i = 0; i < v->num_vehicles; i++) {
        printf("%d ", v->autonomy[i]);
    }
    printf("\n");
}

void printfConnections(Station *station) {
    for (int i = 0; i < station->num_connections; i++) {
        printf("(%d, %d, %d)", station->connections[i].start, station->connections[i].end,
               station->connections[i].weight);
    }
    printf("\n");
}

void printGraph(Highway *highway) {
    for (int i = 0; i < highway->num_stations; i++) {
        printf("Station: %d km, %d veicoli, veicoli: ", highway->stations[i].distance,
               highway->stations[i].cars->num_vehicles);
        printfConnections(&highway->stations[i]);
        printHeap(highway->stations[i].cars);
    }
}
// endregion print


int main() {
    Highway highway;
    initializeHighway(&highway);

    char command[20];
    int distance, num_vehicles, autonomy; //start_station, target_station;
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
            //    if (scanf("%d %d", &start_station, &target_station) != 2) {
            //        return 1;
            //    }
            //    findShortestPath(&graph, start_station, target_station);
            printf("nessun percorso\n");
        }
    }

    //printGraph(&highway);

    freeHighway(&highway);

    return 0;
}
