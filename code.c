#include <stdio.h>
#include <stdlib.h>

//possible commands: capital letters are used to distigush between them

//Aggiungi-Stazione     (add station)
//Aggiungi-Auto         (add car)
//Demolisci-stazione    (demolish station)
//Rottama-auto          (scrap car)
//Pianifica-percorso    (plan route)

//this struct contains the informations about the cars parked in every station
typedef struct heap{
    int* park;      //array of integers
    int park_len;   //lenght of the array
    int heap_len;   //number of actual cars in the array (must be <= park_len)
} Heap; typedef Heap* P;

//node of the Binary Search Tree that contains every station on the highway
typedef struct node{
    struct node* parent; 
    int km;                 //key, indicates the distance of the station from the start of the highway
    int dist;
    P park_p;               //pointer to the parking lot (heap of integers)
    struct node* previous;  //points to the previous node, used only by the ROUTE function
    struct node* left;
    struct node* right;
} Node; typedef Node* T;

//single node of a list, used as a Stack in the 'inverse_route' funcion
typedef struct plate{
    T key;
    struct plate* next;
} Plate; typedef Plate* Stack;


//functions associated with every input command
void add_station( T );
void add_car();
void demolish_station();
void scrap_car();
void route();

//useful funcions called by route()
void direct_route( int , int );
int dir_route_print( int , int , T );
void inverse_route( int , int );
void inv_route_print( int, int , T );

//functions used to build, modify and access data in the BST
T BST_search( T , int );
T BST_min( T );
T BST_max( T );
T BST_successor( T );
T BST_predecessor( T );
void BST_delete( T );

//functions used to handle the Stack
Stack push( Stack , T );
T pop( Stack* );
void free_stack( Stack );

//functions to handle the Heap (parking lot array)
void make_max_heap ( P );
void max_heapify( P , int );

//declaring the root of the BST
T station_tree = NULL;
//variable only used to catch the return value of the scanf() function, to avoid warnings
int scanf_avoid_warning;

int main(){
    char command[19];
    
    //identifies the input command and calls the corresponding function, until the end of the input file
    while(!feof(stdin)){
        if(scanf("%s", command) == 1){
            if(command[0]=='a' && command[9]=='s')
                add_station( station_tree );
            else if (command[0]=='a')
                add_car();
            else if (command[0]=='d')
                demolish_station();
            else if (command[0]=='r')
                scrap_car();
            else if (command[0]=='p')
                route();  
            printf("\n");
        }
    }
    return 0;
}

//adds station to the station tree at distance station_dist with the cars, prints "aggiunta" (added)
//if there is already station there prints "non aggiunta" (not added) and does nothing else
void add_station( T station_p ){
    int station_dist; scanf_avoid_warning = scanf("%d", &station_dist);
    int nr_cars; scanf_avoid_warning = scanf("%d", &nr_cars);
    int car_battery;

    //looking where to connect the node
    T prev = NULL;
    while( station_p != NULL ){
        if (station_p->km == station_dist){     // there is already a station at at that distance
            printf("non aggiunta");             // not added
            while (nr_cars > 0){                // needs to scan the cars anyway to get to the next command
                scanf_avoid_warning = scanf("%d", &car_battery);
                nr_cars--;
            }
            
            return;
        }
        else if (station_dist > station_p->km){ //looks for a spot to add the station in the right sub-tree
            prev = station_p;
            station_p = station_p->right;
        }
        else{                                   //left sub tree
            prev = station_p;
            station_p = station_p->left;
        }
    } //prev will point at the father of the node that needs to be added

    //adding the node
    T newnode = (Node*)malloc(sizeof(Node));
    newnode->parent = prev;
    newnode->left = NULL;
    newnode->right = NULL;
    newnode->km = station_dist;

    //connecting the node
    if(prev == NULL)    //newnode is the tree root
        station_tree = newnode;
    else{
        if(station_dist > prev->km)
            prev->right = newnode;
        else
            prev->left = newnode;
    }

    //calculating the size of the parking lot as the smallest power of 2 >= than nr_cars
    int i=1;
    while (i < nr_cars)
        i = 2*i;

    //adding the parking lot
    newnode->park_p = (Heap*)malloc(sizeof(Heap));
    newnode->park_p->park_len = i;
    newnode->park_p->heap_len = nr_cars;
    newnode->park_p->park = (int*)malloc(sizeof(int)*i);

    //adding cars to the parking lot, then ordering the as a heap
    if(nr_cars != 0){
        while (nr_cars>0){
            
            scanf_avoid_warning = scanf("%d", &car_battery);
            (newnode->park_p->park)[nr_cars - 1] = car_battery;
            nr_cars--;
        }
        make_max_heap( newnode->park_p );

    } else
        (newnode->park_p->park)[0] = 0; //its important to initialize the car battery to 0 even if there are no cars

    printf("aggiunta");
    return;
}

