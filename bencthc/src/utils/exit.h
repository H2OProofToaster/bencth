//
// Created by nick on 6/27/26.
//

#ifndef BENCTH_EXIT_H
#define BENCTH_EXIT_H

//_Noreturn to tell ide that this ends the program
_Noreturn void die(const char* err);
void warn(const char* warn);

#endif //BENCTH_EXIT_H