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
 
void clear_bit(unsigned char *array, int index) {

    unsigned char * to_change = array + (index / 8);
    *to_change = *to_change & ~(1 << (index % 8));
    
}

void set_bit(unsigned char *array, int index) {
    unsigned char * to_change = array + (index / 8);
    *to_change = *to_change | 1 << (index % 8);
}

char get_bit(unsigned char *array, int index) {
    return 1 & (array[index / 8] >> (index % 8));
}

unsigned long get_frames(int* arr, unsigned int _n_frames, unsigned int range)
{

    int found = -1; // -1: finding, 0: not found, 1: found

    for(unsigned long i = 0; (i+_n_frames < range); ){
        
        printf("i = %li",i);

        found = -1; //start finding frames again

        if(arr[i] == WORK_BUSY){
            printf(" --> i: index busy\n");
            i++;
            continue;
        }

        printf(" --> i: index free\n");

        for(unsigned long j = i+1; j < i+_n_frames; j++){

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


void print_array_char(unsigned char *buf, int char_count)
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


void print_array_long(unsigned long* buf)
{
    printf("------------------->\n");
    unsigned long memory = *buf;
    for (unsigned long i = 0; i < 32; i++)
    {
        if(i%4 == 0){
            printf("_");
        }

        printf("%d", memory >> i & 1);
        

    }
    printf("\n");

}

unsigned long dir_ten_bits(unsigned long* v_addr){
    unsigned long dir_bits = (*v_addr) & 0xFFC00000;

    dir_bits = dir_bits >> 22;

    return (dir_bits);
}

unsigned long table_ten_bits(unsigned long* v_addr){
    unsigned long table_bits = (*v_addr) & 0x3FF000;

    table_bits = table_bits >> 12;

    return (table_bits);
}

unsigned long offset_twelve_bits(unsigned long* v_addr){
    unsigned long offset_bits = (*v_addr) & 0xFFF;
    return (offset_bits);
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

    
    unsigned long v_addr = 0x8C6AAFC0;
    print_array_long(&v_addr);

    /*
    v_addr = v_addr >> 10;
    v_addr = v_addr | 0xFFC00000;
    v_addr = v_addr >> 2;
    v_addr = v_addr << 2;
    print_array_long(&v_addr);
    */
   
    v_addr = v_addr >> 20;
    v_addr = v_addr | 0xFFFFF000;
    v_addr = v_addr >> 2;
    v_addr = v_addr << 2;    
    print_array_long(&v_addr);

    /*
    unsigned long v_addr = 0xFFFFFFFF;
    print_array_long(&v_addr);

    v_addr = 0xFFFFFFFF >> 12;
    print_array_long(&v_addr);

    v_addr = 0xFFFFFFFF << 12;
    print_array_long(&v_addr);
    */
    /*
    unsigned long v_addr = 0xBA851DE9;
    print_array_long(&v_addr);


    unsigned long offset_bits = offset_twelve_bits(&v_addr);
    print_array_long(&offset_bits);
    */


    /*
    unsigned long d_addr  = 0x9C000;
    d_addr |= 2;
    print_array_long(&d_addr);
    
    unsigned long one_bit = d_addr & 1;
    print_array_long(&one_bit);
    */

    //printf("size of unsigned long = %li\n", sizeof(unsigned long));
    /*
    unsigned long addr    = 3;
    unsigned long d_addr  = 0x9C000;
    unsigned long t_addr  = 0x9D000;

    unsigned long to_print = d_addr;
    print_array_long(&to_print);

    to_print = d_addr | 3;
    print_array_long(&to_print);
    
    printf("In decimal = %lu\n", addr);
    */


/*
    char a = 3;
    int i;
    for (i = 0; i < 8; i++) {
      printf("%d", !!((a << i) & 0x80));
    }
    printf("\n");
    exit(0);
*/
/*  printf("\n---------------------------------------------------------------------------------\n");
    
    
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
*/

    return 0;
}