//adds a car to the station at distance station_dist, prints "aggiunta" (added)
//more than 1 car can have the same battery
//if there is no such station prints "non aggiunta"  (not added)
void add_car(){
    int station_dist; scanf_avoid_warning = scanf("%d", &station_dist);
    int car_battery; scanf_avoid_warning = scanf("%d", &car_battery);

    //look for the station
    T station = BST_search( station_tree , station_dist );
    if(station == NULL){
        printf("non aggiunta");
        return;
    }

    //if the parking lot is full, double the parking lot space
    int h_len = station->park_p->heap_len;
    if(h_len == station->park_p->park_len){ 
        station->park_p->park_len *=2;
        int* tmp = (int*)malloc(sizeof(int)*h_len*2);
        int i;
        //copy the cars into the new array
        for(i=0; i < h_len; i++)
            tmp[i] = (station->park_p->park)[i];
        free(station->park_p->park);
        station->park_p->park = tmp;
    }

    //add the car to the heap
    station->park_p->heap_len++;
    (station->park_p->park)[h_len] = (station->park_p->park)[0];
    (station->park_p->park)[0] = car_battery;
    make_max_heap(station->park_p); //optimizable, not always needed :/

    printf("aggiunta"); //(added)
    return;
}

//removes the station at distance station_dist, prints "demolita" (demolished)
//if there is no such station prints "non demolita" (not demolished)
void demolish_station(){
    int station_dist; scanf_avoid_warning = scanf("%d", &station_dist);
    //look for the station
    T station = BST_search( station_tree , station_dist );
    if( station == NULL ){      //that thation doesn't exist
        printf("non demolita"); //(not demolished)
        return;
    }

    //remove parking lot
    free(station->park_p->park);
    free(station->park_p);

    //remove the node from the tree
    BST_delete(station);
    printf("demolita"); //demolished

    return;
}

//removes a car whith battery "car_battery" 
//from the station at "station_dist" prints "rottamata" (scrapped)
//if there is no such station or car prints "non rottamata" (not scrapped)
void scrap_car(){
    int station_dist; scanf_avoid_warning = scanf("%d", &station_dist);
    int car_battery; scanf_avoid_warning = scanf("%d", &car_battery);

    //looking for the station
    T target_station = BST_search(station_tree, station_dist);
    if(target_station == NULL){ //the station doesn't exist
        printf("non rottamata"); // (not scrapped)
        return;
    }

    //looking for the car
    int* target_car = target_station->park_p->park;
    int hlen = target_station->park_p->heap_len;
    int i = 0;
    while(i < hlen && *(target_car + i) != car_battery)
        i++;
    
    if(i == hlen){      //there is no car in the parking lot with that battery
        printf("non rottamata");    //(not scrapped)
        return;
    }

    //swap car whith the last in the heap
    int tmp = *(target_car + i);
    *(target_car + i) = *(target_station->park_p->park + hlen - 1);
    *(target_station->park_p->park + hlen - 1) = tmp;
    target_station->park_p->heap_len--;     //decrease the number of actual cars in the parking lot
    
    //remake the heap
    make_max_heap(target_station->park_p);  //optimizable!! should start  make max heap procedure where the target station was
                                            //optimizable, half the array lenght if needed

    printf("rottamata");
    if(target_station->park_p->heap_len == 0)   //if the removed car was the only one in the array
        *(target_station->park_p->park) = 0;    //the first element must be set again to 0
                                                //when deleting a car I simply put it at the end of the array and reduce the counter by 1
    return;
}

