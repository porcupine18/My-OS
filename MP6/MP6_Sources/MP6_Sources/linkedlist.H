#include "thread.C"


class ListItem{

    private:

        ListItem* next;
        Thread* th;

    public:

        ListItem(Thread* inp);
    
        void push(Thread* inp);
        
        Thread* pop();
};