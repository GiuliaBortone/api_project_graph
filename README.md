Algorithms project 2022/23

Grade: 30 cum laude/30

The specifications in Italian for the project are in the documentation folder.

To sum up in english: There is a highway with a sequence of gas stations. Every gas station is found
at a certain distance from the beginning of the highway, in km (positive or null). There can't
be gas stations at the same distance.
Each station has a series of electric vehicles to rent, each with a certain autonomy in
km (positive), and there can be at most 512 vehicles in a station. If I take one of these cars
from a station, I can reach those stations that are at most at a distance of that car's autonomy.
So if I am at station 20, which has a car with an autonomy of 20 km, and after me there are
stations 25 32 41, I can only reach stations 25 and 32, but not 41. Note that you are only planning
the trip, so you are not actually moving cars from one station to the other.
The objective of the project was to find the best data stracture (temporally and spatially)
to represent the highway with its stations and the stations with their cars, in order to then
find very efficiently the best path from a station to another. The best path is that with the
fewest stops (every time you stop, you must change car) and that which has stations closest
to the 0 km. So if I have two possible paths 20 30 50 and 20 45 50, I'd choose the first one, because
30 < 45; while if I had 50 30 20 and 50 45 20, again it would be the first one, because 30 < 45.

For my project I decided to use a graph, however since actually finding and creating the
edges between nodes it takes a lot of time and space, I decided to use a "virtual" graph. So the
algorithm you use to find the best path is still the one you would use on a graph, but adapted so
that it checks if the connection is possible each time. Furthermore, the cars in each station are
saved with a max heap: since I didn't have to actually move cars, I could take each time the one with
the highest autonomy and a max heap allowed me to it very quickly, since I only had to take the car at
position 0 in the array.