void route(){
    int start_st_dis; scanf_avoid_warning = scanf("%d", &start_st_dis);
    int end_st_dist; scanf_avoid_warning = scanf("%d", &end_st_dist);

    //if they coincide
    if(start_st_dis == end_st_dist)
        printf("start_st_dis");

    //if we are going forward:
    if(start_st_dis < end_st_dist){
        direct_route(start_st_dis, end_st_dist);
        return;
    } else{
        inverse_route(end_st_dist, start_st_dis);
        return;
    }
    return;
}

void direct_route( int S , int F ){
    int found = 0;
    int tmp;
    T X = BST_search(station_tree, S); 

    //useless check, we assume S and F are in the station tree
    if(X != NULL)
        X->previous = NULL;
    T Y = BST_successor(X);
    if(Y != NULL) 
        Y->previous = NULL;

    while(!found && (X->previous != NULL || X->km == S)){
        tmp = X->km + *(X->park_p->park);   //this is the maximum distace reachable from station x
        while(Y->km <= tmp){
            Y->previous = X;
            if(Y->km == F){
                found = 1;
                break;
            }
            Y = BST_successor(Y);
            Y->previous = NULL;
        }
        X = BST_successor(X);
    }

    //print route in order
    if(!dir_route_print(S, F, Y))
        printf("nessun percorso");  //if the function returns 0, print (no route)
    return;
}

//recursive funtion used to print the route calculated, returns 0 if there is no such route
int dir_route_print( int S , int F , T X ){
    if(X->previous == NULL && X->km == S){
        printf("%d", X->km);
        return 1;
    }
    if(X->previous == NULL)
        return 0;
    if(dir_route_print(S, F, X->previous)){
        printf(" %d", X->km);
        return 1;
    } else return 0;
}

//different funtion from "direct_route" due to the nature of the problem
void inverse_route( int S , int F ){
    //creating stacks
    Stack even = NULL;  //used to kepp track of the stations reachable from the station X
    Stack odd = NULL;
    int step = 1;
    int found = 0;
    //looking for starting node
    T X = BST_search(station_tree, F);
    
    X->previous = NULL;
    even = push(even, X);

    T Y = BST_predecessor(X);
    Y->previous = NULL;

    while((even!=NULL || odd!=NULL) && !found){
        if(step%2 == 0)
            X = pop(&odd);
        else X = pop(&even);

        if(X == NULL){
            step++;
            continue;
        }
        while(Y->km >= X->km - (X->park_p->park)[0]){
            Y->previous = X;

            if(step%2==0)
                even = push(even, Y);
            else odd = push(odd, Y);

            if(Y->km != S){
                Y = BST_predecessor(Y);
                Y->previous = NULL;
            } else{
                found = 1;
                break;
            }
        }
    }

    //print route in order
    if(!dir_route_print(F, S, Y))
        printf("nessun percorso");

    return;
}

//recursive funtion used to print the route calculated by inverse route, returns 0 if there is no such route
void inv_route_print( int S, int F, T X ){
    if(X->previous == NULL){
        printf("nessun percorso");
        return;
    }
    while(X->previous != NULL){
        printf("%d ", X->km);
        X = X->previous;
    }
    printf("%d", S);
    return;
}


