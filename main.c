#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

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
    int reach;
    int parent; // index of parent
    int costToReach;
    bool visited; // true for visited, false for not visited
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
    int low = 0, high = highway->num_stations - 1;
    int index = -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (highway->stations[mid].distance == distance) {
            index = mid;
            break;
        } else if (highway->stations[mid].distance < distance) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (index == -1) {
        printf("non aggiunta\n"); // station not found
        return;
    }

    int num_vehicles = highway->stations[index].cars->num_vehicles;

    if (num_vehicles < MAX_VEHICLES) {
        highway->stations[index].cars->autonomy[num_vehicles] = vehicle;
        insertionHelper(highway->stations[index].cars, highway->stations[index].cars->num_vehicles);
        highway->stations[index].cars->num_vehicles++;

        printf("aggiunta\n");
        return;
    }
    printf("non aggiunta\n"); // max vehicles reached
}

void removeVehicle(Highway *highway, int distance, int vehicle) {
    int low = 0, high = highway->num_stations - 1;
    int index = -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (highway->stations[mid].distance == distance) {
            index = mid;
            break;
        } else if (highway->stations[mid].distance < distance) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (index == -1) {
        printf("non rottamata\n"); // station not found
        return;
    }

    int num_vehicles = highway->stations[index].cars->num_vehicles;

    if (num_vehicles == 0) {
        printf("non rottamata\n");
        return;
    }

    for (int j = 0; j < num_vehicles; j++) {
        if (highway->stations[index].cars->autonomy[j] == vehicle) {
            highway->stations[index].cars->autonomy[j] = highway->stations[index].cars->autonomy[num_vehicles - 1];
            highway->stations[index].cars->num_vehicles--;
            if (highway->stations[index].cars->num_vehicles == 0) {
                highway->stations[index].cars->autonomy[0] = 0;
            } else
                maxHeapify(highway->stations[index].cars, j);
            printf("rottamata\n");
            return;
        }
    }
    printf("non rottamata\n"); // vehicle not found
}
// endregion max heap methods for vehicles in each station

// region graph methods
void addStation(Highway *highway, int distance, int num_vehicles, int *cars) {
    int left = 0;
    int right = highway->num_stations - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (highway->stations[mid].distance == distance) {
            printf("non aggiunta\n");
            return;
        } else if (highway->stations[mid].distance < distance) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    highway->stations = (Station *) realloc(highway->stations, (highway->num_stations + 1) * sizeof(Station));

    for (int i = highway->num_stations - 1; i >= left; i--) {
        highway->stations[i + 1] = highway->stations[i];
    }

    highway->stations[left].distance = distance;
    highway->stations[left].cars = createHeap(num_vehicles, cars);
    highway->num_stations++;
    printf("aggiunta\n");

}

void removeStation(Highway *highway, int distance) {
    int low = 0, high = highway->num_stations - 1;
    int index = -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (highway->stations[mid].distance == distance) {
            index = mid;
            break;
        } else if (highway->stations[mid].distance < distance) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (index == -1) {
        printf("non demolita\n"); // station not found
        return;
    }

    free(highway->stations[index].cars->autonomy);

    for (int j = index; j < highway->num_stations - 1; j++) {
        highway->stations[j] = highway->stations[j + 1];
    }

    highway->num_stations--;
    highway->stations = (Station *) realloc(highway->stations, highway->num_stations * sizeof(Station));

    printf("demolita\n");
}
// endregion graph methods

