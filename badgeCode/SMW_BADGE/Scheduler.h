#include <Arduino.h>

typedef void (*ptrToFunction)();

#define maxFunctions 5

class Scheduler{
  
  struct todoItem_t 
  {
    ptrToFunction func;
    long unsigned int howOften;
    long unsigned int lastTime;
  };
  
  private:
    todoItem_t list[maxFunctions];
    int count;
  
  public:
    Scheduler(){
      count = 0;
    }
    
  void addItem(ptrToFunction func,long unsigned int howOften){
    if(count<maxFunctions){
      todoItem_t item;
      item.howOften = howOften;
      item.func = func;
      item.lastTime = millis();
      list[count++] = item;
    }
  }
  
  void update(){
    todoItem_t *item;
    for(int i=0;i<count;i++){
      item = &(list[i]);
      if(item->howOften>0){
        long unsigned int t = millis();
        long unsigned int diff = t - item->lastTime;
        if(diff > item->howOften){
          //Serial.print("diff:");
          //Serial.println(diff);
          item->lastTime = t - (diff - item->howOften);
          item->func();
        }
      } else {
        item->func();
      }
    }
  }
};
