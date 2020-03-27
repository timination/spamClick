//
//  main.cpp
//  spamClick
//
//  Created by Timothy Talbot on 15/03/2020.
//

#include <thread>
#include <CoreGraphics/CGEventSource.h>
#include <CoreGraphics/CGEvent.h>
#include <Carbon/Carbon.h>
#include <iostream>
#include <ctype.h>

bool isInt(const char *str);
void doClick();
void toggle();
void printUsage();
void tick(useconds_t intermission);

OSStatus mbHotKeyHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData);

bool click = false;
bool exiting = false;

CGPoint point;

int main(int argc, const char * argv[]) {
    
    EventHotKeyRef  toggleHotkeyRef, exitHotkeyRef;
    EventHotKeyID   toggleHotkey, exitHotkey;
    EventTypeSpec   eventType;
    
    eventType.eventClass    = kEventClassKeyboard;
    eventType.eventKind     = kEventHotKeyPressed;
    
    EventTargetRef eventTarget = GetApplicationEventTarget();
    InstallEventHandler(eventTarget, &mbHotKeyHandler, 1, &eventType, NULL, NULL);
    
    toggleHotkey.signature  = 'tghk';
    toggleHotkey.id         = 1337;
    
    RegisterEventHotKey(kVK_ANSI_T, cmdKey, toggleHotkey, eventTarget, 0, &toggleHotkeyRef);

    exitHotkey.signature  = 'exhk';
    exitHotkey.id         = 2674;
    RegisterEventHotKey(kVK_ANSI_Y, cmdKey, exitHotkey, eventTarget, 0, &exitHotkeyRef);

    useconds_t intermission = 300000;
    
    if(argc > 1) {
        if(!isInt(argv[1])) {
            printUsage();
            exit(0);
        } else {
            if(atoi(argv[1]) < 300 || atoi(argv[1]) > 60000) {
                printUsage();
                exit(0);
            }
        }
        intermission = atoi(argv[1]) * 1000;
    }
    
    std::thread t1(tick, intermission);
    
    EventRef theEvent;
    while
        (
         ReceiveNextEvent
         (
          0,
          NULL,
          kEventDurationForever,
          kEventRemoveFromQueue,
          &theEvent
          )
         ==
         noErr
         )
    {
        if (GetEventKind(theEvent) == kEventAppleEvent)
            AEProcessEvent(theEvent);
        
        SendEventToEventTarget(theEvent, eventTarget);
        ReleaseEvent(theEvent);
    }
    
    exiting = true;
    t1.join();
    return 0;
}

OSStatus mbHotKeyHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
     EventHotKeyID hkCom;
     GetEventParameter(event,kEventParamDirectObject,typeEventHotKeyID,NULL,
    sizeof(hkCom),NULL,&hkCom);
     int switchCase = hkCom.id;
     
     switch (switchCase) {
      case 1337:
             toggle();
       break;
      case 2674:
             // exit
             QuitEventLoop(GetCurrentEventLoop());
       break;
     }

    return noErr;
}

void tick(useconds_t intermission) {
    while(!exiting) {
        if(click) {
            doClick();
        }
        usleep(intermission);
    }
}

void doClick() {
    CGEventRef mouseDownEv = CGEventCreateMouseEvent (NULL,kCGEventLeftMouseDown,point,kCGMouseButtonLeft);
    CGEventPost (kCGHIDEventTap, mouseDownEv);
    
    CGEventRef mouseUpEv = CGEventCreateMouseEvent (NULL,kCGEventLeftMouseUp,point,kCGMouseButtonLeft);
    CGEventPost (kCGHIDEventTap, mouseUpEv );
}

void toggle() {
    // get current cursor position
    CGEventRef ourEvent = CGEventCreate(NULL);
    point = CGEventGetLocation(ourEvent);
    CFRelease(ourEvent);
    
    click = !click;
}

bool isInt(const char *str) {
    while (*str)
        if (!isdigit(*str++))
            return false;
    return true;
}

void printUsage() {
    std::cout << "Usage: spamClick" << std::endl;
    std::cout << "Usage: spamClick interval_in_ms" << std::endl;
    std::cout << "ms range (300-60000)" << std::endl;
}