// region find path methods
void findShortestPathForward(Highway *highway, int start, int target) {
    int start_index = -1, target_index = -1;

    // Search for start and target stations
    for (int i = 0; i < highway->num_stations && (start_index == -1 || target_index == -1); i++) {
        if (highway->stations[i].distance == start) {
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
    StationForPath path[highway->num_stations - start_index + 1];
    int path_dim = 1;

    path[0].distance = start;
    path[0].reach = start + highway->stations[start_index].cars->autonomy[0];
    path[0].parent = -1;
    path[0].costToReach = 0;
    path[0].visited = false;

    for (int i = start_index + 1; i <= target_index; i++) {
        path[path_dim].distance = highway->stations[i].distance;
        path[path_dim].reach = highway->stations[i].distance + highway->stations[i].cars->autonomy[0];
        path[path_dim].parent = -1;
        path[path_dim].costToReach = INT_MAX;
        path[path_dim].visited = false;

        path_dim++;
    }

    for (int i = 0; i < path_dim - 1; i++) {
        for (int j = i + 1; j < path_dim; j++) {
            if (path[i].reach >= path[j].distance) {
                if (!path[j].visited) {
                    path[j].parent = i;
                    path[j].costToReach = path[i].costToReach + 1;
                    path[j].visited = true;
                } else {
                    if (path[i].costToReach + 1 < path[j].costToReach) {
                        path[j].parent = i;
                        path[j].costToReach = path[i].costToReach + 1;
                    }
                }
            } else
                break;
        }
    }

    if (path[path_dim - 1].visited == false) {
        printf("nessun percorso\n");
        return;
    }

    int *true_path = (int *) malloc(sizeof(int) * path_dim);
    int true_dim = 1;
    true_path[0] = path[path_dim - 1].distance;

    int i = path[path_dim - 1].parent;
    while (i >= 0) {
        true_path[true_dim] = path[i].distance;
        true_dim++;

        i = path[i].parent;
    }

    if (true_path[true_dim - 1] != start) {
        printf("nessun percorso\n");
        return;
    }

    for (int j = true_dim - 1; j > 0; j--) {
        printf("%d ", true_path[j]);
    }
    printf("%d\n", true_path[0]);

    free(true_path);
}

void findShortestPathBackward(Highway *highway, int start, int target) {
    int start_index = -1, target_index = -1;

    // Search for start and target stations
    for (int i = 0; i < highway->num_stations && (start_index == -1 || target_index == -1); i++) {
        if (highway->stations[i].distance == start) {
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
    StationForPath path[start_index - target_index + 1];
    int path_dim = 1;

    path[0].distance = start;
    path[0].reach = start - highway->stations[start_index].cars->autonomy[0];
    path[0].parent = -1;
    path[0].costToReach = 0;
    path[0].visited = false;

    for (int i = start_index - 1; i >= target_index; i--) {
        path[path_dim].distance = highway->stations[i].distance;
        path[path_dim].reach = highway->stations[i].distance - highway->stations[i].cars->autonomy[0];
        path[path_dim].parent = -1;
        path[path_dim].costToReach = INT_MAX;
        path[path_dim].visited = false;
        path_dim++;
    }

    for (int i = 0; i < path_dim - 1; i++) {
        for (int j = i + 1; j < path_dim; j++) {
            if (path[i].reach <= path[j].distance) {
                if (!path[j].visited) {
                    path[j].parent = i;
                    path[j].costToReach = path[i].costToReach + 1;
                    path[j].visited = true;
                } else {
                    if (path[i].costToReach + 1 <= path[j].costToReach) {
                        path[j].parent = i;
                        path[j].costToReach = path[i].costToReach + 1;
                    }
                }
            } else
                break;
        }
    }

    if (path[path_dim - 1].visited == false) {
        printf("nessun percorso\n");
        return;
    }

    int *true_path = (int *) malloc(sizeof(int) * path_dim);
    int true_dim = 1;
    true_path[0] = path[path_dim - 1].distance;

    int i = path[path_dim - 1].parent;
    while (i >= 0) {
        true_path[true_dim] = path[i].distance;
        true_dim++;

        i = path[i].parent;
    }

    if (true_path[true_dim - 1] != start) {
        printf("nessun percorso\n");
        return;
    }

    for (int j = true_dim - 1; j > 0; j--) {
        printf("%d ", true_path[j]);
    }
    printf("%d\n", true_path[0]);

    free(true_path);
}


// endregion find paths methods

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

            if (start_station < target_station) {
                findShortestPathForward(&highway, start_station, target_station);
            } else if (start_station > target_station) {
                findShortestPathBackward(&highway, start_station, target_station);
            } else
                printf("%d\n", start_station);
        }
    }

    freeHighway(&highway);

    return 0;
}
