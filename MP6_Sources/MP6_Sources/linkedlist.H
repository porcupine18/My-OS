#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "thread.H"

class ListItem{

    private:

        ListItem* next;
        Thread* th;

    public:

        ListItem(Thread* inp);
    
        void push(Thread* inp);
        
        Thread* pop();
};

#endif
