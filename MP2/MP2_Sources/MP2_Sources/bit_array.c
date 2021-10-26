#include <stdio.h>

#define WORK_FREE 0
#define WORK_BUSY 1
 
#define SIZE (40) /* amount of bits */
#define ARRAY_SIZE(x) (x/8 + (x%8 > 0 ? 1: 0))     // KAP this looks wrong   #define ARRAY_SIZE(x) (x/8+((x%8)))
 
char get_bit(unsigned char *array, int index);

void toggle_bit(unsigned char *array, int index);

void toggle_bit(unsigned char *array, int index) {
    array[index / 8] ^= 1 << (index % 8);
}
 
char clear_bit(unsigned char *array, int index) {

    unsigned char * to_change = array + (index / 8);
    *to_change = *to_change & ~(1 << (index % 8));
}

char set_bit(unsigned char *array, int index) {
    unsigned char * to_change = array + (index / 8);
    *to_change = *to_change | 1 << (index % 8);
}

char get_bit(unsigned char *array, int index) {
    return 1 & (array[index / 8] >> (index % 8));
}

unsigned long get_frames(int* arr, unsigned int _n_frames, unsigned int range)
{

    int found = -1; // -1: finding, 0: not found, 1: found

    for(ssize_t i = 0; (i+_n_frames < range); ){
        
        printf("i = %li",i);

        found = -1; //start finding frames again

        if(arr[i] == WORK_BUSY){
            printf(" --> i: index busy\n");
            i++;
            continue;
        }

        printf(" --> i: index free\n");

        for(ssize_t j = i+1; j < i+_n_frames; j++){

            printf("    j = %li",j);

            if( arr[j] == WORK_BUSY){
                printf(" --> j: index busy --> i = %li\n",j+1);
                i = j+1; 
                found = 0; // not found
                break;
            }
            printf(" --> j: index free\n");
        }


        if (found == -1){ // finding and not NOT FOUND => found!
            printf("Found free sequence\n");            
            found = 1;
            return (i);
        }

    }

    if(found == -1){
        printf("Not Found free sequence\n");                    
        return 0;
    }

    printf("ContframePool::get_frames - Should not reach here!\n");
    return 0;

}


void print_array(unsigned char *buf, int char_count)
{
    int i;
    unsigned char * buffer = buf;
    for (i = 0; i < char_count; i++)
    {
        for (int j = 0; j < 8; j++) 
        {
            printf("%d", !!(((*buffer) << j) & 0x80));
        }
        printf("  ");
        buffer = buffer + 1;
    }

    printf("\n---------------------------------------------------------------------------------\n");
}
#include <stdlib.h>
int main(void) {
    /* initialize empty array with the right size */

    int n = 1;
    // little endian if true
    if(*(unsigned char *)&n == 1) 
    {
        printf("Little Endian");
    }
    else
    {
        printf("Big Endian");
    }
    printf("\n");
/*
    char a = 3;
    int i;
    for (i = 0; i < 8; i++) {
      printf("%d", !!((a << i) & 0x80));
    }
    printf("\n");
    exit(0);
*/
    printf("\n---------------------------------------------------------------------------------\n");
    
    
    unsigned char x[ARRAY_SIZE(SIZE)] = {0};
    printf("No. of Char = %d\n",ARRAY_SIZE(SIZE));
    print_array(x, ARRAY_SIZE(SIZE));
  
    for(int i = 0; i < SIZE; i++){
        set_bit(x, i);        
    }
    print_array(x, ARRAY_SIZE(SIZE));
    
    for(int i = 0; i < SIZE; i++){
        clear_bit(x, i);        
        print_array(x, ARRAY_SIZE(SIZE));
    }


    set_bit(x, 6);        
    set_bit(x, 2);        
    print_array(x, ARRAY_SIZE(SIZE));    

    set_bit(x, 14);        
    set_bit(x, 10);        
    print_array(x, ARRAY_SIZE(SIZE));    

    set_bit(x, 22);        
    set_bit(x, 18);        
    print_array(x, ARRAY_SIZE(SIZE));    


    return 0;
}