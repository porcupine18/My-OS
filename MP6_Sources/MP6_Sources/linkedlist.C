#include "linkedlist.H"


ListItem::ListItem(Thread* inp){
    next = NULL;
    th = inp;
}


void ListItem::push(Thread* inp){
    if(this->th == NULL){
        this->th = inp;
        return;
    }
    if(this->next == NULL)
        this->next = new ListItem(inp);
    else
        this->next->push(inp);
}

Thread* ListItem::pop(){

    if(!this->th){
        return NULL;
    }
    else{
        Thread* tmp_th = this->th;
        ListItem* tmp_nxt = this->next;
        
        if(!this->next){
            this->th = NULL;
            return tmp_th;
        }
        
        this->th = tmp_th;
        this->next = tmp_nxt->next;
        return tmp_th;
    }
}