void max_heapify( P heap , int i ){
    int l = i*2;
    int r = i*2 +1;
    int max;
    int tmp;

    //finding the maximum value among i, and its children
    if(l<=heap->heap_len - 1 && *(heap->park + l) > *(heap->park + i))
        max = l;
    else max = i;
    if(r<=heap->heap_len - 1 && *(heap->park + r) > *(heap->park + max))
        max = r;
    
    //swapping i with the maximum and repeating the procedure
    if(max != i){
        tmp = *(heap->park + i);
        *(heap->park + i) = *(heap->park + max);
        *(heap->park + max) = tmp;
        max_heapify ( heap , max );
    }

    return;
}

void make_max_heap ( P heap ){
    int i; 
    for(i = heap->heap_len - 1; i >= 0; i--)
        max_heapify( heap, i );

    return;
}


//serches recursively the station ad distance "distance" inside the sub-tree with root "station"
//returns a pointer to the node if the station is found, NULL otherwise
T BST_search( T station , int distance ){
    if(station == NULL)
        return NULL;
    if(station->km == distance)
        return station;
    if(distance > station->km)
        return BST_search(station->right, distance);
    else
        return BST_search(station->left, distance);
}

//resturns a pointer to the station closer to the start of the highway
T BST_min( T station ){
    if(station == NULL){
        printf("\n\n NULL ARGUMENT TO BST_MIN\n\n");
        return NULL;
    }
    while(station->left != NULL)
        station = station->left;
    return station;
}

//resturns a pointer to the station closer to the end of the highway
T BST_max( T station ){
    if(station == NULL){
        printf("\n\n NULL ARGUMENT TO BST_MIN\n\n");
        return NULL;
    }
    while(station->right != NULL)
        station = station->right;
    return station;
}

//returns a pointer to the station following the one passed as an argument
T BST_successor( T station ){
    if(station == NULL){
        printf("\n\n NULL ARGUMENT TO BST_SUCCESSOR\n\n");
        return NULL;
    }
    if(station->right != NULL)
        return BST_min(station->right);
    T father = station->parent;
    while (father != NULL && station == father->right){
        station = father;
        father = father->parent;
    }
    return father;
}

T BST_predecessor( T station ){
    if(station == NULL){
        printf("\n\n NULL ARGUMENT TO BST_SUCCESSOR\n\n");
        return NULL;
    }
    if(station->left != NULL)
        return BST_max(station->left);
    T father = station->parent;
    while (father != NULL && station == father->left){
        station = father;
        father = father->parent;
    }
    return father;
}

//deletes a station from the BST
void BST_delete( T station ){
    T to_delete;
    if(station->left == NULL || station->right == NULL)
        to_delete = station;
    else to_delete = BST_successor(station);            

    T substitute;
    if(to_delete->left != NULL)
        substitute = to_delete->left;
    else substitute = to_delete->right;

    if(substitute != NULL)
        substitute->parent = to_delete->parent;
    if(to_delete->parent == NULL)
        station_tree = substitute;
    else if(to_delete == to_delete->parent->left)
        to_delete->parent->left = substitute;
    else    to_delete->parent->right = substitute;

    if(to_delete != station){
        station->km = to_delete->km;
        station->park_p = to_delete->park_p;
    }

    free(to_delete);
    return;
}

//adds a station on the stack
Stack push( Stack st , T x ){
    Stack tmp = (Plate*)malloc(sizeof(Plate));
    tmp->key = x;
    tmp->next = st;
    return tmp;
}

//removes a station from the stack and returnes it
T pop( Stack* stp ){
    if(*stp == NULL)
        return NULL;

    T to_return = (*stp)->key;
    Stack to_delete = *stp;
    *stp = (*stp)->next;
    free(to_delete);
    return to_return;
}

//frees the memory occupied by a stack
void free_stack( Stack st ){
    if(st == NULL)
        return;

    free_stack(st->next);
    free(st);
    return;
}
